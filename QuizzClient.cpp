#include "mymacros.h"

int main () {
	Player player;
	char keystroke = 0;
	DeletWork_dat();							// Delete any instance of Work.dat which may have not been deleted yet
	if(!ReadTextFiles()) 										// Create Work.dat in DUMP
		std::cout << "No files detected" << std::endl;
	while (1) {
		std::cout << "\n";
		std::cout << "1. Login" << std::endl;
		std::cout << "2. View leaderboards" << std::endl;
		keystroke = _getch();										// assigns value of keystroke
		if (keystroke == ESC) break;								// break if ESC is pressed
		switch (keystroke) {
			case '1':								// 1 to login
				player = login();
				Newgame(player);						// start game
				break;
			case '2':								// 2 to view leaderboards
				view_leaderboards();
				break;
		}
	}
	DeletWork_dat();					// Delete Work.dat
	return 0;
}


 
int ReadTextFiles() {								/* READ AND CONVERT QUESTIONS FROM ALL TEXT FILES IN QBANK INTO A SINGLE BINARY FILE Work.dat */
	char answer[20];
	char buffer[300], buffer1[300], buffer2[10],buffer3[10];
	WIN32_FIND_DATAA ffd;										// Findfirst block
	int count = 0, i=0, done = 0;
	
	strcpy(buffer1, QBANK);
	pathof(buffer1,NULL,NULL,buffer);								// assign PATH + QBANK to buffer
	strcat(buffer, "\\*.txt");									// buffer now is PATH\QBANK\*.txt
	HANDLE hfile = FindFirstFileA(buffer, &ffd);				// Find first instance of above										
	while(!done) {
		count++;
		strcpy(buffer1, QBANK);
		pathof(buffer1,NULL,NULL,buffer);
		strcat(buffer, "\\");
		strcat(buffer, ffd.cFileName);							// buffer has PATH\QBANK\something.TXT
		std::ifstream fin(buffer, std::ifstream::in);			// ifstream object pointing to above file 
		if (!fin) return -1;
		strcpy(buffer1, DUMP);
		strcpy(buffer2, "Work");
		strcpy(buffer3, DAT);
		std::ofstream fout(pathof(buffer1, buffer2, buffer3, buffer), std::ofstream::app | std::ofstream::binary);			//ofstream object appending Work.dat file
		fin.seekg(0);
		Question q;
		while (!fin.eof()) {
			fin.getline(q.question, 300, '\n');					// read question into struct
			for (int j = 0;j < 4; j++) {
				fin.getline(q.option[j], 300, '\n');			// read each option into struct
			}
			fin.getline(answer, 20, '\n');						// read answer
			q.answer = std::atoi(answer);						// store answer as int to struct (either 1, 2, 3 or 4)
			fout.write((char *)&q, sizeof(Question));			// write to Work.dat
		}
		fin.close();
		fout.close();
		done = FindNextFileA(hfile,&ffd);						// Find next instance in directory, returns -1 if no more files detected, 0 if found								
	}
	return count;
}

/*
void DeleteFolder(char* folder) {						/* DELETE A FOLDER AND ALL ITS CONTENTS 
	char buffer[300];
	struct ffblk ffblk;																// Findfirst block
	int done = findfirst(pathof(folder, "*", ".*", buffer), &ffblk, 0);					// Find first instance of PATH\folder\*.*
	while(!done) {
		unlink(pathof(folder, ffblk.ff_name, NULL, buffer));							// Delete file
		done = findnext(&ffblk);														// find next file
	}
	rmdir(pathof(folder, NULL, NULL, buffer));											// Delete the now empty folder
}
*/

void DeletWork_dat() {											/* DELETE Work.dat */
	char buffer[50],buffer1[50],buffer2[10],buffer3[50];
	strcpy(buffer,DUMP);
	strcpy(buffer1,"Work");
	strcpy(buffer2,DAT);
	unlink(pathof(buffer,buffer1,buffer2,buffer3));				// delete the file
	return;
}

void FisherYates(int* a, int n) {				/* CREATE AN ARRAY OF NUMBERS 1 TO n IN RANDOM ORDER */
	srand(time(0));									// seed
	int i, k; 
	for(i = 0; i < n; i++) a[i] = i;				// assign array values from 0 to n-1
	for(k = 0; k < n; k++) {
		swap(a, k, k + (rand() % (n-k)));			// swap k-th value with random position in n-k positions
	}
}

void swap(int* a, int i, int j) {			/* SWAP TWO ELEMENTS OF INTEGER ARRAY */
	int temp = a[i];
	a[i] = a[j];
	a[j] = temp;
}

Player login () {							/* BASIC LOGIN FUNCTION */
	Player player;
	std::cout << "Enter username : ";
	std::cin >> player.uname;						// get username and return player struct
	if (strlen(player.uname) < 2) return login();
	return player;
}

