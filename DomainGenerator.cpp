//============================================================================
// Name        : DomainGenerator.cpp
// Author      : Tuan Nguyen
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================
#include "DomainGenerator.h"
#include <sstream>
using namespace std;

int main(int argc, char *argv[]) {

//#define DEBUG_MAIN

	// Process the command line
	CommandLine cl;
	process_command_line(argc, argv, cl);
	if (cl.input_domain == "") {
		cerr<<"Input domain unspecified."<<endl;
		exit(1);
	}

	if (cl.num_ouput_domains == 0) {
		cerr<<"Number of output incomplete domains unspecified."<<endl;
		exit(1);
	}

	if (cl.num_poss_pres + cl.num_poss_adds + cl.num_poss_dels == 0) {
		cerr<<"Number of possible preconditions/effects unspecified."<<endl;
		exit(1);
	}


	// Set up seed for the generator
	generator.seed(static_cast<unsigned int>(std::time(0)));		// Initialize seed using the current time

	// Read the input domain
	string domain_str = read_domain(cl.path + cl.input_domain);

	// Parse domain string
	DomainStruct domain_structure = parse_domain_string(domain_str);

#ifdef DEBUG_MAIN
	cout<<"=== ORIGINAL DOMAIN ==="<<endl;
	domain_structure.print();
#endif

	for (size_t num_poss_pres = 0; num_poss_pres <= cl.num_poss_pres; num_poss_pres++)
		for (size_t num_poss_adds = 0; num_poss_adds <= cl.num_poss_adds; num_poss_adds++)
			for (size_t num_poss_dels = 0; num_poss_dels <= cl.num_poss_dels; num_poss_dels++) {

				if (num_poss_pres + num_poss_adds + num_poss_dels == 0)
					continue;

				for (size_t i = 0; i < cl.num_ouput_domains; i++) {

					DomainStruct output_domain_structure = domain_structure;
					// Make the domain incomplete
					make_domain_incomplete(output_domain_structure, num_poss_pres, num_poss_adds, num_poss_dels);

#ifdef DEBUG_MAIN
					cout<<"=== INCOMPLETE DOMAIN ==="<<endl;
					domain_structure.print();
#endif

					// Get the whole incomplete domain in string, and write it to output file
					string incomplete_domain_str = make_incomplete_domain_string(output_domain_structure);

					// Write the incomplete domain
					stringstream ss;
					ss<<num_poss_pres<<"p_"<<num_poss_adds<<"a_"<<num_poss_dels<<"d."<<i;
					string output = cl.input_domain + "." + ss.str();

					string filename = cl.path + output;
					ofstream f(filename.c_str());
					f<<incomplete_domain_str;
					f.close();
				}
			}
}

/**************************************************************************
 * FUNCTION DEFINITONS
 **************************************************************************/

void trim_extra_spaces(string& s) {

	// Trim extra spaces between tokens
	size_t i = 0;
	while (i < s.size()) {
		if (s[i] == SPACE) {
			while (i + 1 < s.size() && s[i+1] == SPACE)
				s.erase(i+1, 1);
		}
		i++;
	}

	// Trim spaces at the left and right ends
	while (s.size() && s[0] == SPACE)
		s.erase(0, 1);

	while (s.size() && s[s.size()-1] == SPACE)
		s.erase(s.size()-1, 1);
}

