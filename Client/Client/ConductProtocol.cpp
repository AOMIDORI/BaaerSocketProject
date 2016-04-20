#include "stdafx.h"
using namespace std;

//Function declarations from other files

//readFunctions.cpp
int read_positive_integer();
string read_string(const string& message);
//sendMessage.cpp
int sendMessage(SOCKET ConnectSocket, std::string message);
vector<string> packageString(string input);
int newMessage(SOCKET);
//timeline.cpp
int printFriendsTimeline(SOCKET ConnectSocket);
int printUserTimeline(SOCKET ConnectSocket);
//Parsing.cpp
vector<string> makeTolkens(char* input, char knife);
//follow.cpp
int follow(SOCKET ConnectSocket);
int unfollow(SOCKET ConnectSocket);

int ready(SOCKET ClientSocket){ //tell the client you are ready for the next instruction
	char *sendbuf = "00";
	int iSendResult = send(ClientSocket, sendbuf, (int)strlen(sendbuf), 0 ); 

	return 0;
}

string getAllPackages(vector<string> tolkens, SOCKET ClientSocket){
	//message format: 
	//##:NP:PN:LENGH:message
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

	packageNumber++;//if there's only one package, we're done
	while(packageNumber<numberOfPackages){
		
		ready(ClientSocket);//ask for more

		int iRecieve = recv(ClientSocket, recvbuf, recvbuflen, 0);
		vector<string> newTolkens = makeTolkens(recvbuf,':');

		int code = stoi(tolkens.at(0));
		
		if(code>0){
			//they sent more of the message
			string messageContinued=newTolkens.at(4);
			int newLengthChars = stoi(newTolkens.at(3));
			packageNumber = stoi(newTolkens.at(2));
			messageContinued = messageContinued.substr(0,newLengthChars);
			message.append(messageContinued);
			//cout<<"new package "<<packageNumber<<" of "<<numberOfPackages<<" says: "<<messageContinued<<endl;
			
		}else{
			//error: unexpected message code from client
			cout<<"Error: unexpected command from server";
			closesocket(ClientSocket);
            WSACleanup();
            return "";
		}
		
	}

	//now we have the whole message
	//cout<<"The whole message: "<<message<<endl;
	ready(ClientSocket);	
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
		int package = i+1; //index from 1
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
			//uh oh. I guess we're done
			//error
			cout<<"Error, unexpected server response. The server said: "<<iResponse<<endl;
			return 1;
		}

	}
	return 0;
}

int askForCommand(){
	cout<<endl;
	cout<<"What would you like to do? (enter a number)"<<endl;
	cout<<"   0: I'm done! Log me out"<<endl;
	cout<<"   1: Post a new message"<<endl;
	cout<<"   2: View my timeline"<<endl;
	cout<<"   3: View my friends' posts"<<endl;
	cout<<"   4: Follow a user"<<endl;
	cout<<"   5: Unfollow a user"<<endl;
	cout<<"   To Delete a message, view your timeline"<<endl;

	int command = read_positive_integer();
	return command;
}

int conductProtocol(SOCKET ConnectSocket){
	 // Ask the user what they want to do next
	int iRecieve = 0;
    do {

		int command = askForCommand();
		if(command == 0){
			ready(ConnectSocket);
			char check[DEFAULT_BUFLEN];
			iRecieve = recv(ConnectSocket, check, DEFAULT_BUFLEN, 0);//[99] from ConductProtocol;
			cout<<"You have been logged out. Exiting"<<endl;
			//logout
			break;
		}
		switch(command){
		case 1:
			//the functions return the last response of the server, which should be 0 if its ready
			iRecieve = newMessage(ConnectSocket);
			break;
		case 2:
			iRecieve = printUserTimeline(ConnectSocket);
			break;
		case 3: 
			iRecieve = printFriendsTimeline(ConnectSocket);
			break;
		case 4:
			iRecieve = follow(ConnectSocket);
			break;
		case 5: 
			iRecieve = unfollow(ConnectSocket);
			break;

		//delete: isn't done from main menu. must user Print My Timeline [2] first
		default:
			cout<<"This is not a valid command! Please enter another one";
			continue;
		}
		

    } while( iRecieve > -1 );
	return 0;
}