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
#include "Database.h"

using namespace std;

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int g_usercount=0;

vector<string> g_namelist;
vector<User> g_userlist;

int iResult;
int iSendResult;

SOCKET ListenSocket = INVALID_SOCKET;
SOCKET ClientSocket = INVALID_SOCKET;

void connection_handler(void);

int __cdecl main(void) 
{
    WSADATA wsaData;
    

    struct addrinfo *result = NULL;
    struct addrinfo hints;

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
    while(ClientSocket = accept(ListenSocket, NULL, NULL)){
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}
		threads.push_back(new thread(connection_handler));
		g_usercount++;
	}
	cout<<"g_usercount: "<<g_usercount<<endl;
	for(int i=0;i<g_usercount;i++){
		threads[i]->join();
	}

	closesocket(ListenSocket);
    WSACleanup();

	

	getchar();
    return 0;
}

void connection_handler(){
	char *sendbuf=NULL;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
	int iflogin;
	
	if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			exit(1);
		}
	    int userID; 
		//Send the welcome info to the user, ask for the username.---------------------
		sendbuf="Hello client! Please type in your username:";
		if(send(ClientSocket, sendbuf, (int)strlen(sendbuf),0)==SOCKET_ERROR){
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			exit(1);
		}

		//Receive the username from the client------------------------------------------
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			recvbuf[iResult]='\0';
			string username(recvbuf);
			cout<<"Name received: ["<<username<<"], checking..."<<endl;
			if(find(g_namelist.begin(),g_namelist.end(),username)!=g_namelist.end()){
				cout<<"The user exists."<<endl;
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
				if(send(ClientSocket, sendbuf, (int)strlen(sendbuf),0)==SOCKET_ERROR){
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ClientSocket);
					WSACleanup();
					exit(1);
				}

			}
			else{
				sendbuf="01"; //The user can't log in
				if(send(ClientSocket, sendbuf, (int)strlen(sendbuf),0)==SOCKET_ERROR){
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
		//else  {
		//printf("recv failed with error: %d\n", WSAGetLastError());
		//closesocket(ClientSocket);
		//WSACleanup();
		//exit(1);
		//}

		//Receive menu choice from client ------------------------------------------
		if(recv(ClientSocket,recvbuf,recvbuflen,0)>0){
			if(recvbuf[0]=='7'){ //log out
				g_userlist[userID].status=0;				
			}
			else if(recvbuf[0]=='1'){ //send a message
			}
		}


		 //shutdown the connection since we're done
		/*iResult = shutdown(ClientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			printf("shutdown failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			exit(1);
		}*/
		closesocket(ClientSocket);
}
