using namespace std;

class Message {

  public:	
	int id;
	string content;
	double timeStamp;
	string user;

    Message(string text, string user);

	//for writing and reading to permanent storage
	// Insertion operator
	friend std::ostream& operator<<(std::ostream& os, const Message& s);

	// Extraction operator
	friend std::istream& operator>>(std::istream& is, Message& s);


};

class User {
public: 
	char *name;
	int status; // 0: offline,  1: online
	//vector<Message> messagelist;
	vector<User> following;


	//for writing and reading to permanent storage
	// Insertion operator
	friend std::ostream& operator<<(std::ostream& os, const User& s);
	// Extraction operator
	friend std::istream& operator>>(std::istream& is, User& s);

};
