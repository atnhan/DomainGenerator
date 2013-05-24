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
using namespace std;

#define MAX_LEN	 256
#define OPEN	"("
#define CLOSE	")"
#define ACTION_START_STR	":action"
#define PRECONDITION_START_STR	":precondition"
#define EFFECT_START_STR	":effect"
#define NOT_STR		"not"
#define AND_STR		"and"

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

/**************************************************************************
 * Main
 **************************************************************************/
int main(void) {
	string domain_str = read_domain("zeno.pddl");
	vector<string> action_strings = extract_action_strings(domain_str);
	for (size_t i=0;i<action_strings.size();i++) {
		cout<<"Action "<<i+1<<": "<<endl;
		cout<<action_strings[i]<<endl;

		cout<<"Preconditions:"<<endl;
		vector<string> preconditions = extract_precondition_strings(action_strings[i]);
		for (size_t j=0;j<preconditions.size(); j++) {
			cout<<j<<": "<<preconditions[j]<<endl;
		}

		vector<string> add_effects, delete_effects;
		extract_effect_strings(action_strings[i], add_effects, delete_effects);
		cout<<"Add effects:"<<endl;
		for (size_t j=0;j<add_effects.size(); j++) {
			cout<<j<<": "<<add_effects[j]<<endl;
		}
		cout<<"Delete effects:"<<endl;
		for (size_t j=0;j<delete_effects.size(); j++) {
			cout<<j<<": "<<delete_effects[j]<<endl;
		}

		cout<<endl<<endl;
	}
}

/**************************************************************************
 * FUNCTION DEFINITONS
 **************************************************************************/

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
