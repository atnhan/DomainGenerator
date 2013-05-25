//============================================================================
// Name        : DomainGenerator.cpp
// Author      : Tuan Nguyen
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cassert>
#include <ctime>
#include <algorithm>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>

using namespace std;

#define MAX_LEN	 256
#define TAB		"\t"
#define OPEN	"("
#define CLOSE	")"
#define ACTION_START_STR	":action"
#define PRECONDITION_START_STR	":precondition"
#define POSSIBLE_PRECONDITION_START_STR	 ":poss-precondition"
#define EFFECT_START_STR	":effect"
#define POSSIBLE_EFFECT_START_STR	":poss-effect"
#define NOT_STR		"not"
#define AND_STR		"and"

struct CommandLine {
	string path;
	string input_domain;
	string output_domain;

	size_t num_poss_pres;
	size_t num_poss_adds;
	size_t num_poss_dels;

	CommandLine() {
		path = "./";
		input_domain = "";
		output_domain = "";
		num_poss_adds = 0;
		num_poss_dels = 0;
		num_poss_pres = 0;
	}
	void print() {
		cout<<"Path: "<<path<<endl;
		cout<<"Input: "<<input_domain<<endl;
		cout<<"Output: "<<output_domain<<endl;
		cout<<"#Poss pres: "<<num_poss_pres<<endl;
		cout<<"#Poss adds: "<<num_poss_adds<<endl;
		cout<<"#Poss dels: "<<num_poss_dels<<endl;
	}
};

CommandLine gcmd_line;

// Random generator
typedef boost::mt19937 base_generator_type;
base_generator_type generator;

// A structure for actions with separate precondition and effect lists
struct ActionStruct {
	size_t id;
	string before_pre_str;	// String before the ":precondition"

	// Original preconditions, effects
	vector<string> pres;
	vector<string> adds;
	vector<string> dels;

	// Known/possible preconditions, effects
	vector<string> known_pres;
	vector<string> known_adds;
	vector<string> known_dels;

	vector<string> poss_pres;
	vector<string> poss_adds;
	vector<string> poss_dels;

	void print() {
		cout<<"ACTION "<<id<<":"<<endl<<endl;
		cout<<"before_pre_str: "<<endl<<before_pre_str<<endl<<endl;
		cout<<"Original preconditions:"<<endl<<endl;
		for (size_t i=0;i<pres.size();i++) {
			cout<<pres[i]<<endl;
		}
		cout<<endl;

		cout<<"Known preconditions:"<<endl<<endl;
		for (size_t i=0;i<known_pres.size();i++) {
			cout<<known_pres[i]<<endl;
		}
		cout<<endl;

		cout<<"Possible preconditions:"<<endl<<endl;
		for (size_t i=0;i<poss_pres.size();i++) {
			cout<<poss_pres[i]<<endl;
		}
		cout<<endl;

		cout<<"Known adds:"<<endl<<endl;
		for (size_t i=0;i<known_adds.size();i++) {
			cout<<known_adds[i]<<endl;
		}
		cout<<endl;

		cout<<"Possible adds:"<<endl<<endl;
		for (size_t i=0;i<poss_adds.size();i++) {
			cout<<poss_adds[i]<<endl;
		}
		cout<<endl;

		cout<<"Known deletes:"<<endl<<endl;
		for (size_t i=0;i<known_dels.size();i++) {
			cout<<known_dels[i]<<endl;
		}
		cout<<endl;

		cout<<"Possible deletes:"<<endl<<endl;
		for (size_t i=0;i<poss_dels.size();i++) {
			cout<<poss_dels[i]<<endl;
		}
		cout<<endl;


	}

};

struct DomainStruct {
	string before_action_str;	// String before the first "(<space>:action"
	vector<ActionStruct> action_structures;

	void print() {
		cout<<"before_action_str:"<<endl<<before_action_str<<endl;
		for (size_t i=0;i<action_structures.size();i++) {
			action_structures[i].print();
			cout<<endl;
		}
	}
};

/**************************************************************************
 * FUNCTIONS
 **************************************************************************/

// Read the entire domain into a string
string read_domain(string domain_file);