string make_incomplete_domain_string(const DomainStruct& domain_structure) {
	string result;

	result += domain_structure.before_action_str;

	for (size_t i=0;i<domain_structure.action_structures.size();i++) {
		const ActionStruct& a = domain_structure.action_structures[i];
		result += OPEN;

		result += a.before_pre_str;
		if (result[result.length()-1] != '\n')
			result += "\n";

		// Known preconditions
		result += string(PRECONDITION_START_STR) + "\n";
		int tab = 1;
		result += string(tab, '\t') + "(and\n";

		for (size_t j = 0; j < a.known_pres.size(); j++) {
			result += string(tab+1, '\t') + a.known_pres[j] + "\n";
		}

		result += string(tab, '\t') + ")\n";	// for "(and"

		// Possible preconditions
		result += string(POSSIBLE_PRECONDITION_START_STR) + "\n";
		tab = 1;
		result += string(tab, '\t') + "(and\n";

		for (size_t j = 0; j < a.poss_pres.size(); j++) {
			result += string(tab+1, '\t') + a.poss_pres[j] + "\n";
		}

		result += string(tab, '\t') + ")\n";	// for "(and"

		// Known effects
		result += string(EFFECT_START_STR) + "\n";
		tab = 1;
		result += string(tab, '\t') + "(and\n";

		for (size_t j = 0; j < a.known_adds.size(); j++) {
			result += string(tab+1, '\t') + a.known_adds[j] + "\n";
		}

		for (size_t j = 0; j < a.known_dels.size(); j++) {
			result += string(tab+1, '\t') + a.known_dels[j] + "\n";
		}

		result += string(tab, '\t') + ")\n";	// for "(and"

		// Possible effects
		result += string(POSSIBLE_EFFECT_START_STR) + "\n";
		tab = 1;
		result += string(tab, '\t') + "(and\n";

		for (size_t j = 0; j < a.poss_adds.size(); j++) {
			result += string(tab+1, '\t') + a.poss_adds[j] + "\n";
		}

		for (size_t j = 0; j < a.poss_dels.size(); j++) {
			result += string(tab+1, '\t') + a.poss_dels[j] + "\n";
		}

		result += string(tab, '\t') + ")\n";	// for "(and"

		result += string(CLOSE) + "\n\n";	// end of this action
	}

	result += CLOSE;	// end of domain

	return result;
}

template<class T> void shuffle(vector<T>& a) {
	boost::random::uniform_int_distribution<size_t> int_dist;
	for (size_t i = 0; i < a.size(); i++) {
		// Randomly choose a position from "i" to a.size()-1
		size_t pos = int_dist(generator, boost::random::uniform_int_distribution<size_t>::param_type(i, a.size()-1));

		// Swap a[pos] and a[i]
		if (pos != i) {
			T tmp = a[pos];
			a[pos] = a[i];
			a[i] = tmp;
		}
	}
}

