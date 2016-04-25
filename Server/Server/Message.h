using namespace std;

class Message {

  public:	
	int id;
	string content;
	double timeStamp;
	int user;

    Message(string text, int user);

	//for writing and reading to permanent storage
	// Insertion operator
	friend std::ostream& operator<<(std::ostream& os, const Message& s);

	// Extraction operator
	friend std::istream& operator>>(std::istream& is, Message& s);


};

class User {
public: 
	int ID;
	char *name;
	int status; // 0: offline,  1: online
	//vector<Message> messagelist;
	vector<int> following;

	//for writing and reading to permanent storage
	// Insertion operator
	friend std::ostream& operator<<(std::ostream& os, const User& s);
	// Extraction operator
	friend std::istream& operator>>(std::istream& is, User& s);

	User();

	User getUser(int id);
};
