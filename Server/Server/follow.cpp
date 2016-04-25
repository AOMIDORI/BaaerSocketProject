#include "stdafx.h"

using namespace std;

extern map<string,vector<Message>> g_messages;
extern vector<User> g_userlist;
extern mutex g_user_mutex;
extern mutex g_storage_mutex;

//from User.cpp
User getUser(int id);
//from Message.cpp
map<string,vector<Message>> loadFriendMessages(vector<int> friendList);
int update_g_messages(map<string,vector<Message>> friendMessages);
vector<int> whoToLoad(list<int> keepLocally);

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
		//have they already followed?
		for(i=0;i<g_userlist[userID].following.size();i++){
			User theFriend=getUser(g_userlist[userID].following[i]);
			if(strcmp(name,theFriend.name)==0){
				havefollowed=TRUE;
				sendbuf="52";
				break;
			}
		}
		//trying to follow themself
		if(strcmp(name,g_userlist[userID].name)==0){
			sendbuf="51";
		}
		else{
			if(!havefollowed){
				for(i=0;i<g_userlist.size();i++){
					if(strcmp(g_userlist[i].name,name)==0){
						//Add them to following
						//The ID of the user to follow is pushed to current user.following<int>
						g_userlist[userID].following.push_back(g_userlist[i].ID);
						cout<<"["<<g_userlist[userID].name<<"] follows "<<name<<"."<<endl;
						sendbuf="53";

						list<int> keepLocally;
						keepLocally.push_back(g_userlist[i].ID);
						//only load user if they're not in local storage yet
						vector<int> usersToLoad = whoToLoad(keepLocally);
						//load them to local storage
						loadFriendMessages(usersToLoad);

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
		//check if already following 
		for(i=0;i<g_userlist[userID].following.size();i++){
			User theFriend=getUser(g_userlist[userID].following[i]);
			if(strcmp(name,theFriend.name)==0){
				havefollowed=TRUE;
				break;
			}
		}
		if(!havefollowed) sendbuf="62";
		//trying to unfollow themself
		if(strcmp(name,g_userlist[userID].name)==0){
			sendbuf="61";
		}
		else{
			if(havefollowed){// remove from follow<int>
				for(i=0;i<g_userlist.size();i++){
					if(strcmp(g_userlist[i].name,name)==0){
						//find the friend's id in following<int>
						for(auto iter=g_userlist[userID].following.begin();iter!=g_userlist[userID].following.end();iter++){
							int friendID=*iter;
							User theFriend=getUser(friendID);
							if(strcmp(theFriend.name,name)==0){
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
