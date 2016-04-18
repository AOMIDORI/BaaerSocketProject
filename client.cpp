#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

using namespace std;

int __cdecl main(int argc, char **argv) 
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    char *sendbuf = "";
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
	bool ifconnect=FALSE;
    
    // Validate the parameters
    if (argc != 2) {
        printf("usage: %s server-name\n", argv[0]);
        return 1;
    }

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }		
        break;
    }

	
    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    // Send the username------------------------------------------------------
	cout<<"Hello, welcome. Your username?"<<endl;
	string username;
	cin>>username;

	sendbuf=(char*)username.c_str();
	
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf)+1, 0 );
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

	//Receive log info-----------------------------------------------------------
	iResult=recv(ConnectSocket, recvbuf, recvbuflen, 0);
	if(iResult>0){
		if(recvbuf[0]=='0'&&recvbuf[1]=='0'){
			cout<<"Successful login."<<endl;
			ifconnect=TRUE;
		}
		else{
			cout<<"The authentication fails. The user is already being used by another client."<<endl;
			return 0;
		}
	}

	//Main menu--------------------------------------------------------------------

	while(ifconnect){
		cout<<"==================Main menu====================="<<endl;
		cout<<"1. Send a message."<<endl;
		cout<<"2. Delete a message."<<endl;
		cout<<"3. Print user timeline."<<endl;
		cout<<"4. Print timeline of friends."<<endl;
		cout<<"5. Follow someone."<<endl;
		cout<<"6. Unfollow someone."<<endl;
		cout<<"7. Log out." <<endl;
		cout<<"-------------------------------------------------"<<endl;

		char choice;
		cout<<"Your choice number: ";
		cin>>choice;
		sendbuf[0]=choice;
		if(send(ConnectSocket,sendbuf,(int)strlen(sendbuf)+1,0)==SOCKET_ERROR){
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return 1;
		}
		string s;
		switch (choice){
		case '7'://Log out----------------------------------------------------------
			cout<<"Log out. Bye."<<endl;
			// shutdown the connection since no more data will be sent
			iResult = shutdown(ConnectSocket, SD_SEND);
			if (iResult == SOCKET_ERROR) {
				printf("shutdown failed with error: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
			return 0;
			break;
		case '1'://Send message-------------------------------------------------------
			cout<<"Please type in your Message: "<<endl;
			//string s;
			getchar();
			getline(cin,s);
			sendbuf=(char*)s.c_str();
			if(send(ConnectSocket,sendbuf,(int)strlen(sendbuf)+1,0)==SOCKET_ERROR){
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
			break;
		case '5': //Follow--------------------------------------------------------------
			cout<<"Who do you want to follow?"<<endl;
			//string s;
			cin>>s;
			sendbuf=(char*)s.c_str();
			if(send(ConnectSocket,sendbuf,(int)strlen(sendbuf)+1,0)==SOCKET_ERROR){
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
			iResult=recv(ConnectSocket,recvbuf,recvbuflen,0);
			if(iResult>0){
				recvbuf[iResult]='\0';
				if(strcmp(recvbuf,"50")==0){
					cout<<"Sorry, the user ["<<s<<"] doesn't exist."<<endl;
				}
				else if(strcmp(recvbuf,"51")==0){
					cout<<"Sorry, you can't follow yourself."<<endl;
				}
				else if(strcmp(recvbuf,"52")==0){
					cout<<"Sorry, you have already followed ["<<s<<"]."<<endl;
				}
				else{
					cout<<"You follow ["<<s<<"] successfully."<<endl;
				}
			}
			break;
		default:
			cout<<"Wrong Selection"<<endl;
			break;
		}
	}

    // cleanup

    closesocket(ConnectSocket);
    WSACleanup();

	getchar();
    return 0;
}
