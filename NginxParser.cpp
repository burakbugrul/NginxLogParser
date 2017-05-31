#include "NginxParser.hpp"

NginxParser::NginxParser(string input, string output){
	input_file = input;
	output_file = output;
}

inline string NginxParser::findIP(char *line){
	
	int number=0;
	int point_count = 0;
	string result;
	
	for( int i=0 ; line[i] != ' ' && line[i] != '-' && line[i] != '\0' ; i++ ){
		
		if( isdigit(line[i]) )
			number = number*10 + (line[i]-'0');
		else if( number > 255 )
			return "-1";
		else if( line[i] == '.' ){
			number = 0;
			point_count++;
		}
		else
			return "-1";
		
		result += line[i];
	}
	
	if( point_count == 3 )
		return result;
	return "-1";
}

inline int NginxParser::characterCount(char *line, int length, char character){
	
	int result = 0;
	
	for( int i=0 ; i<length ; i++ )
		result += (line[i] == character);
	
	return result;
}

inline int NginxParser::findCharacter(char *line, int begin, int length, char character){
	
	for( int i=begin ; i<length ; i++ )
		if( line[i] == character )
			return i;
	
	return length;
}

void NginxParser::parse(){
	
	ifstream input(input_file);
	int length, left, right, begin, end, bitrate, ten, http_status, sent_bytes;
	char line[MAX_LINE], type;
	string ip, id, user_agent;
	bool flag = true, cs_hit, cs_flag;
	
	while( input.getline(line,MAX_LINE,'\n') ){
		
		lines++;
		length = strlen(line);
		
		if( (ip = findIP(line)) == "-1" )
			continue;
		if( characterCount(line,length,'"') != 10 )
			continue;
		
		right = 0;
		flag = true;
		
		for( int i=0 ; i<5 ; i++ ){
			
			left = findCharacter(line,right,length,'"');
			right = findCharacter(line,left+1,length,'"');
			
			if( right-left == 2 ) // Empty parts
				continue;
			
			else if( strncmp(line+left+1,"GET",3)==0 || 
				strncmp(line+left+1,"POST",4)==0 || 
				strncmp(line+left+1,"HEAD",4)==0 || 
				strncmp(line+left+1,"PUT",3)==0 || 
				strncmp(line+left+1,"DELETE",6)==0 || 
				strncmp(line+left+1,"OPTIONS",7)==0 || 
				strncmp(line+left+1,"CONNECT",7)==0 ){ // Url part
				
				end = left;
				for( int j=0 ; j<2; j++ ){ // Finding ID
					begin = findCharacter(line,end+1,right,'/');
					end = findCharacter(line,begin+1,right,'/');
				}
				
				id = ""; // Saving ID
				for( int j=begin+1 ; j<end ; j++ )
					id += line[j];
				
				begin = findCharacter(line,end+1,right,'('); // Finding bitrate
				if( begin == right ){
					flag = false;
					break;
				}
				
				bitrate = (line[begin+1]-'0')*10+(line[begin+2]-'0');
				
				for( int j=begin+3 ; line[j]!=')' ; j++ ) // Saving bitrate
					bitrate = bitrate*10;
				
				begin = findCharacter(line,begin+1,right,'('); // Finding type
				if( begin == right ){
					flag = false;
					break;
				}
				
				type = line[begin+1]; // Saving type
				
			}
			
			else if( isdigit(line[left+2]) ){ // HTTP Status and sent bytes
				
				http_status = line[left+2]-'0'; // Saving HTTP Status
				
				sent_bytes = 0;
				ten = 1;
				for( int j=right-2 ; line[j]!=' ' ; j--,ten*=10 ) // Saving sent bytes
					sent_bytes += (line[j]-'0')*ten;
				
			}
			else{
				
				user_agent = "";

				for( int j=left+1 ; j < right && line[j] != ' ' ; j++ ) // Finding User Agent
					user_agent += line[j];
			}
			
		}
		
		cs_flag = false;
		for( int j=length-1 ; j>right ; j-- ) // Finding cs hit
			if( line[j] == 'c' && line[j+1] == 's' && line[j+2] == '=' ){
				cs_flag = true;
				cs_hit = (line[j+3] == 'H');
				break;
			}
		
		if( !flag || !cs_flag )
			continue;
		
		valid_lines++;
		total_data += sent_bytes;
		cs_hits += cs_hit;
		if( type == 'a' )
			audios++;
		else
			videos++;
		http_statuses[http_status-1]++;
		
		/* // This part is for comparing users by amount of their data transfers. If used, type of the second parameter of map should be 'long long int'
		 * if( users.find(ip) == users.end() )
			users[ip] = sent_bytes;
		else
			users[ip] += sent_bytes;
		*/
		
		// This part is for comparing users by amount of their data requests
		if( users.find(ip) == users.end() )
			users[ip] = 1;
		else
			users[ip] += 1;
		
		if( contents.find(id) == contents.end() )
			contents[id] = 1;
		else
			contents[id] ++;
			
		if( bitrates.find(bitrate) == bitrates.end() )
			bitrates[bitrate] = 1;
		else
			bitrates[bitrate] ++;
		
		if( user_agents.find(user_agent) == user_agents.end() )
			user_agents[user_agent] = 1;
		else
			user_agents[user_agent] ++;
		
	}
	
	input.close();
}

void NginxParser::statistics(){
	
	int current_max,w=35;
	string ip,id;
	
	ofstream output(output_file);
	output << setw(w) << left << "Number of unique IPs: " << users.size() << "\n\n";
	output << setw(w) << "Number of unique contents: " << contents.size() << "\n\n";
	output << setw(w) << "Total data sent: " << (double)total_data/1e9 << "GB\n\n";
	output << setw(w) << "Number of video requests: " << videos << "\n\n";
	output << setw(w) << "Number of audio requests: " << audios << "\n\n";
	output << setw(w) << "Number of cs hits requests: " << cs_hits << "\n\n";
	
	current_max = 0;
	for( auto &user: users )
		if( user.second > current_max ){
			current_max = user.second;
			ip = user.first;
		}
	
	output << setw(w) << "The IP that watched contents most: " << ip << " (" << current_max << " times)\n\n";
	
	current_max = 0;
	for( auto &content: contents )
		if( content.second > current_max ){
			current_max = content.second;
			id = content.first;
		}
	
	output << setw(w) << "The content that watched most: " << id << " (" << current_max << " times)\n\n";
	
	output << "Browsers:\n";
	for( auto &browser: user_agents )
		output << browser.first << ": %" << ((double)browser.second/valid_lines)*100.0 << "\n";
	
	output << "\nHTTP Statuses:\n";
	for( int i=0 ; i<5 ; i++ )
		output << i+1 << "xx: %" << ((double)http_statuses[i]/valid_lines)*100.0 << "\n";
	
	output << "\nBitrates:\n";
	for( auto &bitrate: bitrates )
		output << bitrate.first << ": %" << ((double)bitrate.second/valid_lines)*100.0 << "\n";
	
	output.close();
}
