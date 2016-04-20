#include "stdafx.h"
using namespace std;

extern map<string,vector<Message>> g_messages;
extern vector<User> g_userlist;

int initialize(int userID){

	string currentUser=g_userlist[userID].name;
		
	User bill;
	bill.name="Bill";

	User jim; 
	jim.name="Jim";
	jim.status=1;
	jim.following.push_back(bill); //Jim is following bill. bob is the example user

	g_userlist.push_back(jim);
	g_userlist.push_back(bill);

	g_messages["Bill"]; 

	//add a message for bill
	auto billIt = g_messages.find("Bill");
	Message newMessage = Message("Bill Says Hi","Bill");
	if(billIt!=g_messages.end()){
		billIt->second.push_back(newMessage);
		cout<<"Bill now has a message: "<<g_messages.find("Bill")->second.at(0).content<<endl;
	}else{
		cout<<"Bill not found!"<<endl;
	}
	
	return 0;
}