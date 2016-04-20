#include "stdafx.h"
using namespace std;

extern map<string,vector<Message>> g_messages;
extern vector<User> g_userlist;

int deleteMessage(SOCKET ClientSocket, vector<string> tolkens, int userID){

	string currentUser=g_userlist[userID].name;

	//06:MessageToDelete
	int messageToDelete= stoi(tolkens.at(1));
	//find user messages in db
	auto userIt = g_messages.find(currentUser); //userIt->second is the user's message vector
	cout<<"user wants to delete message message "<<messageToDelete;
	cout<<" of "<<userIt->second.size()<<endl;

	if(messageToDelete < userIt->second.size()){
		userIt->second.erase(userIt->second.begin()+messageToDelete);
		cout<<"Deleted user message at "<<messageToDelete<<endl;
		int iSendResult = send(ClientSocket, "Y", 1, 0 ); 
	}else{
		cout<<"No message found"<<endl;
		//we didnt delete it because the index was wrong
		int iSendResult = send(ClientSocket, "N", 1, 0 ); 
	}
	
	char recvbuf[DEFAULT_BUFLEN];
	int iRecieve = recv(ClientSocket, recvbuf, DEFAULT_BUFLEN, 0);//ready to exit? [D1]
	return 0;
}