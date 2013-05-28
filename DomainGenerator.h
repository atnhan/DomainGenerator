/*
 * DomainGenerator.h
 *
 *  Created on: May 27, 2013
 *      Author: tanguye1
 */

#ifndef DOMAINGENERATOR_H_
#define DOMAINGENERATOR_H_

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cassert>
#include <ctime>
#include <algorithm>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>


#define MAX_LEN	 256
#define TAB		"\t"
#define OPEN	"("
#define CLOSE	")"
#define SPACE ' '
#define ACTION_START_STR	":action"
#define PRECONDITION_START_STR	":precondition"
#define POSSIBLE_PRECONDITION_START_STR	 ":poss-precondition"
#define EFFECT_START_STR	":effect"
#define POSSIBLE_EFFECT_START_STR	":poss-effect"
#define NOT_STR		"not"
#define AND_STR		"and"

struct CommandLine {
	std::string path;
	std::string input_domain;

	size_t num_poss_pres;
	size_t num_poss_adds;
	size_t num_poss_dels;

	size_t num_ouput_domains;

	CommandLine() {
		path = "./";
		input_domain = "";
		num_ouput_domains = 0;
		num_poss_adds = 0;
		num_poss_dels = 0;
		num_poss_pres = 0;
	}
	void print() {
		std::cout<<"Path: "<<path<<std::endl;
		std::cout<<"Input: "<<input_domain<<std::endl;
		std::cout<<"Number of ouput domains: "<<num_ouput_domains<<std::endl;
		std::cout<<"#Poss pres: "<<num_poss_pres<<std::endl;
		std::cout<<"#Poss adds: "<<num_poss_adds<<std::endl;
		std::cout<<"#Poss dels: "<<num_poss_dels<<std::endl;
	}
};

CommandLine gcmd_line;

// Random generator
typedef boost::mt19937 base_generator_type;
base_generator_type generator;

// A structure for actions with separate precondition and effect lists
struct ActionStruct {
	size_t id;
	std::string before_pre_str;	// std::string before the ":precondition"

	// Original preconditions, effects
	std::vector<std::string> pres;
	std::vector<std::string> adds;
	std::vector<std::string> dels;

	// Original preconditions not in delete effects, and vice versa
	std::vector<std::string> pres_minus_dels;
	std::vector<std::string> dels_minus_pres;

	// Known/possible preconditions, effects
	std::vector<std::string> known_pres;
	std::vector<std::string> known_adds;
	std::vector<std::string> known_dels;

	std::vector<std::string> poss_pres;
	std::vector<std::string> poss_adds;
	std::vector<std::string> poss_dels;

	void print() {
		std::cout<<"ACTION "<<id<<":"<<std::endl<<std::endl;
		std::cout<<"before_pre_str: "<<std::endl<<before_pre_str<<std::endl<<std::endl;

		std::cout<<"Original preconditions:"<<std::endl<<std::endl;
		for (size_t i=0;i<pres.size();i++) {
			std::cout<<pres[i]<<std::endl;
		}
		std::cout<<std::endl;

		std::cout<<"Preconditions minus deletes:"<<std::endl<<std::endl;
		for (size_t i=0;i<pres_minus_dels.size();i++) {
			std::cout<<pres_minus_dels[i]<<std::endl;
		}
		std::cout<<std::endl;


		std::cout<<"Original adds:"<<std::endl<<std::endl;
		for (size_t i=0;i<adds.size();i++) {
			std::cout<<adds[i]<<std::endl;
		}
		std::cout<<std::endl;

		std::cout<<"Original deletes:"<<std::endl<<std::endl;
		for (size_t i=0;i<dels.size();i++) {
			std::cout<<dels[i]<<std::endl;
		}
		std::cout<<std::endl;

		std::cout<<"Deletes minus preconditions:"<<std::endl<<std::endl;
		for (size_t i=0;i<dels_minus_pres.size();i++) {
			std::cout<<dels_minus_pres[i]<<std::endl;
		}
		std::cout<<std::endl;


		std::cout<<"Known preconditions:"<<std::endl<<std::endl;
		for (size_t i=0;i<known_pres.size();i++) {
			std::cout<<known_pres[i]<<std::endl;
		}
		std::cout<<std::endl;

		std::cout<<"Possible preconditions:"<<std::endl<<std::endl;
		for (size_t i=0;i<poss_pres.size();i++) {
			std::cout<<poss_pres[i]<<std::endl;
		}
		std::cout<<std::endl;

		std::cout<<"Known adds:"<<std::endl<<std::endl;
		for (size_t i=0;i<known_adds.size();i++) {
			std::cout<<known_adds[i]<<std::endl;
		}
		std::cout<<std::endl;

		std::cout<<"Possible adds:"<<std::endl<<std::endl;
		for (size_t i=0;i<poss_adds.size();i++) {
			std::cout<<poss_adds[i]<<std::endl;
		}
		std::cout<<std::endl;

		std::cout<<"Known deletes:"<<std::endl<<std::endl;
		for (size_t i=0;i<known_dels.size();i++) {
			std::cout<<known_dels[i]<<std::endl;
		}
		std::cout<<std::endl;

		std::cout<<"Possible deletes:"<<std::endl<<std::endl;
		for (size_t i=0;i<poss_dels.size();i++) {
			std::cout<<poss_dels[i]<<std::endl;
		}
		std::cout<<std::endl;

	}

};