void Newgame (Player player) {				/* CREATES NEW INSTANCE OF GAME; Each game starts and ends within this function */
	LeaderboardEntry lbe;
	if (GameUI(lbe, player) == ABORTED) return; 									//Returns -1 if ESC is pressed during game
	
	if (SendLeaderboardEntry(lbe) < 0)										//Sends leader board entry to server
		std::cout << "Failed to send leaderboard entry" << std::endl;
	
	PostGame(lbe);							//Post game stats
}

Question fetchquestion(char* fname, int n) {		/* RETURNS n-th QUESTION FROM SPECIFIED FILE */
	Question q;
	memset((void*)&q, 0, sizeof(Question));								// Initialize struct to 0
	std::ifstream fin(fname, std::ifstream::in | std::ifstream::binary);							// File input stream object initialization
	if (!fin) return q;													// Returns empty q if file not found
	fin.seekg((long) n * sizeof(Question));								// seeks to n-th question and reads
	fin.read((char *)&q, sizeof(Question));
	fin.close();
	return q;
}

void displayquestion(int index, Question q) {							/* DISPLAY QUESTION, FUNNY YOU SHOULD ASK */
	std::cout << index + 1 << ". " << q.question << std::endl << std::endl;
	for (int i = 0; i < 4; i++) 
		std::cout << i+1 << ") " << q.option[i] << std::endl;
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

int GameUI (LeaderboardEntry &lbe, Player player) {						/* GAME USER INTERFACE */
	int array[300], score, j, i, n, answer = 1, qlimit;
	QResult qresult = {0,0,0};
	char buffer[300],buffer1[50],buffer2[10],buffer3[5];
	long duration = 0, flagtime = -1;
	Question q;
	strcpy(buffer1, DUMP);
	strcpy(buffer2, "Work");
	strcpy(buffer3, DAT);
	n = countinfile(buffer1, buffer2, buffer3, sizeof(Question));										// Count number of questions in Work.dat
	FisherYates(array, n = countinfile(buffer1, buffer2, buffer3, sizeof(Question)));					// (Fisher Yates algorithm) Create array of random numbers upto n
	std::cout << n << " question(s) detected" << std::endl;
	std::cout << "Choose number of questions : ";
	std::cin >> qlimit;
	while (qlimit <= 0 || qlimit > n) {														// select a Qlimit <= total number of detected questions
			std::cout << "Invalid response\n";
			std::cin >> qlimit;
	}
	time_t tt; 																				
    struct tm *ti; 													  						
    time (&tt); 																			// start date and time
    ti = localtime(&tt); 
	auto start = std::chrono::steady_clock::now();											// start time
	std::cout << std::endl;
	
	for(score = 0; qresult.qtotal < qlimit;) {											/* Questions LOOP */
		strcpy(buffer1, DUMP);
		strcpy(buffer2, "Work");
		strcpy(buffer3, DAT);
		q = fetchquestion(pathof(buffer1, buffer2, buffer3, buffer), array[qresult.qtotal]);			// fetch question from Work.dat indexed as i-th number of random numbers array
		displayquestion(qresult.qtotal, q);																// and display
		
		answer = GetAnswer();																// get answer
		std::cout << answer << std::endl;
		std::cout << "--------------------------------------------------" << std::endl;
		qresult.qtotal++;
		
		if(answer < 0) {																	// if ESC was pressed in GetAnswer()
			return ABORTED;																	// return -1
		}
		
		if(answer == q.answer) {										// Right answer
			qresult.qright++;
			score += 4;
		}
		else {															// Wrong answer
			qresult.qwrong++;
			score -= 1;
		}
	}
	
	auto end = std::chrono::steady_clock::now();						// end time
	
	strcpy(lbe.uname, player.uname);									// Player name
	strcpy(lbe.date, asctime(ti));										// Date and time
	lbe.qresult = qresult;
	lbe.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();			//Duration
	lbe.score = score;																// Score
	lbe.accuracy = qresult.qtotal ? (100.0 * qresult.qright)/qresult.qtotal : 0;			// Accuracy
	lbe.speed = (((float)qresult.qtotal)/(duration ? (duration / 1000) : 1));			// Speed 

	std::cout << std::endl;
	return SUCCESS;
}

int GetAnswer() {					/* READS KEYSTROKE AND RETURNS CHOSEN ANSWER */
	int keystroke;
	while(1) {
		keystroke = _getch();				// Read keystroke
		switch(keystroke) {			
			case ESC: return -1;				// Return only if key is ESC, 1, 2, 3 or 4
			case '1': return 1;
			case '2': return 2;
			case '3': return 3;
			case '4': return 4;					
		}
	}
}

std::string ldb_struct_to_string (LeaderboardEntry ldb) {					/* SERIALIZE STRUCT INTO A STRING WITH EACH COMPONENT SEPARATED BY '|' */
	std::stringstream ss;
	std::string str, s1(ldb.uname), s2(ldb.date), s3;
	ss << DELIM << ldb.score << DELIM															// Read each component of struct into string stream
        << ldb.accuracy << DELIM << ldb.speed << DELIM << ldb.duration << DELIM					// separated by delimiter '|'
            << ldb.qresult.qtotal << DELIM << ldb.qresult.qright << DELIM
                << ldb.qresult.qwrong << DELIM;
	str =  DELIM + s1 + DELIM + s2;
	ss >> s3;																		// write stream to string
	str += s3;
	std::cout << str << std::endl;
	return str;																		// return the string
}

int SendLeaderboardEntry (LeaderboardEntry lbe) {				/* SEND LEADERBOARD ENTRY TO SERVER */
	std::string lbe_str = ldb_struct_to_string (lbe);					// obtain string equivalent of struct lbe

	WSADATA wsaData;
    SOCKADDR_IN target;
    SOCKET s;
    int bytes;

	/* Initialize Windows Sockets API */
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "WSAStartup error : " << WSAGetLastError() << std::endl;
		WSACleanup();
		return -1;
    }

	/* Build address structure to bind to socket */
    target.sin_family = AF_INET;                        // address family Internet (IPv4 / IPv6), in this case IPv4
	target.sin_port = htons (SERVER_PORT);                 // port to connect to
	target.sin_addr.s_addr = inet_addr (LOOPBACK_IP);            // target IP is a loop back due to availability of only one system

    /* Create socket */
    s = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);     // creating socket with parameters IPv4, Socket stream, TCP
	if (s == INVALID_SOCKET) {
		std::cout << "Socket error : " << WSAGetLastError() << std::endl;
		WSACleanup();
		return -1;
	}

	/* Connect to server socket */
	if (connect(s, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR) {				// establishing connection between
		std::cout << "Connect error : " << WSAGetLastError() << std::endl;						// created socket s (client) and
		WSACleanup();																			// target socket target (server)
		return -1;
	}
		
	/* Send to server */
	char p[lbe_str.length()+1];										// char p[] from str, because send only takes char*, not const char *
	int i;
	for (i = 0; i < lbe_str.length(); i++) { 
		p[i] = lbe_str[i]; 
	} 
	p[i] = '\0';
	bytes = strlen(p)+1;

	if (sendall(s, p, bytes) < 0) {									// send p through socket s
		std::cout <<  "Send error" << std::endl;
		WSACleanup();																			
		return -1;
	}									
	std::cout <<  "Bytes Sent : " << bytes << std::endl << std::endl;;					// send() returns bytes sent as an int
	
	/* Receive from server */
	char recvbuffer[3000];
	int bytesRecv;
	bytesRecv = recv(s, recvbuffer, 1000, 0);							// receive response from server
	if ( bytesRecv == 0 || bytesRecv == WSAECONNRESET ) {
		std::cout <<  "Connection Closed." << std::endl;
		WSACleanup();
    }
	std::cout << recvbuffer << std::endl;
	std::cout << "Bytes Received : " << bytesRecv << std::endl;
	

	/* Close */
	closesocket(s);									// end operations on socket s
	WSACleanup();	
	return 0;
}


