#include "mymacros.h"

int main() {
    WSADATA wsaData;
    SOCKADDR_IN channel;    //to hold IP Address
    SOCKET s, sa;
    int bytes, bytesRecv, b, l, on = 1, i;
    char recvbuffer[1000];
	char buffer[200];
	std::string str;

    /* Initialize Windows Sockets API */
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "WSAStartup error :" << WSAGetLastError() << std::endl;
		WSACleanup();
		return false;
    }

    /* Build address structure to bind to socket */
    memset(&channel, 0, sizeof(channel));
	channel.sin_family = AF_INET;
	channel.sin_addr.s_addr = htonl(INADDR_ANY);
	channel.sin_port = htons(SERVER_PORT);

    /* Create socket */
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s < 0) {
		std::cout << "Socket error :" << WSAGetLastError() << std::endl;
		WSACleanup();
		return false;
	}
	int error;
	error = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));
    if ( error < 0 ) {
        std::cout << "Set sockopt error :" << WSAGetLastError() << std::endl;
		WSACleanup();
		return false;
	}

    /* Bind to socket */
    b = bind(s, (struct sockaddr *) &channel, sizeof(channel));
	if (b < 0) {
		std::cout << "Bind error :" << WSAGetLastError() << std::endl;
		WSACleanup();
		return false;
	}

	while (1) {
		/* Listen for socket connections */
		l = listen(s, QUEUE_SIZE);                 // specify queue size
		if (l < 0) {
			std::cout << "Listen error :" << WSAGetLastError() << std::endl;
			WSACleanup();
			return false;
		}

		/* Accept connection */
		sa = accept(s, 0, 0);                  					// block for connection request
		if (sa < 0) {														// their address and address len are not required from accept()
			std::cout << "Accept error : " << WSAGetLastError() << std::endl;
			WSACleanup();
			return false;
		}
		else {
			std::cout << "Connection established" << std::endl;
		}
		
		/* Receive from client */
		int count = 0;
		recvbuffer[0] = '\0';
	//	while (1) {
			bytesRecv = recv(sa, recvbuffer, 200, 0);
			if ( bytesRecv == 0 || bytesRecv == WSAECONNRESET ) {
	//			break;
				std::cout << "Recv error : " << WSAGetLastError() << std::endl;
				WSACleanup();
				return false;
			}
//			count++;
//			strcat(recvbuffer, buffer);
			std::cout << "Bytes Received : " << bytesRecv << std::endl;
	//	}
		std::cout << "Received from client : "  << recvbuffer << std::endl;
		
		
		
		int k = strcmp(recvbuffer,REQUEST_LBE);				// check whether received string matches with REQUEST_LBE string
		
		/* CLIENT is requesting leaderboard entries */
		if (k==0) {														// received string matches REQUEST_LBE string		
			LeaderboardEntry lbe[MAX_LBE_DISPLAY];							
			count = retrieve_lbe (lbe);										// populate lbe[] from lboard.dat
			std::cout << count << " entries retrieved" << std::endl;
			if (count < 0) {
				std::cout << "Error : could not access file" << std::endl;
				return -1;
			}
			if (count == 0) {
				std::cout << "Empty file" << std::endl;
				return -1;
			}
			
			for (i = 0; i < count; i++)
				str += ldb_struct_to_string(lbe[i]);						// make single string containing all lbe's

			char p[str.length()+1];										
			int i;
			for (i = 0; i < str.length(); i++) { 
				p[i] = str[i]; 										// char p[] from str, because send only takes char*, not const char *
			} 
			p[i] = '\0';
			bytes = strlen(p)+1;
			if (sendall(sa, p, bytes) < 0) {							// send char* to client through socket sa, along with sizeof(buffer)
				std::cout <<  "Send error" << std::endl;
				WSACleanup();																			
				return false;
			}	
		}
		
		/* CLIENT is sending a Leaderboard entry */
		else {																	
			LeaderboardEntry lbe[1];
			str = recvbuffer;
			string_to_ldb_struct(lbe, str);								// convert received string into lbe struct
			AddLeaderboardEntry(lbe[0]);								// write those entries to file
			
			
			strcpy(buffer, ENTRY_ADDED);
			bytes = strlen(buffer)+1;
			if (sendall(sa, buffer, bytes) < 0) {						// send confirmation to client
				std::cout <<  "Send error" << std::endl;
				WSACleanup();																			
				return false;
			}	
		}
		std::cout <<  "Bytes Sent : " << bytes << std::endl;					// send() returns bytes sent as an int

		/* Close */
		closesocket( sa );
	}
	closesocket( s );
	WSACleanup();
	return 0;
}

int sendall(int s, char *buf, int &len)						/* SEND BUFFER THROUGH SOCKET IN PACKETS IF NEED BE */
{
    int total = 0;        // bytes sent so far
    int bytesleft = len; // bytes left to send
    int n, i=0;
	
    while(total < len) {							// Loop per packet of data
        n = send(s, buf+total, bytesleft, 0);			// send data from where it last left off from
		if (n == -1) { break; }							// error in sending
        total += n;
        bytesleft -= n;
		i++;
    }

    len = total; 						// Bytes actually sent

    return n==-1?-1:0; 					// return -1 on failure, 0 on success
}

int countinfile(char* folder, char *name, char* extn, int size) {				/* RETURNS NUMBER OF STRUCTS IN BINARY FILE */
	char buffer[300];
	std::ifstream fin(pathof(folder, name, extn, buffer), std::ifstream::in | std::ifstream::binary);		//File input stream object initialization
	if (!fin) return BADFILE;																				//File not found
	fin.seekg(0, std::ifstream::end);																					//seek to end and return (file size in bytes/ sizeof struct)
	int n = fin.tellg()/size;
	fin.close();
	return n;
}

