#include "stdafx.h"

using namespace std;

extern map<string,vector<Message>> g_messages;
extern vector<User> g_userlist;
extern mutex g_user_mutex;

/***********************************************************************
                         follow_handler()
***********************************************************************/
//50: Not found
//51: Follow him/herself
//52: Already followed
//53: Success
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

	iResult = send(ClientSocket, "F0", 2, 0);//ready for you to send the username [F0]
	iResult=recv(ClientSocket,recvbuf,recvbuflen,0); //[F1] user to follow
	if(iResult>0){
		recvbuf[iResult]='\0';
		char *name=(char*)malloc(strlen(recvbuf)+1);
		strcpy_s(name, sizeof(name)+1, recvbuf);

		g_user_mutex.lock();
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
						//The User Object of the user to follow is pushed to following<User>
						cout<<"["<<g_userlist[userID].name<<"] follows "<<name<<"."<<endl;
						sendbuf="53";
						break;
					}
				}
				if(i==g_userlist.size()){
					sendbuf="50";
					cout<<"Can't find the user."<<endl;
				}
			}
		}
		g_user_mutex.unlock();
	}

	iSendResult=send(ClientSocket,sendbuf,(int)strlen(sendbuf)+1,0);//[F2]
	if(iSendResult==SOCKET_ERROR){
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
	}

	int iRecieve = recv(ClientSocket, recvbuf, recvbuflen, 0);//ready to exit? [F3]
	//cout<<"exiting follow function"<<endl;
	return friendID;
}
/***********************************************************************
                         unfollow_handler()
***********************************************************************/
//60: User doesn't exist
//61: unfollow him/her self
//62: The user is not in the following list
//63: Success

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

	iResult = send(ClientSocket, "U0", 2, 0);//ready for you to send the username [U0]
	iResult = recv(ClientSocket,recvbuf,recvbuflen,0);//;[U1]
	if(iResult>0){
		recvbuf[iResult]='\0';
		char *name=(char*)malloc(strlen(recvbuf)+1);
		strcpy_s(name,sizeof(name)+1,recvbuf);
		g_user_mutex.lock();
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
						sendbuf="63";
						break;
					}
				}
				if(i==g_userlist.size()){
					sendbuf="60";
					cout<<"Can't find the user."<<endl;
				}
			}
		}
		g_user_mutex.unlock();
	}
	iSendResult=send(ClientSocket,sendbuf,(int)strlen(sendbuf)+1,0); //[U2]
	if(iSendResult==SOCKET_ERROR){
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
	}

	int iRecieve = recv(ClientSocket, recvbuf, recvbuflen, 0);//ready to exit? [U3]
	//cout<<"exiting unfollow function"<<endl;
	return friendID;
}