int sendall(int s, char *buf, int &len)	{					/* SEND BUFFER THROUGH SOCKET IN PACKETS IF NEED BE */
    int total = 0;        // bytes sent so far
    int bytesleft = len; // bytes left to send
    int n;

    while(total < len) {									// Loop per packet of data
        n = send(s, buf+total, bytesleft, 0);					// send data from where it last left off from
        if (n == -1) { break; }									// error in sending
        total += n;
        bytesleft -= n;
    }

    len = total; 						// Bytes actually sent

    return n==-1?-1:0; 					// return -1 on failure, 0 on success
}

void PostGame (LeaderboardEntry lbe) {									/* DISPLAY POST GAME STATS */
	std::cout << "\nPost game stats\n" << std::endl;
	std::cout << "Username : " << lbe.uname << std::endl;
	std::cout << "Date and Time : " << lbe.date << std::endl;
	std::cout << "Duration : " << lbe.duration << std::endl;
	std::cout << "Score : " << lbe.score << std::endl;
	std::cout << "Questions answered : " << lbe.qresult.qtotal << std::endl;
	std::cout << "Questions answered right : " << lbe.qresult.qright << std::endl;
	std::cout << "Questions answered wrong : " << lbe.qresult.qwrong << std::endl;
	std::cout << "Accuracy : " << lbe.accuracy << std::endl;
	std::cout << "Speed : " << lbe.speed << std::endl;
}

