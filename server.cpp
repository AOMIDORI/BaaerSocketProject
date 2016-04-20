#undef UNICODE

#define WIN32_LEAN_AND_MEAN


#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include "Database.h"

#include "stdafx.h"

using namespace std;

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

//.......Global variables.................
int g_usercount=0;
int g_messagecount=0;
mutex g_mutex;
vector<string> g_namelist;
vector<User> g_userlist;
//vector<Message> g_messagelist;

//global variables
map<string,vector<Message>> g_messages;
string currentUser =  "user";
int allMessageIds = 0;


//.......functions predeclaration..........
void connection_handler(SOCKET);
void message_handler(int, int,SOCKET);
void print_timeline(int,SOCKET);
int follow_handler(int,SOCKET);
int unfollow_handler(int,SOCKET);
//from ConductProtocol.cpp
int conduct_protocol(SOCKET ClientSocket);
int ready(SOCKET ClientSocket);

/*************************************************************
                             Main
*************************************************************/
int __cdecl main(void) 
{
    WSADATA wsaData;
    int iResult;

    struct addrinfo *result = NULL;
    struct addrinfo hints;
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	vector<thread*> threads;
	
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = ::bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
	cout<<"=========================================================="<<endl;
	cout<<"                   The Server starts"<<endl;
	cout<<"=========================================================="<<endl;
    
    // Accept a client socket
	struct sockaddr_in clientAddr;
    while(ClientSocket = accept(ListenSocket, (sockaddr *)&clientAddr, NULL)){
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}
		threads.push_back(new thread(connection_handler,ClientSocket));
		g_usercount++;
	}

	for(int i=0;i<g_usercount;i++){
		threads[i]->join();
	}

	closesocket(ListenSocket);
    WSACleanup();
    return 0;
}

void connection_handler(SOCKET ClntSock){
	//this_thread::sleep_for(chrono::milliseconds(10));
	SOCKET ClientSocket=ClntSock;
	char *sendbuf=NULL;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
	bool iflogin=FALSE;
	WSADATA wsaData;
	int iResult;
	int iSendResult;
	iResult=WSAStartup(MAKEWORD(2,2),&wsaData);
	int userID; 
	//Receive the username from the client------------------------------------------
	iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	if (iResult > 0) {
		recvbuf[iResult]='\0';
		string username(recvbuf);
		cout<<"Name received: ["<<username<<"], checking..."<<endl;
		if(find(g_namelist.begin(),g_namelist.end(),username)!=g_namelist.end()){
			//cout<<"The user exists."<<endl;
		}
		else{
			g_namelist.push_back(username);
			cout<<"New user ["<<username<<"] is registered."<<endl;
			User u;
			u.name=(char *)malloc(username.length()+1);
			int i;
			for(i=0;username[i]!='\0';i++){
				u.name[i]=username[i];
			}
			u.name[i]='\0';
			u.status=0;
			g_userlist.push_back(u);
		}
		//check if the user is taken
		bool usertaken=FALSE;
		for(userID=0;userID<g_userlist.size();userID++){
			if(username.compare(g_userlist[userID].name)==0){		
				if(g_userlist[userID].status==1){
					usertaken=TRUE;
					cout<<"Log in declined."<<endl;
					break;
				}	
				break;
			}
		}
		//send the login result to the user.
		if(!usertaken){
			g_userlist[userID].status=1;
			sendbuf="00"; //The user can log in
			iflogin=TRUE;
			if(send(ClientSocket, sendbuf, (int)strlen(sendbuf)+1,0)==SOCKET_ERROR){
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				exit(1);
			}
			cout<<"["<<g_userlist[userID].name<<"] log in"<<endl;

		}
		else{
			sendbuf="01"; //The user can't log in
			if(send(ClientSocket, sendbuf, (int)strlen(sendbuf)+1,0)==SOCKET_ERROR){
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				exit(1);
			}
			///break;
		}
	}
	else if (iResult == 0)
		printf("Connection closing...\n");


	//Receive menu choice from client ------------------------------------------
	while(iflogin){
		iResult=recv(ClientSocket,recvbuf,recvbuflen,0);
		if(iResult>0){
			cout<<"Choice from ["<<g_userlist[userID].name<<"]: "<<recvbuf[0]<<endl;
			switch (recvbuf[0]){
			case '7':
				g_userlist[userID].status=0;
				cout<<"["<<g_userlist[userID].name<<"] log out."<<endl;
				iflogin=FALSE;
				break;
			case '1':
				cout<<"["<<g_userlist[userID].name<<"] send a message."<<endl;
				message_handler(0,userID,ClientSocket);
				break;
			case '3':
				cout<<"Printing ["<<g_userlist[userID].name<<"]'s timeline:"<<endl;
				print_timeline(userID,ClientSocket);
				break;
			case '5':
				follow_handler(userID,ClientSocket);
				break;
			case '6':
				unfollow_handler(userID,ClientSocket);
				break;
			default:
				cout<<"Invalid choice."<<endl;
				break;
			}
		}
	}
	g_userlist[userID].status=0;

	//shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		exit(1);
	}
	closesocket(ClientSocket);
}

