#include "stdafx.h"
using namespace std;

//functions from Parsing.cpp
vector<string> packageString(string input);
//functions from ConductProtocol.cpp
int sendAllPackages(vector<string> messagePkgs, SOCKET ConnectSocket);

int sendMessage(SOCKET ConnectSocket, std::string message){
	bool notDoneYet = false;
	int iResponse=-1;

	//divide message into sets w length DEFAULT_BUFLEN
	vector<string> messagePkgs = packageString(message);
	iResponse = sendAllPackages(messagePkgs, ConnectSocket);
	
	return iResponse;
}

int newMessage(SOCKET ConnectSocket){
	
	cout<<"Enter a message: "<<endl<<"(Click enter to exit)"<<endl;
	string message;
	getline(cin,message);
	if(message.length()<1){
		return 0;
	}

	int iResult = sendMessage(ConnectSocket, message);

	//let them know we're done here
	char check[DEFAULT_BUFLEN];
	int iRecieve = send(ConnectSocket, "M1", 2, 0);//ready to exit [M1]
	iRecieve = recv(ConnectSocket, check, DEFAULT_BUFLEN, 0);//[CP] from ConductProtocol;
	return iRecieve;
}