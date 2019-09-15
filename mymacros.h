#include <iostream>
#include <cstdlib>
#include <fstream>
#include <chrono>
#include <unistd.h>
#include <string>
#include <vector>
#include <sstream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <dir.h>
#include <dirent.h>
#include <conio.h>
#include <ctype.h>

#define PATH "C:\\MinGW\\bin\\Networking\\"			// PATH
#define DUMP "DUMP"									// Dump folder
#define QBANK "QBANK"								// folder where questions are stored in .txt files
#define LBOARD "LBOARD"								// Leaderboard entries are stored in LBOARD.dat
#define	DATA "DATA"
#define DAT ".DAT"
#define TXT ".TXT"
#define SERVER_PORT 12345
#define QUEUE_SIZE 5
#define LOOPBACK_IP "127.0.0.1"
#define DELIM "|"
#define SUCCESS 0
#define ESC 27
#define ABORTED -1
#define BADFILE -6
#define REQUEST_LBE "pls"							// Magic word
#define ENTRY_ADDED "Leaderboard has been updated"
#define MAX_LBE_DISPLAY 3							// Max entries displayed by view leaderboards

typedef struct {
	int qtotal,qright,qwrong;
} QResult;

typedef struct {
	char uname[50], date[30];
	int  score;
	float accuracy, speed;
	long duration;
	QResult qresult;
} LeaderboardEntry;

typedef struct {
	char uname[50];
//	char password[20];
} Player;

typedef struct {
	char question[1000];
	char option[4][1000];
	int answer;
} Question;

int ReadTextFiles();
Player login();
void Newgame(Player);
int GameUI (LeaderboardEntry&, Player);
Question fetchquestion(char*, int);
void displayquestion(int, Question); 
int GetAnswer();
int SendLeaderboardEntry (LeaderboardEntry);
std::string ldb_struct_to_string(LeaderboardEntry);
int sendall(int, char*, int&);
void PostGame (LeaderboardEntry);
int view_leaderboards();
int fetch_leaderboards(LeaderboardEntry*);
int string_to_ldb_struct(LeaderboardEntry*, std::string);

int retrieve_lbe(LeaderboardEntry*);
int AddLeaderboardEntry(LeaderboardEntry);

void swap(int*, int, int); 
void FisherYates(int*, int);
void DeleteFolder(char*); 
void DeletWork_dat();
char* pathof(char*, char*, char*, char*);
int countinfile(char*, char*, char*, int);


