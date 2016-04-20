#include "stdafx.h"
using namespace std;


//from ConcudtProtocol.cpp
int ready(SOCKET);
string	getAllPackages(vector<string>,SOCKET);

//from Parsing.cpp
vector<string> makeTolkens(char* input);


extern map<string,vector<Message>> g_messages;
extern vector<User> g_userlist;

int addMessage(vector<string> tolkens, SOCKET ClientSocket, int userID){

	string currentUser=g_userlist[userID].name;

	string message = getAllPackages(tolkens,ClientSocket);
	//check if they're done yet
	char recvbuf[DEFAULT_BUFLEN];
	int iRecieve = recv(ClientSocket, recvbuf, DEFAULT_BUFLEN, 0);//ready to exit? [M1]

	if(message.size()<1){
		cout<<"no message entered"<<endl;
		return 0;
	}
	//put it in the database	
	Message newMessage = Message(message, currentUser);
	g_messages[currentUser]; 
	auto it = g_messages.find(currentUser);
	it->second.push_back(newMessage);

	//check the last message:
	int numberOfUserMsg = it-> second.size();
	Message lastMsg = it->second.at(numberOfUserMsg-1);
	cout<<"added a message: "<<lastMsg.content<< " with ID "<<lastMsg.id<<" at "<< lastMsg.timeStamp << endl;

	return 0;
}