void make_domain_incomplete(DomainStruct& domain_structure, int num_poss_pres, int num_poss_adds, int num_poss_dels) {

//#define DEBUG_MAKE_DOMAIN_INCOMPLE

	// All preconditions and effects of actions, associated with actions' id
	vector<pair<size_t, string> > all_pres, all_adds, all_dels;
	for (size_t i=0;i<domain_structure.action_structures.size(); i++) {

		const ActionStruct& a = domain_structure.action_structures[i];

		for (size_t j = 0; j < a.pres.size(); j++) {
			all_pres.push_back(make_pair(a.id, a.pres[j]));
		}

		for (size_t j = 0; j < a.adds.size(); j++)
			all_adds.push_back(make_pair(a.id, a.adds[j]));

		for (size_t j = 0; j < a.dels.size(); j++)
			all_dels.push_back(make_pair(a.id, a.dels[j]));
	}

	// Shuffle them all
	shuffle(all_pres);
	shuffle(all_adds);
	shuffle(all_dels);

	// Now randomly choose possible preconditions and effects
	vector<pair<size_t, string> > known_pres, poss_pres;
	break_arrays(all_pres, poss_pres, known_pres, num_poss_pres);

	vector<pair<size_t, string> > known_adds, poss_adds;
	break_arrays(all_adds, poss_adds, known_adds, num_poss_adds);

	vector<pair<size_t, string> > known_dels, poss_dels;
	break_arrays(all_dels, poss_dels, known_dels, num_poss_dels);

	// Put back these known and possible preconditions and effects back into the domain structure
	for (size_t i=0;i<domain_structure.action_structures.size(); i++) {

		ActionStruct& a = domain_structure.action_structures[i];

		for (size_t j=0;j<known_pres.size();j++) {
			size_t id = known_pres[j].first;
			string pre = known_pres[j].second;
			if (id == a.id)
				a.known_pres.push_back(pre);
		}

		for (size_t j=0;j<poss_pres.size();j++) {
			size_t id = poss_pres[j].first;
			string pre = poss_pres[j].second;
			if (id == a.id)
				a.poss_pres.push_back(pre);
		}

		for (size_t j=0;j<known_adds.size();j++) {
			size_t id = known_adds[j].first;
			string add = known_adds[j].second;
			if (id == a.id)
				a.known_adds.push_back(add);
		}

		for (size_t j=0;j<poss_adds.size();j++) {
			size_t id = poss_adds[j].first;
			string add = poss_adds[j].second;
			if (id == a.id)
				a.poss_adds.push_back(add);
		}

		for (size_t j=0;j<known_dels.size();j++) {
			size_t id = known_dels[j].first;
			string del = known_dels[j].second;
			if (id == a.id)
				a.known_dels.push_back(del);
		}

		for (size_t j=0;j<poss_dels.size();j++) {
			size_t id = poss_dels[j].first;
			string del = poss_dels[j].second;
			if (id == a.id)
				a.poss_dels.push_back(del);
		}

#ifdef DEBUG_MAKE_DOMAIN_INCOMPLE
		assert(a.poss_pres.size() + a.known_pres.size() == a.pres.size());
		assert(a.poss_adds.size() + a.known_adds.size() == a.adds.size());
		assert(a.poss_dels.size() + a.known_dels.size() == a.dels.size());
#endif

	}

#ifdef DEBUG_MAKE_DOMAIN_INCOMPLE
	size_t num_poss_pres = 0;
	size_t num_poss_adds = 0;
	size_t num_poss_dels = 0;

	for (size_t i=0;i<domain_structure.action_structures.size(); i++) {
		const ActionStruct& a = domain_structure.action_structures[i];
		num_poss_pres += a.poss_pres.size();
		num_poss_adds += a.poss_adds.size();
		num_poss_dels += a.poss_dels.size();
	}

	assert(num_poss_pres == cmd.num_poss_pres);
	assert(num_poss_adds == cmd.num_poss_adds);
	assert(num_poss_dels == cmd.num_poss_dels);
#endif

	// Now mix up the possible preconditions with delete effects not in preconditions,
	// and the possible delete effects with preconditions not in delete list
	// Those possible preconditions and delete effects that are "withdrawn" will be
	// put back into the known list
	boost::random::uniform_int_distribution<size_t> int_dist;
	for (size_t i=0;i<domain_structure.action_structures.size(); i++) {
		ActionStruct& a = domain_structure.action_structures[i];

		//
		// Possible preconditions and delete effects not in preconditions
		//
		if (a.poss_pres.size() && a.dels_minus_pres.size()) {

			shuffle(a.dels_minus_pres);

			// The number to be swapped
			size_t min_sz = (a.poss_pres.size() < a.dels_minus_pres.size()) ? a.poss_pres.size() : a.dels_minus_pres.size();
			size_t n = int_dist(generator, boost::random::uniform_int_distribution<size_t>::param_type(0, min_sz));

			// Choose n elements from the two lists to swap
			vector<size_t> l1 = sample_k(a.poss_pres.size(), n);
			vector<size_t> l2= sample_k(a.dels_minus_pres.size(), n);

			// Replace possible precondition j-th with the delete effect j-th
			// And put it back to the known precondition list
			for (size_t j=0;j<n;j++) {
				size_t pos1 = l1[j];	// a.poss_pres[pos1] is to be replaced
				size_t pos2 = l2[j];	// with a.dels_minus_pres[pos2]

				// The delete effect "(not (...) )" to be used instead
				string del_eff = a.dels_minus_pres[pos2];

				// Remove "(<space> not.."
				size_t begin_pos = del_eff.find_first_of(OPEN);
				begin_pos = del_eff.find_first_of(OPEN, begin_pos + 1);

				size_t end_pos = del_eff.find_first_of(CLOSE, begin_pos);
				end_pos++;

				// Put back the possible precondition into the known precondition list
				a.known_pres.push_back(a.poss_pres[pos1]);

				// Replace this possible precondition with the delete effect
				a.poss_pres[pos1] = del_eff.substr(begin_pos, end_pos - begin_pos);
			}
		}

		//
		// Possible delete effects and preconditions not in delete list
		//
		if (a.poss_dels.size() && a.pres_minus_dels.size()) {
			shuffle(a.pres_minus_dels);

			// The number to be swapped
			size_t min_sz = (a.poss_dels.size() < a.pres_minus_dels.size()) ? a.poss_dels.size() : a.pres_minus_dels.size();
			size_t n = int_dist(generator, boost::random::uniform_int_distribution<size_t>::param_type(0, min_sz));

			// Choose n elements from the two lists to swap
			vector<size_t> l1 = sample_k(a.poss_dels.size(), n);
			vector<size_t> l2= sample_k(a.pres_minus_dels.size(), n);

			// Replace possible delete effect j-th with the precondition j-th
			// And put it back to the known delete list
			for (size_t j=0;j<n;j++) {
				size_t pos1 = l1[j];
				size_t pos2 = l2[j];

				// The new possible delete to be used
				string new_poss_eff = string("(not ") + a.pres_minus_dels[pos2] + string(")");

				// Put back the possible delete into the known delete list
				a.known_dels.push_back(a.poss_dels[pos1]);

				// Replace this possible delete with the new one
				a.poss_dels[pos1] = new_poss_eff;
			}
		}
	}

#ifdef DEBUG_MAKE_DOMAIN_INCOMPLE
	num_poss_pres = 0;
	num_poss_adds = 0;
	num_poss_dels = 0;

	for (size_t i=0;i<domain_structure.action_structures.size(); i++) {
		const ActionStruct& a = domain_structure.action_structures[i];
		num_poss_pres += a.poss_pres.size();
		num_poss_adds += a.poss_adds.size();
		num_poss_dels += a.poss_dels.size();
	}

	assert(num_poss_pres == cmd.num_poss_pres);
	assert(num_poss_adds == cmd.num_poss_adds);
	assert(num_poss_dels == cmd.num_poss_dels);
#endif

}

