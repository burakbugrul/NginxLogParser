#ifndef NGINXPARSER_HPP
#define NGINXPARSER_HPP

#include <cstring>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <unordered_map>

using namespace std;

const int MAX_LINE = 500;

class NginxParser{
	
	int lines=0, valid_lines=0, cs_hits, videos=0, audios=0;
	int http_statuses[5] = {0,0,0,0,0};
	string input_file,output_file;
	unordered_map<int, int> bitrates;
	unordered_map<string, int> contents, user_agents, users;
	long long int total_data = 0;
	
	public:
		NginxParser(string input, string output);
		inline string findIP(char *line);
		inline int characterCount(char *line, int length, char character);
		inline int findCharacter(char *ar, int begin, int length, char character);
		void parse();
		void statistics();
};
#endif
