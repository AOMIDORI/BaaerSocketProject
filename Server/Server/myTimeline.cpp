#include "stdafx.h"
using namespace std;

extern map<string,vector<Message>> g_messages;
extern vector<User> g_userlist;

//from parsing
vector<string> packageString(string input);
//from conduct protocol
int sendAllPackages(vector<string> messagePkgs, SOCKET ConnectSocket);
int ready(SOCKET);

int printMyTimeline(SOCKET ClientSocket, int userID){
	
	string currentUser=g_userlist[userID].name;

	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen=DEFAULT_BUFLEN;

	//get my messages from db
	int setSize = DEFAULT_BUNDLE; //max number of messages to send at one time

	//we need to get the messages of every user in the user's 'following' list
	
	auto it = g_messages.find(currentUser);
	if(it==g_messages.end()){
		send(ClientSocket, "-1", 2, 0 ); //I dont have any messages!
		int iRecieve = recv(ClientSocket, recvbuf, recvbuflen, 0);//ready to exit? [T0]
		return 0;
	}

	int nMessages = it->second.size();
	cout<<"found "<<nMessages<<" messages for user "<<currentUser<<endl;
	if(nMessages<1){
		send(ClientSocket, "-1", 2, 0 ); //I dont have any messages!
		return 0;
	}

	//get 10 messages at a time
	int setNumber = 1;
	int numberOfSets = nMessages/setSize + 1;
	int startWith = 0; //which user message is the first that needs to be sent

	do{
		vector<string> userMessages;
		string sendString="";

		for(int i=startWith; i<nMessages && i<(startWith + setSize ); i++){
			string message = it->second.at(i).content; 
			userMessages.push_back(message);
		}
		
		//now we have up to 10 messages in a vector. send them with | knife 
		for(string s:userMessages){
			sendString+=s;
			sendString+="|";
		}

		//whole message I want to send: userMessages
		//send it in packages, as normal
		//divide message into sets w length DEFAULT_BUFLEN
		vector<string> messagePkgs = packageString(sendString);

		int iSendResult = sendAllPackages(messagePkgs, ClientSocket);
		int iRecieve = recv(ClientSocket, recvbuf, recvbuflen, 0);//let me know whey you're ready [A]
		cout<<"sent messages"<<endl;


		//do we have more messages?
		if(startWith+setSize<nMessages){
			//ask if they want more or are done
			char* sendcmd = "12"; //I have more messages!
			cout<<"I have more messages..."<<endl;
			send(ClientSocket, sendcmd, (int)strlen(sendcmd), 0 ); //ready to get server info
		}else{//no more messages available
			cout<<"no more messages. sending 00"<<endl;
			ready(ClientSocket);
		}
		iRecieve = recv(ClientSocket, recvbuf, recvbuflen, 0);//0 done, 12 print more [B]
		//cout<<"Client says: "<<recvbuf<<endl;
		int cmd = atoi(recvbuf);
		if(cmd != 12){//nope, they don't want any more
			break;
		}

			startWith += setSize;
			setNumber += 1;
	}while(startWith < nMessages);


	int iRecieve = recv(ClientSocket, recvbuf, recvbuflen, 0);//ready to exit? [C]
	cout<<"exiting printTimeline function"<<endl;
	return 0;
}