// Extract all action strings
vector<string> extract_action_strings(string domain_str);

// Extract all precondition strings of from an action string
vector<string> extract_precondition_strings(string action_string);

// Extract add and delete effect strings from an action string
void extract_effect_strings(string action_string, vector<string>& adds, vector<string>& dels);

// Command line processing
bool process_command_line( int argc, char *argv[], CommandLine& cl );

// Make an incomplete action string from an action string
string make_incomplete_action(const string& action_string, size_t num_poss_pres, size_t num_poss_adds, size_t num_poss_dels);

// Randomly break string array "a" of n elements into two parts "a1" and "a2" with specified size k and n-k
template<class T>
void break_arrays(const vector<T>& a, vector<T>& a1, vector<T>& a2, size_t k);

// Randomly select k number from 0 to n-1
vector<size_t> sample_k(size_t n, size_t k);

// Parse the domain string to put it into structure
// Return the number of actions
DomainStruct parse_domain_string(string domain_string);

// Make the domain incomplete
void make_domain_incomplete(DomainStruct& domain_structure, const CommandLine& cmd);

// Shuffle arrays
template<class T> void shuffle(vector<T>& a);

// Make domain string from structure
string make_incomplete_domain_string(const DomainStruct& domain_structure);

/**************************************************************************
 * Main
 **************************************************************************/
int main(int argc, char *argv[]) {

	// Process the command line
	CommandLine cl;
	process_command_line(argc, argv, cl);
	if (cl.input_domain == "" || cl.output_domain == "" || cl.input_domain == cl.output_domain) {
		cerr<<"Invalid input and/or output domains."<<endl;
		exit(1);
	}

	// Set up seed for the generator
	generator.seed(static_cast<unsigned int>(std::time(0)));		// Initialize seed using the current time

	// Read the input domain
	string domain_str = read_domain(cl.path + cl.input_domain);

	// Parse domain string
	DomainStruct domain_structure = parse_domain_string(domain_str);
	cout<<"=== ORIGINAL DOMAIN ==="<<endl;
	domain_structure.print();

	// Make the domain incomplete
	make_domain_incomplete(domain_structure, cl);
	cout<<"=== INCOMPLETE DOMAIN ==="<<endl;
	domain_structure.print();

	// Get the whole incomplete domain in string, and write it to output file
	string incomplete_domain_str = make_incomplete_domain_string(domain_structure);

	string filename = cl.path + cl.output_domain;
	ofstream f(filename.c_str());
	f<<incomplete_domain_str;
	f.close();
}

/**************************************************************************
 * FUNCTION DEFINITONS
 **************************************************************************/

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

void make_domain_incomplete(DomainStruct& domain_structure, const CommandLine& cmd) {

	// All preconditions and effects of actions, associated with actions' id
	vector<pair<size_t, string> > all_pres, all_adds, all_dels;
	for (size_t i=0;i<domain_structure.action_structures.size(); i++) {

		const ActionStruct& a = domain_structure.action_structures[i];

		for (size_t j = 0; j < a.pres.size(); j++)
			all_pres.push_back(make_pair(a.id, a.pres[j]));

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
	break_arrays(all_pres, poss_pres, known_pres, cmd.num_poss_pres);

	vector<pair<size_t, string> > known_adds, poss_adds;
	break_arrays(all_adds, poss_adds, known_adds, cmd.num_poss_adds);

	vector<pair<size_t, string> > known_dels, poss_dels;
	break_arrays(all_dels, poss_dels, known_dels, cmd.num_poss_dels);

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
	}
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

		// Store this action structure
		action_structures.push_back(a);
	}

	// Store action structures
	domain_structure.action_structures = action_structures;

	return domain_structure;
}

vector<size_t> sample_k(size_t n, size_t k) {
	assert(k > 0 && k < n);

	// Array of numbers [0..n-1]
	vector<size_t> a(n);
	for (size_t i=0;i<n;i++)
		a[i] = i;

	// The resulting numbers
	vector<size_t> result;

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

			// Output domain
			if (option == "-o")
				cl.output_domain = *argv;

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
