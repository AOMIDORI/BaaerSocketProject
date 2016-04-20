#include "stdafx.h"
using namespace std;

//parsing.cpp
string read_string(const string& message);

int follow(SOCKET ConnectSocket){
	char *sendbuf = "";
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	char check[DEFAULT_BUFLEN];
	string s;

	sendbuf = "04";
	int iSendResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 ); 

	int iRecieve = recv(ConnectSocket, check, recvbuflen, 0);//checkpoint [F0]
	s=read_string("Who do you want to follow? ");
	sendbuf=(char*)s.c_str();
	if(send(ConnectSocket,sendbuf,(int)strlen(sendbuf)+1,0)==SOCKET_ERROR){ //[F1]
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	int iResult=recv(ConnectSocket,recvbuf,recvbuflen,0);//[F2]

	recvbuf[iResult]='\0';
	//cout<<"recvbuf: "<<recvbuf<<endl;

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
	
	iResult = send(ConnectSocket, "F3", 2, 0);//ready to exit [F3]
	iRecieve = recv(ConnectSocket, check, recvbuflen, 0);//[CP] from ConductProtocol;
	return 0;
}

int unfollow(SOCKET ConnectSocket){
	char *sendbuf = "";
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	char check[DEFAULT_BUFLEN];
	string s;

	sendbuf = "05";
	int iSendResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 ); 

	int iRecieve = recv(ConnectSocket, check, recvbuflen, 0);//checkpoint [U0]

	s=read_string("Who do you want to unfollow? ");
	sendbuf=(char*)s.c_str();
	if(send(ConnectSocket,sendbuf,(int)strlen(sendbuf)+1,0)==SOCKET_ERROR){//[U1]
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	int iResult=recv(ConnectSocket,recvbuf,recvbuflen,0);//[U2]
	if(iResult>0){
		recvbuf[iResult]='\0';
		if(strcmp(recvbuf,"60")==0){
			cout<<"Sorry, the user ["<<s<<"] doesn't exist."<<endl;
		}
		else if(strcmp(recvbuf,"61")==0){
			cout<<"Sorry, you can't unfollow yourself."<<endl;
		}
		else if(strcmp(recvbuf,"62")==0){
			cout<<"Sorry, ["<<s<<"] is not your friend."<<endl;
		}
		else{
			cout<<"You unfollow ["<<s<<"] successfully."<<endl;
		}
	}

	iResult = send(ConnectSocket, "U3", 2, 0);//ready to exit [U3]
	iRecieve = recv(ConnectSocket, check, recvbuflen, 0);//[CP] from ConductProtocol;
	return 0;
}