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
//from User.cpp
User getUser(int ID);

int printFriendsTimeline(SOCKET ClientSocket, int userID){

	g_user_mutex.lock();
	string currentUser=g_userlist[userID].name;
	g_user_mutex.unlock();

	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen=DEFAULT_BUFLEN;

	//get my messages from db
	//get 10 messages at a time
	int setSize = DEFAULT_BUNDLE; //max number of messages to send at one time

	vector<Message> friendMessages;//hold messages from all friends in following<User>
	//get the messages from each follower
	g_user_mutex.lock();
	vector<int> iFollow = g_userlist[userID].following;
	for(int friendID: iFollow){
		User myFriend=getUser(friendID);
		//get the messages of this friend
		g_message_mutex.lock();
		auto friendIt = g_messages.find(myFriend.name);
		if(friendIt !=g_messages.end()){
			int nFriendMsg = friendIt->second.size(); 
			//load all of the friend's messages
			for(int i=0; i<nFriendMsg; i++){
				Message thisMessage = friendIt->second.at(i); 
				friendMessages.push_back(thisMessage);
			}
		}//else, leave friendMessages empty
		g_message_mutex.unlock();
	}
	g_user_mutex.unlock();

	int nMessages = friendMessages.size();
	if(nMessages<1){
		send(ClientSocket, "-1", 2, 0 ); //No messages to show!
		int iRecieve = recv(ClientSocket, recvbuf, recvbuflen, 0);//ready to exit? [T0]

		return 0;
	}

	std::cout<<"found "<<friendMessages.size()<<" messages from the friends of "<<currentUser<<endl;
	
	//maybe, we should order the messages by timestamp. Right now all of 1 user's messages will be grouped

	//we will send messages in sets (of 10), not ALLLL of the messages
	int setNumber = 1;
	int numberOfSets = nMessages/setSize + 1;
	int startWith = 0; //which user message is the first that needs to be sent
	int iRecieve;

	//sending sets of DEFAULT_BUNDLE messages to client, asking if they want more
	do{ //while(there are more messages left)
		string sendString="";

		//now we have up to setSize (eg 10) messages in a vector. send them with | knife 
		for(Message s:friendMessages){
			string username = getUser(s.user).name;
			sendString+="@"+username+"  ";
			sendString+=s.content;
			sendString+="|";
		}

		//whole message I want to send: sendString
		//divide message into packages w length DEFAULT_BUFLEN
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
			ready(ClientSocket);
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
	//cout<<"exiting printTimeline function"<<endl;
	return 0;
}