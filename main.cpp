#include <iostream>

#include "NginxParser.hpp"

using namespace std;

int main(int argc, char **argv){
	
	ios::sync_with_stdio(false); // For speed boost
	
	if( argc == 3 ){
		NginxParser parser(argv[1],argv[2]);
		parser.parse();
		parser.statistics();
	}
	else{
		
		string input_file, output_file;
		
		cerr << "Please enter path of the log file: ";
		cin >> input_file;
		cerr << "Please enter path of the output file: ";
		cin >> output_file;
		
		NginxParser parser(input_file,output_file);
		parser.parse();
		parser.statistics();
	}
	
	return 0;
}
