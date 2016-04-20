#include "stdafx.h"
using namespace std;

//from parsing
int read_positive_integer();
//from ConductProtocol
int ready(SOCKET);

//user tells us ordered number of message they want to delete (indexed from 0)

int deleteMessage(SOCKET ConnectSocket){
	string command = "06:";
	cout<<"Enter the number beside the message you wish to delete"<<endl;
	int messageToDelete=read_positive_integer();
	command += to_string(messageToDelete);
	const char* cmd = command.c_str();
	//cout<<"delete command: "<<cmd<<endl;
	int iSendResult = send(ConnectSocket, cmd, (int)strlen(cmd), 0 ); 
	
	char mybuf[DEFAULT_BUFLEN];
	int iResult = recv( ConnectSocket, mybuf, (int)strlen(mybuf), 0 );//recieve Y or N from deleteMessage 
	if(mybuf[0]=='Y'){
		cout<<"Deleted message"<<endl;
	}else{
		cout<<"Message not found"<<endl;
	}

	ready(ConnectSocket);//ready to exit [D1]
	iResult = recv( ConnectSocket, mybuf, (int)strlen(mybuf), 0 );//[CP] from conductprotocol

	return 0;
}