char* pathof(char* folder, char* name, char* extn, char* result) {				/* RETURNS FULL PATH OF FILE/FOLDER AS CHAR* */
	
	strcpy(result, PATH);														//PATH is defined in header file
	if(folder) {																//Concatenate if non NULL
		strcat(result,folder);
		if(name) strcat(result, "\\");
	}
	if(name) strcat(result, name);
	if(extn) strcat(result, extn);
	return result;
}

int retrieve_lbe (LeaderboardEntry *lbe) {										/* EXTRACT REQUIRED NUMBER OF ENTRIES FROM LBOARD.dat */
	int count = 0;
	char buffer[300],buffer1[50],buffer2[10],buffer3[5];
	strcpy(buffer1, DATA);
	strcpy(buffer2, LBOARD);
	strcpy(buffer3, DAT);
	int n = countinfile(buffer1, buffer2, buffer3, sizeof(LeaderboardEntry));						//count number of entries
	if (n == 0) return count;
	std::ifstream fin(pathof(buffer1, buffer2, buffer3, buffer), std::ifstream::in | std::ifstream::binary);		// open file object to read
	if (!fin) return -1;
	for (int i = 0; i < MAX_LBE_DISPLAY; i++) {
		if (n <= i) break;
		
		fin.seekg((n-1-i)*sizeof(LeaderboardEntry));								// seek to required number of recent entries and populate lbe with them
		fin.read((char *) &lbe[i], sizeof(LeaderboardEntry));
		count++;
/*		std::cout << "Username : " << lbe[i].uname << std::endl;
		std::cout << "Date and Time : " << lbe[i].date << std::endl;
		std::cout << "Duration : " << lbe[i].duration << "ms" << std::endl;
		std::cout << "Score : " << lbe[i].score << std::endl;
		std::cout << "Questions answered : " << lbe[i].qresult.qtotal << std::endl;
		std::cout << "Questions answered right : " << lbe[i].qresult.qright << std::endl;
		std::cout << "Questions answered wrong : " << lbe[i].qresult.qwrong << std::endl;
		std::cout << "Accuracy : " << lbe[i].accuracy << "%" << std::endl;
		std::cout << "Speed : " << lbe[i].speed << " Q/s" << std::endl;
		std::cout <<  "-----------------------------------" << std::endl;
*/	}		
	fin.close();
	return count;
}

std::string ldb_struct_to_string (LeaderboardEntry ldb) {					/* SERIALIZE STRUCT INTO A STRING WITH EACH COMPONENT SEPARATED BY '|' */
	std::stringstream ss;
	std::string str, s1(ldb.uname), s2(ldb.date), s3;
	ss << DELIM << ldb.score << DELIM												// Read each component of struct into string stream
        << ldb.accuracy << DELIM << ldb.speed << DELIM << ldb.duration << DELIM					// separated by delimiter '|'
            << ldb.qresult.qtotal << DELIM << ldb.qresult.qright << DELIM
                << ldb.qresult.qwrong << DELIM;
	str =  DELIM + s1 + DELIM + s2;
	ss >> s3;														// write stream to string
	str += s3;
	return str;														// return string
}

int string_to_ldb_struct (LeaderboardEntry* lbe1, std::string s) {					/* PARSE BIG STRING INTO ARRAY OF LBE STRUCTS */
	int index, count = 0;																					// of the form |uname1|date1|.....||uname2|date2|...|
	char p[100];
	std::vector<std::string> tokens;												// to store each token
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, '|'))									// parse istringstream object by DELIM '|' into a string vector
	{
		tokens.push_back(token);
	}
	for(int i = 0, j = 0; i < tokens.size() && i < 10 * MAX_LBE_DISPLAY; i++) {	 		// 
		index = i/10;																		// index is the k-th lbe in range 0 to MAX_LBE_DISPLAY
		if (index > count) count++;
		switch(i%10) {
			case 0: break;
			case 1: 
					for (j = 0; j < tokens[i].length(); j++)					//	populate k-th lbe.uname
						p[j] = tokens[i][j]; 
					p[j] = '\0';
					strcpy(lbe1[index].uname,p); break;
			case 2: 
					for (j = 0; j < tokens[i].length(); j++)					// k-th lbe.date
						p[j] = tokens[i][j]; 
					p[j] = '\0';
					strcpy(lbe1[index].date,p); break;
			case 3: lbe1[index].score = std::stoi(tokens[i]); break;			//score
			case 4: lbe1[index].accuracy = std::stof(tokens[i]); break;				//accuracy
			case 5: lbe1[index].speed = std::stof(tokens[i]); break;				//speed
			case 6: lbe1[index].duration = std::stol(tokens[i]); break;					//duration
			case 7: lbe1[index].qresult.qtotal = std::stoi(tokens[i]); break;			//qtotal
			case 8: lbe1[index].qresult.qright = std::stoi(tokens[i]); break;			//qright
			case 9: lbe1[index].qresult.qwrong = std::stoi(tokens[i]); break;			//qwrong
		}
	}
	return count+1;
}

int AddLeaderboardEntry(LeaderboardEntry lbe) {									/* WRITE LEADERBOARD ENTRY TO LBOARD.dat */
	char buffer[300],buffer1[50],buffer2[10],buffer3[5];
	strcpy(buffer1, DATA);
	strcpy(buffer2, LBOARD);
	strcpy(buffer3, DAT);
	std::ofstream fout(pathof(buffer1, buffer2, buffer3, buffer), std::ofstream::app | std::ofstream::binary);
	fout.write((char *)&lbe, sizeof(LeaderboardEntry));
	fout.close();
	return countinfile(buffer1, buffer2, buffer3, sizeof(LeaderboardEntry));
}