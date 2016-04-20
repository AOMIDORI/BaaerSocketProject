#include "stdafx.h"
using namespace std;
//
////from Parsing.cpp
//vector<string> makeTolkens(char* input, char knife);
//int read_positive_integer();
////from ConductProtocol.cpp
//int ready(SOCKET);
//string getAllPackages(vector<string> tolkens, SOCKET ClientSocket);
//
//int printUserTimeline(SOCKET ConnectSocket){
//	char recvbuf[DEFAULT_BUFLEN];
//	int recvbuflen=DEFAULT_BUFLEN;
//	char *sendcmd = "02";
//	bool userWantsMore=false;
//	int iSendResult = send(ConnectSocket, sendcmd, (int)strlen(sendcmd), 0 ); 
//	int n=0;//counter
//	
//	do{
//		userWantsMore=false;
//
//		int iRecieve = recv(ConnectSocket, recvbuf, recvbuflen, 0);
//		//FORMAT: "02:NumberPackages:PackageNumber:Length:Package"
//		vector<string> tolkens= makeTolkens(recvbuf, ':');
//		string gotMessages = getAllPackages(tolkens,ConnectSocket);
//		//cout<<"Got the messages: "<<gotMessages<<endl;
//		
//		//now we have a big long string. how do we separate the messages? The | character
//		char *c = new char[gotMessages.length() + 1];
//		strcpy_s(c, strlen(c)+1, gotMessages.c_str());
//		vector<string> userMessages = makeTolkens(c, '|');
//
//
//		for(string message: userMessages){
//			cout<<to_string(n)<<": "<<message<<endl;
//			n++;
//		}
//
//
//		
//		iRecieve = recv(ConnectSocket, recvbuf, recvbuflen, 0);//are there more msgs?
//		int cmd = atoi(recvbuf);
//		cout<<"Server, are there any more messages? "<<cmd<<endl;
//		if(cmd==12){//there are more messages
//			//ask user: do you want more messages?
//			bool noCommand=true;
//			while(noCommand){
//				cout<<"[0] Return to main menu"<<endl<<"[1] see more messages"<<endl;
//				int command = read_positive_integer();
//				if(command==0){
//					ready(ConnectSocket);//done. return to main menu
//					noCommand=false;
//				}else if(command==1){
//					userWantsMore=true;
//					sendcmd = "12"; //says, send me more of my timeline
//					send(ConnectSocket, sendcmd, (int)strlen(sendcmd), 0 ); 
//					noCommand=false;
//				}else{
//					cout<<"Not a valid command!"<<endl;
//				}
//			}
//		}else{//no more messages
//			cout<<"No more messages"<<endl;
//			ready(ConnectSocket);
//		}
//
//	}while(userWantsMore);
//
//	int iRecieve = recv(ConnectSocket, recvbuf, recvbuflen, 0);
//	return 0;
//}