DomainStruct parse_domain_string(string domain_string) {

	DomainStruct domain_structure;

	// Find the position of the first "(<space>:action"
	size_t first_action_pos = domain_string.find(ACTION_START_STR);
	first_action_pos = domain_string.find_last_of(OPEN, first_action_pos);

	// Get the string from the beginning to before "first_action_pos"
	domain_structure.before_action_str = domain_string.substr(0, first_action_pos);

	// Parse action
	vector<ActionStruct> action_structures;
	vector<string> action_strings = extract_action_strings(domain_string);
	for (size_t i=0;i<action_strings.size();i++) {
		ActionStruct a;

		a.id = i;

		// Find the position of ":precondition"
		size_t pre_pos = action_strings[i].find(PRECONDITION_START_STR);

		// Record the string from the beginning of the action to before "pre_pos"
		a.before_pre_str = action_strings[i].substr(0, pre_pos);

		// Extract preconditions and effects
		a.pres = extract_precondition_strings(action_strings[i]);
		extract_effect_strings(action_strings[i], a.adds, a.dels);

		// Extract preconditions not in delete list and vice versa
		extract_preconditions_minus_deletes(a.pres, a.dels, a.pres_minus_dels);
		extract_deletes_minus_preconditions(a.pres, a.dels, a.dels_minus_pres);

		// Store this action structure
		action_structures.push_back(a);
	}

	// Store action structures
	domain_structure.action_structures = action_structures;

	return domain_structure;
}

