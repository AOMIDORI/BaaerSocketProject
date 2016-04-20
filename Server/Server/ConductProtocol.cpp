#define WIN32_LEAN_AND_MEAN

#include "stdafx.h"

using namespace std;

//from Parsing.cpp
vector<string> makeTolkens(char* input, char knife);
//from addMessage.cpp
int addMessage(vector<string> tolkens, SOCKET ClientSocket, int userID);
//from myTimeline.cpp
int printMyTimeline(SOCKET, int userID);
//from friendsTimeline.cpp
int printFriendsTimeline(SOCKET, int userID);
//from deleteMessage
int deleteMessage(SOCKET ClientSocket, vector<string> tolkens, int userID);
//from initializeSample.cpp
int initialize(int userID);
//from server.cpp
int follow_handler(int userID, SOCKET clntSocket);
int unfollow_handler(int userID, SOCKET clntSocket);

extern vector<User> g_userlist;

int ready(SOCKET ClientSocket){ //tell the client you are ready for the next instruction

	char *sendbuf = "00";
	int iSendResult = send(ClientSocket, sendbuf, (int)strlen(sendbuf), 0 ); 

	return 0;
}

int readyCP(SOCKET ClientSocket){ //tell the client you are waiting in conduct protocol

	char *sendbuf = "CP";
	int iSendResult = send(ClientSocket, sendbuf, (int)strlen(sendbuf), 0 ); 

	return 0;
}



int conduct_protocol(SOCKET ClientSocket, int userID){
	string currentUser=g_userlist[userID].name;

 // Receive until the peer shuts down the connection
    int iResult;
    int iSendResult;
	
	int recvbuflen=DEFAULT_BUFLEN;

	if(g_userlist.size()<2){
		initialize(userID);//creates users Bill and Jim, Jim is following Bill. Bill has a message
	}

    do {
		cout<<"waiting for next user command at CondutProtocol"<<endl;

		char recvbuf[DEFAULT_BUFLEN];
		//get the command of the user
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);

        if (iResult > 0) {
			//we recieved a message
		   vector<string> tolkens= makeTolkens(recvbuf,':');
		   
		   //get the command
		   string command = tolkens.at(0);
		   command = command.substr(0,2); 
		   int intCommand = stoi(command);
		   cout <<"Command: " << intCommand << endl;

		   switch(intCommand){
		   case 0:
			   return 0;
		   case 1: 
			   addMessage(tolkens, ClientSocket, userID);
			   break;
		   case 2: 
			   printMyTimeline(ClientSocket, userID);
			   break;
		   case 3:
			   printFriendsTimeline(ClientSocket, userID);
			   break;
		   case 4: 
			   follow_handler(userID,ClientSocket);
			   break;
		   case 5: 
			   unfollow_handler(userID,ClientSocket);
			   break;
		   case 6: 
			   //delete message
			   deleteMessage(ClientSocket,tolkens,userID);
			   break;
		   default:
			   cout<<"Unknown command"<<endl;
		   }

		   /*while(iflogin){
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
		}*/


		   readyCP(ClientSocket);

        }
        else if (iResult == 0)
            printf("Connection closing...\n");
        else  {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

    } while (iResult > 0);

 return 0;
}

string getAllPackages(vector<string> tolkens, SOCKET ClientSocket){
	//message format: 
	//01:NP:PN:LENGH:message
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen=DEFAULT_BUFLEN;

	int numberOfPackages = stoi(tolkens.at(1));
	int packageNumber = stoi(tolkens.at(2));
	int lengthChars = stoi(tolkens.at(3));

	if (lengthChars==0 || !lengthChars){
		return 0;
	}

	string message=tolkens.at(4);
	message=message.substr(0,lengthChars);

	while(packageNumber<numberOfPackages){

		ready(ClientSocket);//ask for more

		int iRecieve = recv(ClientSocket, recvbuf, recvbuflen, 0);
		vector<string> newTolkens = makeTolkens(recvbuf,':');

		int code = stoi(tolkens.at(0));
		
		if(code==1){
			//they sent more of the message
			string messageContinued=newTolkens.at(4);
			int newLengthChars = stoi(newTolkens.at(3));
			packageNumber = stoi(newTolkens.at(2));
			messageContinued = messageContinued.substr(0,newLengthChars);
			message.append(messageContinued);
			//cout<<"new package "<<packageNumber<<" of "<<numberOfPackages<<" says: "<<messageContinued<<endl;
			
		}else{
			//error: unexpected message code from client (expected 01)
			cout<<"Error: unexpected command from client";
			closesocket(ClientSocket);
            WSACleanup();
            return "";
		}
		
	}

	//now we have the whole message
	//cout<<"The whole message: "<<message<<endl;
	ready(ClientSocket);//ask for more

	return message;
}


int sendAllPackages(vector<string> messagePkgs, SOCKET ConnectSocket){

	char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
	int iResponse=-1;

	//divide message into sets w length DEFAULT_BUFLEN

	int nPackages = messagePkgs.size();
	
	for(int i=0; i<nPackages; i++){
		//send each of the packages and wait for the server for ask for each one
		int package = i; //index from 0
		string message = messagePkgs[i];
		int length=message.length();

	
		// Send package
		//FORMAT: "01:NumberPackages:PackageNumber:LENGH:message"
		string formattedMsg="01:"+to_string(nPackages) + ":"+to_string(package)+":"+to_string(length)+":"+message;
		const char* sendbuf=formattedMsg.c_str();

		iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );
		cout<<"sent message: "<<sendbuf<<endl;

		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return 1;
		}

		//now listen for the client to say: 00 (waiting for more...)
		int iRecieve = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		
		iResponse = atoi(recvbuf);
		//client is waiting for more if it says 0
		if(iResponse != 0){
			//I guess we're done
			//error
			cout<<"Error, unexpected server response. The server said: "<<iResponse<<endl;
			return 1;
		}

	}
	return 0;
}


