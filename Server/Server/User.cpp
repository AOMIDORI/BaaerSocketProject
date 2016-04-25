#include "stdafx.h"
using namespace std;

extern int allMessageIds;
extern vector<string> g_namelist;
extern vector<User> g_userlist;
extern mutex g_user_mutex;

User getUser(int id){
	User def=User();
	for(User u:g_userlist){
		if(u.ID==id){
			return u;
		}
	}
	return def;
}

//for writing and reading to permanent storage
// Insertion operator
std::ostream& operator<<(std::ostream& os, const User& s)
{
	// write out individual members of s with an end of line between each one
	os << s.ID << '\n';
	os << s.name << '\n';
	os << s.status << '\n';
	int sizeFollowing = s.following.size();
	os << sizeFollowing << '\n';
	for(int i=0; i<sizeFollowing; i++){
		os << s.following[i] << '\n';
	}
	return os;
}

// Extraction operator
std::istream& operator>>(std::istream& is, User& s)
{
	// read in individual members of s
	//format: ID \ name \ status \ #following \following Id0 \ followingId1....
	int sizeFollowing;
	vector<int> followingVector;
	is >> s.ID;
	string n;
	is >> n;

	//C++ bs..
	char *c = new char[n.length() + 1];
	strcpy_s(c, strlen(c)+1, n.c_str());
	s.name=c;

	is >> s.status;
	is >> sizeFollowing;
	for (int i=0; i<sizeFollowing; i++){
		int idFollowing;
		is >> idFollowing;
		followingVector.push_back(idFollowing);
	}
	s.following=followingVector;
	return is;
}

User::User(){
	if(g_user_mutex.try_lock()){
		ID=g_userlist.size();
		name="default";
		status=0;
		g_user_mutex.unlock();
		}
		else{
			ID=g_userlist.size();
		}
	return;
}



// Write whole user db to file
int writeUserDB()
{
	std::ofstream ofs("UserDB.txt");

	g_user_mutex.lock();
	int numberUsers = g_userlist.size();
	ofs << numberUsers<< '\n';
	for(User u:g_userlist){
		ofs<<u;
	}
	g_user_mutex.unlock();
	ofs.close();
	return 0;
}


vector<User> readUserDB(){

	vector<User> userDB;
	vector<string> namelist;
	//read: #users, user1[...], user2[...], ...
	std::ifstream ifs("UserDB.txt");
	int numberUsers;
	if(ifs>>numberUsers){
		User newUser = User();
		for(int i=0; i<numberUsers; i++){
			ifs>>newUser;
			userDB.push_back(newUser);
			namelist.push_back(newUser.name);
		}
	}

	//we should also update g_namelist
	g_namelist = namelist;
	return userDB;
	
}