vector<size_t> sample_k(size_t n, size_t k) {
	assert(k >= 0 && k <= n);

	// The resulting numbers
	vector<size_t> result;

	if (k == 0)
		return result;

	if (k == n) {
		for (size_t i=0;i<n;i++)
			result.push_back(i);
		return result;
	}


	// Array of numbers [0..n-1]
	vector<size_t> a(n);
	for (size_t i=0;i<n;i++)
		a[i] = i;

	// Start to select
	boost::random::uniform_int_distribution<size_t> int_dist;
	for (size_t i = 0; i < k; i++) {
		// Choose a random position from "i" to "n-1" to put into "result[i]"
		size_t pos = int_dist(generator, boost::random::uniform_int_distribution<size_t>::param_type(i, n-1));

		// Put the number at position "pos" into the result
		result.push_back(a[pos]);

		// Swap a[pos] with a[i] so that a[pos] won't be considered in the next selection
		if (pos != i) {
			size_t tmp = a[pos];
			a[pos] = a[i];
			a[i] = tmp;
		}
	}
	return result;
}

template<class T>
void break_arrays(const vector<T>& a, vector<T>& a1, vector<T>& a2, size_t k) {
	if (k == 0) {
		a1.clear();
		a2 = a;
	}
	else if (k >= a.size()) {
		a1 = a;
		a2.clear();
	}
	else {
		// Randomly select k positions from 0 to a.size()-1
		vector<size_t> selected_positions = sample_k(a.size(), k);

		// Break "a" into two parts
		for (size_t i = 0; i < a.size(); i++) {
			if (find(selected_positions.begin(), selected_positions.end(), i) != selected_positions.end())
				a1.push_back(a[i]);
			else
				a2.push_back(a[i]);
		}
	}
}

string make_incomplete_action(const string& action_string, size_t num_poss_pres, size_t num_poss_adds, size_t num_poss_dels) {
	string incomplete_action_string;

	// Extract preconditions and effects
	vector<string> preconditions = extract_precondition_strings(action_string);
	vector<string> add_effects, delete_effects;
	extract_effect_strings(action_string, add_effects, delete_effects);

	// Extract preconditions not in delete list, and vice versa
	vector<string> pres_minus_dels, dels_minus_pres;
	extract_preconditions_minus_deletes(preconditions, delete_effects, pres_minus_dels);
	extract_deletes_minus_preconditions(preconditions, delete_effects, dels_minus_pres);

	// Break precondition and effect strings into known and possible parts
	vector<string> known_preconditions, possible_preconditions;
	vector<string> known_adds, possible_adds;
	vector<string> known_deletes, possible_deletes;
	break_arrays(preconditions, possible_preconditions, known_preconditions, num_poss_pres);
	break_arrays(add_effects, possible_adds, known_adds, num_poss_adds);
	break_arrays(delete_effects, possible_deletes, known_deletes, num_poss_dels);

	// Find the position of the precondition
	size_t pre_pos = action_string.find(PRECONDITION_START_STR);

	// Now start to make the resulting action string
	// First, the part from the beginning to before "pos" is unchanged
	incomplete_action_string = action_string.substr(0, pre_pos);

	// The known precondition part
	incomplete_action_string += string(PRECONDITION_START_STR) + "\n";
	int tab = 1;

	incomplete_action_string += string(tab, '\t') + "(and\n";

	for (size_t i=0;i<known_preconditions.size();i++)
		incomplete_action_string += string(tab + 1, '\t') + known_preconditions[i] + "\n";

	incomplete_action_string += string(tab,'\t') + ")\n";	// for "(and "

	// Possible preconditions
	incomplete_action_string += string(POSSIBLE_PRECONDITION_START_STR) + "\n";
	tab = 1;

	incomplete_action_string += string(tab, '\t') + "(and\n";

	for (size_t i=0;i<possible_preconditions.size();i++)
		incomplete_action_string += string(tab + 1, '\t') + possible_preconditions[i] + "\n";

	incomplete_action_string += string(tab,'\t') + ")\n";	// for "(and "

	// Known effects
	incomplete_action_string += string(EFFECT_START_STR) + "\n";
	tab = 1;

	incomplete_action_string += string(tab, '\t') + "(and\n";

	for (size_t i=0;i<known_adds.size();i++)
		incomplete_action_string += string(tab + 1, '\t') + known_adds[i] + "\n";

	for (size_t i=0;i<known_deletes.size();i++)
		incomplete_action_string += string(tab + 1, '\t') + known_deletes[i] + "\n";

	incomplete_action_string += string(tab,'\t') + ")\n";	// for "(and "

	// Possible effects
	incomplete_action_string += string(POSSIBLE_EFFECT_START_STR) + "\n";
	tab = 1;

	incomplete_action_string += string(tab, '\t') + "(and\n";

	for (size_t i=0;i<possible_adds.size();i++)
		incomplete_action_string += string(tab + 1, '\t') + possible_adds[i] + "\n";

	for (size_t i=0;i<possible_deletes.size();i++)
		incomplete_action_string += string(tab + 1, '\t') + possible_deletes[i] + "\n";

	incomplete_action_string += string(tab,'\t') + ")\n";	// for "(and "


	return incomplete_action_string;
}