struct DomainStruct {
	std::string before_action_str;	// std::string before the first "(<space>:action"
	std::vector<ActionStruct> action_structures;

	void print() {
		std::cout<<"before_action_str:"<<std::endl<<before_action_str<<std::endl;
		for (size_t i=0;i<action_structures.size();i++) {
			action_structures[i].print();
			std::cout<<std::endl;
		}
	}
};

/**************************************************************************
 * FUNCTIONS
 **************************************************************************/

// Main
int main(int argc, char *argv[]);

// Read the entire domain into a std::string
std::string read_domain(std::string domain_file);

// Extract all action strings
std::vector<std::string> extract_action_strings(std::string domain_str);

// Extract all precondition strings of from an action std::string
std::vector<std::string> extract_precondition_strings(std::string action_string);

// Extract add and delete effect strings from an action std::string
void extract_effect_strings(std::string action_string, std::vector<std::string>& adds, std::vector<std::string>& dels);

// Extract preconditions not in delete list
// Note: the input delete effect strings contain "not"
void extract_preconditions_minus_deletes(const std::vector<std::string>& pres, const std::vector<std::string>& dels, std::vector<std::string>& pres_minus_dels);

// Extract delete effects not in delete list
// Note: the output delete effect strings contain "not"
void extract_deletes_minus_preconditions(const std::vector<std::string>& pres, const std::vector<std::string>& dels, std::vector<std::string>& dels_minus_pres);

// Command line processing
bool process_command_line( int argc, char *argv[], CommandLine& cl );

// Make an incomplete action std::string from an action std::string
std::string make_incomplete_action(const std::string& action_string, size_t num_poss_pres, size_t num_poss_adds, size_t num_poss_dels);

// Randomly break std::string array "a" of n elements into two parts "a1" and "a2" with specified size k and n-k
template<class T>
void break_arrays(const std::vector<T>& a, std::vector<T>& a1, std::vector<T>& a2, size_t k);

// Randomly select k number from 0 to n-1
std::vector<size_t> sample_k(size_t n, size_t k);

// Parse the domain std::string to put it into structure
// Return the number of actions
DomainStruct parse_domain_string(std::string domain_string);

// Make the domain incomplete
void make_domain_incomplete(DomainStruct& domain_structure, int num_poss_pres, int num_poss_adds, int num_poss_dels);

// Shuffle arrays
template<class T> void shuffle(std::vector<T>& a);

// Make domain std::string from structure
std::string make_incomplete_domain_string(const DomainStruct& domain_structure);

// Remove any extra space in a std::string (i.e., words in the output std::string separated with exactly one space only)
void trim_extra_spaces(std::string& s);



#endif /* DOMAINGENERATOR_H_ */