/////////////////////////////////////////////////////////////////////////////////
/**********************************************************************
                        message_handler()
***********************************************************************/
void message_handler(int option, int userID,SOCKET clntSocket){
	SOCKET ClientSocket=clntSocket;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	int iResult;
	if(option==0){ //send a message
		g_mutex.lock();
		iResult=recv(ClientSocket,recvbuf,recvbuflen,0);
		g_mutex.unlock();
		if(iResult>0){
			recvbuf[iResult]='\0';
			g_messagecount++;  //to generate the ID
			struct Message m;
			m.messageID=g_messagecount;
			m.text=(char*)malloc(iResult);
			strcpy(m.text,recvbuf);
			m.publishtime=NULL;
			//time_t rawtime;
			//m.publishtime=localtime(&rawtime);
			//printf("%s\n",asctime(m.publishtime));
			g_userlist[userID].messagelist.push_back(m);
		}
	}
	if(option==1){ //Delete a message

	}
}
/*********************************************************************
                         print_timeline()
*********************************************************************/
void print_timeline(int userID,SOCKET clntSocket){
	int i;
	for(i=0;i<(int)g_userlist[userID].messagelist.size();i++){
		cout<<g_userlist[userID].messagelist[i].text<<endl;
	}
}

/***********************************************************************
                         follow_handler()
***********************************************************************/
//50: Not found
//51: Follow him/herself
//52: Already followed
int follow_handler(int userID, SOCKET clntSocket){
	SOCKET ClientSocket=clntSocket;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	char *sendbuf="";
	unsigned int i;
	int friendID=-1;
	int iResult;
	int iSendResult;
	bool havefollowed=FALSE;
	g_mutex.lock();
	iResult=recv(ClientSocket,recvbuf,recvbuflen,0);
	g_mutex.unlock();
	if(iResult>0){
		recvbuf[iResult]='\0';
		char *name=(char*)malloc(strlen(recvbuf)+1);
		strcpy(name,recvbuf);
		for(i=0;i<g_userlist[userID].following.size();i++){
			if(strcmp(name,g_userlist[userID].following[i].name)==0){
				havefollowed=TRUE;
				sendbuf="52";
				break;
			}
		}
		if(strcmp(name,g_userlist[userID].name)==0){
			sendbuf="51";
		}
		else{
			if(!havefollowed){
				for(i=0;i<g_userlist.size();i++){
					if(strcmp(g_userlist[i].name,name)==0){
						friendID=i;
						g_userlist[userID].following.push_back(g_userlist[i]);
						//aha, I get it. The User Object of the user to follow is pushed to following<User>
						cout<<"["<<g_userlist[userID].name<<"] follows "<<name<<"."<<endl;
						break;
					}
				}
				if(i==g_userlist.size()){
					sendbuf="50";
					cout<<"Can't find the user."<<endl;
				}
			}
		}
	}
	iSendResult=send(ClientSocket,sendbuf,(int)strlen(sendbuf)+1,0);
	if(iSendResult==SOCKET_ERROR){
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
	}
	return friendID;
}
/***********************************************************************
                         unfollow_handler()
***********************************************************************/
//60: User doesn't exist
//61: unfollow him/her self
//62: The user is not in the following list

int unfollow_handler(int userID, SOCKET clntSocket){
	SOCKET ClientSocket=clntSocket;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	char *sendbuf="";
	unsigned int i;
	int friendID=-1;
	int iResult;
	int iSendResult;
	bool havefollowed=FALSE;

	iResult=recv(ClientSocket,recvbuf,recvbuflen,0);
	if(iResult>0){
		recvbuf[iResult]='\0';
		char *name=(char*)malloc(strlen(recvbuf)+1);
		strcpy(name,recvbuf);
		for(i=0;i<g_userlist[userID].following.size();i++){
			if(strcmp(name,g_userlist[userID].following[i].name)==0){
				havefollowed=TRUE;
				break;
			}
		}
		if(!havefollowed) sendbuf="62";
		if(strcmp(name,g_userlist[userID].name)==0){
			sendbuf="61";
		}
		else{
			if(havefollowed){
				for(i=0;i<g_userlist.size();i++){
					if(strcmp(g_userlist[i].name,name)==0){
						friendID=i;
						for(vector<User>::iterator iter=g_userlist[userID].following.begin();iter!=g_userlist[userID].following.end();iter++){
							if(strcmp(iter->name,name)==0){
								g_userlist[userID].following.erase(iter);
								break;
							}
						}
						cout<<"["<<g_userlist[userID].name<<"] unfollows "<<name<<"."<<endl;
						break;
					}
				}
				if(i==g_userlist.size()){
					sendbuf="60";
					cout<<"Can't find the user."<<endl;
				}
			}
		}
	}
	iSendResult=send(ClientSocket,sendbuf,(int)strlen(sendbuf)+1,0);
	if(iSendResult==SOCKET_ERROR){
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
	}
	return friendID;

}