void extract_preconditions_minus_deletes(const vector<string>& pres, const vector<string>& dels, vector<string>& pres_minus_dels) {
	for (size_t i = 0; i < pres.size(); i++) {
		string s = string("(not ") + pres[i] + ")";
		if (find(dels.begin(), dels.end(), s) == dels.end())
			pres_minus_dels.push_back(pres[i]);
	}
}

void extract_deletes_minus_preconditions(const vector<string>& pres, const vector<string>& dels, vector<string>& dels_minus_pres) {
	for (size_t i=0;i<dels.size();i++) {
		size_t j = 0;
		for (; j < pres.size();j++) {
			string s = string("(not ") + pres[i] + ")";
			if (dels[i] == s)
				break;
		}

		if (j == pres.size())
			dels_minus_pres.push_back(dels[i]);
	}
}

bool process_command_line( int argc, char *argv[], CommandLine& cl) {

	while ( --argc && ++argv ) {
		if ( *argv[0] != '-') {
			return false;
		}

		string option(*argv);
		if (--argc && ++argv) {

			// Path
			if (option == "-P")
				cl.path = *argv;

			// Input domain
			if (option == "-i")
				cl.input_domain = *argv;

			// Number of output domains
			if (option == "-n")
				sscanf(*argv,"%d", &cl.num_ouput_domains);

			// Number of possible preconditions
			if (option == "-p")
				sscanf(*argv,"%d", &cl.num_poss_pres);

			// Number of possible adds
			if (option == "-a")
				sscanf(*argv,"%d", &cl.num_poss_adds);

			// Number of possible deletes
			if (option == "-d")
				sscanf(*argv,"%d", &cl.num_poss_dels);

		}

	}
	return true;
}

void extract_effect_strings(string action_string, vector<string>& adds, vector<string>& dels) {
	string effect_indicator_str(EFFECT_START_STR);

	// Find "begin_pos": the position after "and" in ":effect (and"
	size_t begin_pos = action_string.find(effect_indicator_str);
	assert(begin_pos != string::npos);

	begin_pos += effect_indicator_str.length();

	begin_pos = action_string.find_first_of(OPEN, begin_pos);
	assert(begin_pos);
	begin_pos = action_string.find("and", begin_pos);
	assert(begin_pos);
	begin_pos += string("and").length();

	// Find "end_pos": the end parenthesis in ":effect (and...)"
	size_t end_pos = action_string.find_last_of(CLOSE);	// NOTE: "action_string" must not have close parenthesis for ":action"

	string not_str(NOT_STR);
	size_t open_pos = begin_pos;
	while (open_pos < end_pos) {

		// Find the position of the "(" for the next effect
		open_pos = action_string.find_first_of(OPEN, open_pos);
		if (open_pos == string::npos || open_pos >= end_pos)
			break;

		// Find the position of "not", if any
		size_t not_pos = action_string.find(not_str, open_pos);

		// Find the position of ")" after the above "("
		size_t close_pos = action_string.find_first_of(CLOSE, open_pos);

		// Check if this is an add or delete effect
		if (not_pos != string::npos && not_pos < close_pos) {

			// Delete effect!

			// Now, "open_pos" is the position of "(" in "( <space> not"
			// "close_pos" is the first ")" after the above "("

			// Find the next ")" AFTER the above close parenthesis, i.e the last ")" in "( <space> not ... ) <space> )"
			close_pos = action_string.find_first_of(CLOSE, close_pos + 1);
			close_pos++;

			// Now record this delete effect, including the two parenthesis
			string del = action_string.substr(open_pos, close_pos - open_pos);
			dels.push_back(del);

			// Now move to the position AFTER the ")" of the "(not... (..)"
			open_pos = close_pos;
		}
		else {

			// Add effect!

			close_pos++;

			// Now record this add effect, including the two parenthesis
			string add = action_string.substr(open_pos, close_pos - open_pos);
			adds.push_back(add);

			// Move to the next effect
			open_pos = close_pos;
		}
	}

}

