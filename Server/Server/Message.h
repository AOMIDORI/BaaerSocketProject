using namespace std;

class Message {

  public:	
	int id;
	string content;
	double timeStamp;
	string user;

	Message(string text);
    Message(string text, string user);

};

struct User {
	char *name;
	int status; // 0: offline,  1: online
	vector<Message> messagelist;
	vector<User> following;
};
