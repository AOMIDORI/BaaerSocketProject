#include "stdafx.h"
using namespace std;

extern map<string,vector<Message>> g_messages;
extern vector<User> g_userlist;
extern mutex g_user_mutex;
extern mutex g_message_mutex;

//from parsing
vector<string> packageString(string input);
//from conduct protocol
int sendAllPackages(vector<string> messagePkgs, SOCKET ConnectSocket);
int ready(SOCKET);

int printMyTimeline(SOCKET ClientSocket, int userID){
	
	g_user_mutex.lock();
	string currentUser=g_userlist[userID].name;
	g_user_mutex.unlock();

	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen=DEFAULT_BUFLEN;
	int iRecieve;

	//get my messages from db
	int setSize = DEFAULT_BUNDLE; //max number of messages to send at one time

	//we need to get the messages of every user in the user's 'following' list
	g_message_mutex.lock();
	auto it = g_messages.find(currentUser);
	auto endIt = g_messages.end();
	g_message_mutex.unlock();

	if(it==endIt){
		send(ClientSocket, "-1", 2, 0 ); //I dont have any messages!
		iRecieve = recv(ClientSocket, recvbuf, recvbuflen, 0);//ready to exit? [T0]
		return 0;
	}

	g_message_mutex.lock();
	int nMessages = it->second.size();
	g_message_mutex.unlock();
	cout<<"found "<<nMessages<<" messages for user "<<currentUser<<endl;
	if(nMessages<1){
		send(ClientSocket, "-1", 2, 0 ); //I dont have any messages!
		iRecieve = recv(ClientSocket, recvbuf, recvbuflen, 0);//ready to exit? [T0]
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
			g_message_mutex.lock();
			string message = it->second.at(i).content; 
			g_message_mutex.unlock();
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
		iRecieve = send(ClientSocket, "T1", 2, 0);//ready to proceed? [T1]
		iRecieve = recv(ClientSocket, recvbuf, recvbuflen, 0);//let me know whey you're ready [T2]
		cout<<"sent messages"<<endl;


		//do we have more messages?
		if(startWith+setSize<nMessages){
			//ask if they want more or are done
			char* sendcmd = "12"; //I have more messages!
			cout<<"I have more messages..."<<endl;
			send(ClientSocket, sendcmd, (int)strlen(sendcmd), 0 ); //ready to get server info [T3]
		}else{//no more messages available
			cout<<"no more messages. sending 00"<<endl;
			ready(ClientSocket); //[T3]
		}
		iRecieve = recv(ClientSocket, recvbuf, recvbuflen, 0);//0 done, 12 print more [T4]
		//cout<<"Client says: "<<recvbuf<<endl;
		int cmd = atoi(recvbuf);
		if(cmd != 12){//nope, they don't want any more
			break;
		}

			startWith += setSize;
			setNumber += 1;
	}while(startWith < nMessages);

	iRecieve = send(ClientSocket, "T5", 2, 0);//waiting to exit? [T5]
	iRecieve = recv(ClientSocket, recvbuf, recvbuflen, 0);//ready to exit? [T6]
	cout<<"exiting printTimeline function"<<endl;
	return 0;
}