int fetch_leaderboards(LeaderboardEntry* lbe) {
	
	WSADATA wsaData;
    SOCKADDR_IN target;
    SOCKET s;
    int bytes, count = 0;
	char *buffer;

	/* Initialize Windows Sockets API */
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "WSAStartup error : " << WSAGetLastError() << std::endl;
		WSACleanup();
		return -1;
    }

	/* Build address structure to bind to socket */
    target.sin_family = AF_INET;                        // address family Internet (IPv4 / IPv6), in this case IPv4
	target.sin_port = htons (SERVER_PORT);                 // port to connect to
	target.sin_addr.s_addr = inet_addr (LOOPBACK_IP);            // target IP is a loop back due to availability of only one system

    /* Create socket */
    s = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);     // creating socket with parameters IPv4, Socket stream, TCP
	if (s == INVALID_SOCKET) {
		std::cout << "Socket error : " << WSAGetLastError() << std::endl;
		WSACleanup();
		return -1;
	}

	/* Connect to server socket */
	if (connect(s, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR) {				// establishing connection between
		std::cout << "Connect error : " << WSAGetLastError() << std::endl;						// created socket s (client) and
		WSACleanup();																			// target socket target (server)
		return -1;
	}
		
	/* Send to server */
	std::string str(REQUEST_LBE);							// send request message to server for leaderboard entries
	char p[str.length()+1];
	int i;
	for (i = 0; i < str.length(); i++) { 
		p[i] = str[i]; 
	} 
	p[i] = '\0';
	bytes = strlen(p)+1;
	if (sendall(s, p, bytes) < 0) {							// send buffer to socket s, along with sizeof(buffer)
		std::cout <<  "Send error" << std::endl;
		WSACleanup();																			
		return -1;
	}									
	std::cout <<  "Bytes Sent : " << bytes << std::endl;					// send() returns bytes sent as an int

	
	
	/* Receive from server */
	char recvbuffer[3000], buffer1[1000];
	int bytesRecv;
	recvbuffer[0] = '\0';
	while (1) {
		bytesRecv = recv(s, buffer1, 1000, 0);						// receive leaderboard entries in recvbuffer[]
		if ( bytesRecv == 0 || bytesRecv == WSAECONNRESET ) {
			break;
		}
		count++;
		strcat(recvbuffer, buffer1);
		std::cout << "Bytes Received : " << bytesRecv << std::endl;
	}
//	std::cout << recvbuffer << std::endl;
	
	
	/* Close */
	closesocket(s);									// end operations on socket s
	WSACleanup();
	
	str = recvbuffer;							// convert recvbuffer to string
	count = string_to_ldb_struct (lbe, str);				// convert string to array of LeaderboardEntry struct and obtain count of array
	
	return count;
}

int string_to_ldb_struct (LeaderboardEntry* lbe1, std::string s) {					/* PARSE BIG STRING INTO ARRAY OF LBE STRUCTS */
	int index, count = 0;																					// of the form |uname1|date1|.....||uname2|date2|...|
	char p[100];
	std::vector<std::string> tokens;												// to store each token
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, '|')) {									// parse istringstream object by DELIM '|' into a string vector
		tokens.push_back(token);
	}
	for(int i = 0, j = 0; i < tokens.size() && i < 10 * MAX_LBE_DISPLAY; i++) {	 		//iterate through tokens vector
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

int view_leaderboards () {								/* DISPLAY REQUIED NUMBER OF LEADERBOARD ENTRIES FROM SERVER */
	LeaderboardEntry lbe[MAX_LBE_DISPLAY];
	int count = fetch_leaderboards(lbe);								// populate lbe with data obtained from server
	if (count < 0) 														
		std::cout << "Could not access server" << std::endl;
	else if (count == 0)														// no entries
		std::cout << "No entries found" << std::endl;
	else {
		std::cout << "-----------------------" << std::endl;
		for (int i = 0; i < count; i++) {											// display entries
			std::cout << "Username : " << lbe[i].uname << std::endl;
			std::cout << "Date and Time : " << lbe[i].date << std::endl;
			std::cout << "Duration : " << lbe[i].duration << "ms" << std::endl;
			std::cout << "Score : " << lbe[i].score << std::endl;
			std::cout << "Questions answered : " << lbe[i].qresult.qtotal << std::endl;
			std::cout << "Questions answered right : " << lbe[i].qresult.qright << std::endl;
			std::cout << "Questions answered wrong : " << lbe[i].qresult.qwrong << std::endl;
			std::cout << "Accuracy : " << lbe[i].accuracy << "%" << std::endl;
			std::cout << "Speed : " << lbe[i].speed << " Q/s" << std::endl;
			std::cout <<  "-----------------------------------" << std::endl;
		}
	}
	return 0;
}