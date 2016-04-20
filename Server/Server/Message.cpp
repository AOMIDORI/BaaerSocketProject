#include "stdafx.h"
using namespace std;

extern int allMessageIds;

Message::Message(string text, string thisUser){
		id=allMessageIds++;
		content = text;
		timeStamp= GetCurrentTime();
		user = thisUser;
		return;
}

Message::Message(string text){
		id=allMessageIds++;
		content = text;
		timeStamp= GetCurrentTime();
		user = "default";
		return;
}