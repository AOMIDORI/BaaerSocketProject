#include "stdafx.h"
using namespace std;

extern int allMessageIds;
extern vector<User> g_userlist;
extern map<string,vector<Message>> g_messages;
extern mutex g_message_mutex;

Message::Message(string text, string thisUser){
		id=allMessageIds++;
		content = text;
		timeStamp= GetCurrentTime();
		user = thisUser;
		return;
}

//for writing and reading to permanent storage
// Insertion operator
std::ostream& operator<<(std::ostream& os, const Message& s)
	{
		// write out individual members of s with an end of line between each one
		os << s.id << '\n';
		os << s.content << '\n';
		os << s.timeStamp << '\n';
		os << s.user << '\n';
		os << 'E' <<'\n';
		return os;
	}

// Extraction operator
std::istream& operator>>(std::istream& is, Message& s)
	{
		Message m("","");
		// read in individual members of s
		string hold;
		is >> m.id;// >> m.content;
		char contentA[5120];
		is.getline(contentA, 5120, '\n');//first we have to get to the next line
		is.getline(contentA, 5120, '\n');
		m.content = contentA;
		is>> m.timeStamp >> m.user >> hold;
		s=m;
		return is;
	}

// Write whole db
int writeMsgDB()
{
	std::ofstream ofs("saved.txt");
	g_message_mutex.lock();
	int numberUsers = g_messages.size();
	ofs << numberUsers<< '\n';


	auto it = g_messages.begin();
	while(it != g_messages.end()){

		string username = it->first;
		ofs<<username<< '\n';

		vector<Message> messages = it->second; //this user's messages

		int numberMessages=messages.size();
		ofs<<numberMessages<< '\n';
		for(Message m: messages){
			ofs<<m;
		}
		ofs<<'F'<<'\n';
		it++;
	}
	g_message_mutex.unlock();

	ofs.close();

	return 0;

		
}


map<string,vector<Message>> readMsgDB(){

	map<string,vector<Message>> messageDB;
	std::ifstream ifs("saved.txt");
	int numberUsers;
	if(ifs>>numberUsers){
		//read was successful. we know how many users are in the message db
		//DB format: numberUsers
		//				username1
		//				#msg
		//					m1: ID \n content \n timestamp \n user
		//					m2...
		//				user2...
		//
		//g_users local format: map<username, vector<Message>>
		for(int i=0; i<numberUsers; i++){
			string aUser;
			int nMessages;

			ifs>>aUser;
			if(aUser.length()>0){
				messageDB[aUser];
				ifs>>nMessages;
				cout<<"The user "<<aUser<<" has "<<nMessages<<" messages to load"<<endl;
				for(int j=0; j<nMessages; j++){
					Message m("",aUser);
					ifs >> m;//put the id, content, timestamp and username using override of << operator
					messageDB[aUser].push_back(m);
				}
				string endUser;//debugging check
				ifs>>endUser; //F
			}
			if(ifs.eof()){
				cout<<"Reached end of file"<<endl;
			}
		}

	}
	ifs.close();
	return messageDB;
	
}