vector<string> extract_precondition_strings(string action_string) {
	vector<string> precondition_strings;
	string precondition_indicator_str(PRECONDITION_START_STR);

	// Find "begin_pos": the position after "and" in ":precondition (and"
	size_t begin_pos = action_string.find(precondition_indicator_str);
	assert(begin_pos != string::npos);

	begin_pos += precondition_indicator_str.length();

	begin_pos = action_string.find_first_of(OPEN, begin_pos);
	assert(begin_pos);
	begin_pos = action_string.find("and", begin_pos);
	assert(begin_pos);
	begin_pos += string("and").length();

	// Find "end_pos": the end parenthesis in ":precondition (...)"
	size_t end_pos = action_string.find(EFFECT_START_STR, begin_pos);
	if (end_pos != string::npos) {
		end_pos = action_string.find_last_of(CLOSE, end_pos - 1);
	}
	else {
		end_pos = action_string.find_last_of(CLOSE, action_string.length()-1);
	}

	size_t open_pos = begin_pos;
	while (open_pos < end_pos) {

		// Find the position of the "(" for the next precondition
		open_pos = action_string.find_first_of(OPEN, open_pos);
		if (open_pos == string::npos || open_pos >= end_pos)
			break;

		// Find the position AFTER the ")" for this precondition
		size_t close_pos = action_string.find_first_of(CLOSE, open_pos);
		close_pos++;

		// Now record this precondition, including the two parenthesis
		string precondition = action_string.substr(open_pos, close_pos - open_pos);
		assert(precondition.find_first_of(OPEN, 1) == string::npos);
		precondition_strings.push_back(precondition);

		// Move to the next precondition
		open_pos = close_pos;
	}

	return precondition_strings;
}

vector<string> extract_action_strings(string domain_str) {
	vector<string> action_strings;

	string action_indicator_str(ACTION_START_STR);

	// Each action string is in positions [begin, end) of the domain string
	size_t begin, end;

	// Find the first action
	begin = domain_str.find(action_indicator_str);
	while (begin != string::npos) {

		// Find "end": the position of the closing parenthesis of this action string
		size_t next_begin = domain_str.find(action_indicator_str, begin + 1);
		if (next_begin != string::npos) {
			end = domain_str.find_last_of(CLOSE, next_begin - 1);
		}
		else {
			// For the last action, ignore characters after the closing parenthesis
			size_t last_close = domain_str.find_last_of(CLOSE);
			end = domain_str.find_last_of(CLOSE, last_close - 1);
		}

		action_strings.push_back(domain_str.substr(begin, end - begin));

		// Consider next begin
		begin = next_begin;
	}

	return action_strings;
}

string read_domain(string domain_file) {
	string domain_str;
	char buf[MAX_LEN];
	ifstream f(domain_file.c_str());
	while (f.getline(buf, MAX_LEN)) {
		domain_str += buf;
		domain_str += "\n";
	}
	return domain_str;
}
