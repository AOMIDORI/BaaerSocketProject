#include "stdafx.h"
using namespace std;

extern int allMessageIds;
extern vector<User> g_userlist;
extern map<string,vector<Message>> g_messages;
extern mutex g_message_mutex;
extern mutex g_storage_mutex;
extern mutex g_user_mutex;

//User.cpp
User getUser(int id);

Message::Message(string text, int thisUser){
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
		Message m("",-1);
		// read in individual members of s
		string hold;
		string id= to_string(m.id);
		is >> id;
		char contentA[5120];
		is.getline(contentA, 5120, '\n');//first we have to get to the next line
		is.getline(contentA, 5120, '\n');
		m.content = contentA;
		string userID;
		is>> m.timeStamp >> userID >> hold;
		m.user = atoi(userID.c_str());
		s=m;
		return is;
	}

// Write whole db
int writeMsgDB()
{
	std::ofstream ofs("messageDB.txt");
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
	std::ifstream ifs("messageDB.txt");
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
					Message m("",-1);
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

int update_g_messages(map<string,vector<Message>> friendMessages){
	//friend messages has been retrieved from permanent storage
	for(auto it = friendMessages.begin(); it!=friendMessages.end(); it++){
		string name=it->first;
		vector<Message> messages = it->second;
		//include these friends in the local storage
		g_messages[name]=messages;
	}
	return 0;
}

map<string,vector<Message>> loadFriendMessages(vector<int> friendList){

	map<string,vector<Message>> messageDB;
	std::ifstream ifs("messageDB.txt");
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
				//messageDB[aUser];
				ifs>>nMessages;
				
				for(int j=0; j<nMessages; j++){
					Message m("",-1);
					ifs >> m;//put the id, content, timestamp and username using override of << operator
					//only add this user if they are in list of users to read: (NOT already in g_messages)
					for(int id:friendList){
						if(id==m.user){
							cout<<"The user "<<aUser<<" has "<<nMessages<<" messages to load"<<endl;
							messageDB[aUser];
							messageDB[aUser].push_back(m);
							break;
						}
					}
					
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

	update_g_messages(messageDB);
	return messageDB;
}

// Update permanent storage with the data in g_messages
int updateMsgDB(){
	//here we are temporarily copying the entire permanent database so that we can update it 
	//(Not the ideal solution, but without querying like SQL or something its hard to avoid)

	map<string,vector<Message>> pDB=readMsgDB();
	
	g_message_mutex.lock();

	//update the entry in the permanent storage for this user
	auto localIt = g_messages.begin();
	while(localIt != g_messages.end()){
		string username = localIt->first;
		vector<Message> messages = localIt->second; //this user's messages
		pDB[username]=messages;

		localIt++;
	}
	g_message_mutex.unlock();

	//now rewrite the database
	std::ofstream ofs("messageDB.txt");
	int numberUsers = pDB.size();
	ofs << numberUsers<< '\n';

	auto it = pDB.begin();
	while(it != pDB.end()){

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
	ofs.close();
	return 0;
}

vector<int> whoToLoad(list<int> keepLocally){
	//load all users in keepLocally who are NOT already in g_messages
	vector<int> whoToLoad;

	//wow, I really should have used the id not username as the key
	g_message_mutex.lock();

	for(int checkId:keepLocally){
		//check if they in usersAlreadyStored
		User user = getUser(checkId);
		string username = user.name;
		auto it = g_messages.find(username);
		if(it == g_messages.end()){//this user is not in the database
			whoToLoad.push_back(checkId);
		}
		
	}
	g_message_mutex.unlock();
	
	return whoToLoad;
}

int cleanLocalStorage(){
	//if users aren't logged in or followed, remove from local g_messages
	list<int> usersToKeep;
	g_user_mutex.lock();
	for(User u: g_userlist){
		if(u.status==1){
			usersToKeep.push_back(u.ID);
			for(int aFriend:u.following){
				usersToKeep.push_back(aFriend);
			}
		}
	}
	g_user_mutex.unlock();

	//only keep users who are logged in or followed. others are removed from local storage
	g_message_mutex.lock();
	auto it = g_messages.begin();
	while(it!=g_messages.end()){
		bool remove = true;
		for(int keepUser:usersToKeep){
			string name=getUser(keepUser).name;
			if(strcmp(it->first.c_str(),name.c_str())==0){
				remove = false;
			}
		}
		auto it2=it;
		it++;
		if(remove){
			g_messages.erase(it2);
		}
	}
	g_message_mutex.unlock();
	return 0;
}