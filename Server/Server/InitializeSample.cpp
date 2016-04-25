#include "stdafx.h"
using namespace std;

extern map<string,vector<Message>> g_messages;
extern vector<User> g_userlist;
extern mutex g_user_mutex;
extern mutex g_message_mutex;



int initialize(int userID){

	g_user_mutex.lock();
	string currentUser=g_userlist[userID].name;
	g_user_mutex.unlock();

	User bill = User();
	bill.name="Bill";
	g_user_mutex.lock();
	g_userlist.push_back(bill);
	g_user_mutex.unlock();

	User jim= User(); 
	jim.name="Jim";
	jim.status=1;
	jim.following.push_back(bill.ID); //Jim is following bill. bob is the example user

	g_user_mutex.lock();
	g_userlist.push_back(jim);
	g_user_mutex.unlock();
	

	g_message_mutex.lock();
	g_messages["Bill"]; 

	//add a message for bill
	auto billIt = g_messages.find("Bill");
	Message newMessage = Message("Bill Says Hi",bill.ID);
	if(billIt!=g_messages.end()){
		billIt->second.push_back(newMessage);
		cout<<"Bill now has a message: "<<g_messages.find("Bill")->second.at(0).content<<endl;
	}else{
		cout<<"Bill not found!"<<endl;
	}
	g_message_mutex.unlock();
	
	return 0;
}