#include "stdafx.h"
using namespace std;

//from Parsing.cpp
vector<string> makeTolkens(char* input, char knife);
int read_positive_integer();
//from ConductProtocol.cpp
int ready(SOCKET);
string getAllPackages(vector<string> tolkens, SOCKET ClientSocket);
//from deleteMessage.cpp
int deleteMessage(SOCKET ConnectSocket);


int manageTimelineDialogue(SOCKET ConnectSocket, bool userTimeline){
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen=DEFAULT_BUFLEN;
	bool userWantsMore=false;
	int n=0;//counter
	do{
		userWantsMore=false;

		int iRecieve = recv(ConnectSocket, recvbuf, recvbuflen, 0);//get messages
		//no messages:
		if(recvbuf[0]=='-' && recvbuf[1]=='1'){
			int iRecieve = send(ConnectSocket, "T0", 2, 0);//ready to exit [T0]
			iRecieve = recv(ConnectSocket, recvbuf, recvbuflen, 0);//[CP]
			if(userTimeline){
				cout<<"LAME! You have no messages...Write your first post"<<endl;
			}else{
				cout<<"OH, BOY...your friends haven't posted anything yet."<<endl;
			}
			//Go back to the menu
			return 0;

		}else if(n==0){
			if(userTimeline){
				cout<<"==============Your BAAAs=============="<<endl;
			}else{
				cout<<"=========Your friends' BAAAAs========="<<endl;
			}
		}
		//FORMAT: "01:NumberPackages:PackageNumber:Length:Package"
		vector<string> tolkens= makeTolkens(recvbuf, ':');

		char get[DEFAULT_BUFLEN];

		string gotMessages = getAllPackages(tolkens,ConnectSocket);
		//cout<<"Got the messages: "<<gotMessages<<endl;
		iRecieve = recv(ConnectSocket, get, recvbuflen, 0);//I will work now [T1]
		
		//now we have a big long string. how do we separate the messages? The | character
		char *c = new char[gotMessages.length() + 1];  
		strcpy_s(c, strlen(c)+1, gotMessages.c_str());
		vector<string> vMessages = makeTolkens(c, '|');

		//print the messages for the user
		for(string message: vMessages){
			cout<<"   "<<to_string(n)<<": "<<message<<endl;
			n++;
		}
		
		iRecieve = send(ConnectSocket, "T2", 2, 0);//ready for ya [T2]!
		iRecieve = recv(ConnectSocket, get, recvbuflen, 0);//are there more msgs? [T3]
		//cout<<"Server, are there any more messages? "<<get[0]<<get[10]<<endl;
		if(get[0]=='1'&&get[1]=='2'){//there are more messages
			//ask user: do you want more messages?
			bool noCommand=true;
			while(noCommand){
				cout<<"     [0] That's enough"<<endl<<"     [1] see more messages"<<endl;
				int command = read_positive_integer();
				if(command==0){
					ready(ConnectSocket);//done. return to main menu [T4]
					noCommand=false;
				}else if(command==1){
					userWantsMore=true;
					char* sendcmd = "12"; //says, send me more of my timeline [T4]
					send(ConnectSocket, sendcmd, (int)strlen(sendcmd), 0 ); 
					noCommand=false;
				}else{
					cout<<"Not a valid command!"<<endl;
				}
			}
		}else{//no more messages
			cout<<"No more messages"<<endl;
			ready(ConnectSocket);
		}

	}while(userWantsMore);

	char check[DEFAULT_BUFLEN];
	int iRecieve = recv(ConnectSocket, check, recvbuflen, 0);//waiting to exit [T5]
	iRecieve = send(ConnectSocket, "T5", 2, 0);//ready to exit [T6]
	iRecieve = recv(ConnectSocket, check, recvbuflen, 0);//[CP] from ConductProtocol;

	if(userTimeline){ //they can choose to delete a message from here
		cout<<"     [0] Return to main menu"<<endl;
		cout<<"     [1] Delete one of these messages"<<endl;
		int cmd = read_positive_integer();
		if(cmd==1){
			deleteMessage(ConnectSocket);
		}
	}

	return 0;
}

int printFriendsTimeline(SOCKET ConnectSocket){

	char *sendcmd = "03";
	int iSendResult = send(ConnectSocket, sendcmd, (int)strlen(sendcmd), 0 ); 
	
	manageTimelineDialogue(ConnectSocket,false);
	return 0;
}

int printUserTimeline(SOCKET ConnectSocket){

	char *sendcmd = "02";
	int iSendResult = send(ConnectSocket, sendcmd, (int)strlen(sendcmd), 0 ); 
	
	manageTimelineDialogue(ConnectSocket,true);
	return 0;
}