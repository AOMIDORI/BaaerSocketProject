#include "stdafx.h"
using namespace std;

extern int allMessageIds;


	//for writing and reading to permanent storage
	// Insertion operator
	std::ostream& operator<<(std::ostream& os, const User& s)
	{
		// write out individual members of s with an end of line between each one
		os << s.name << '\n';
		os << s.status << '\n';
		int sizeFollowing = s.following.size();
		os << sizeFollowing;
		for(int i=0; i<sizeFollowing; i++){
			os << s.following[i].name;
		}
		return os;
	}

	// Extraction operator
	std::istream& operator>>(std::istream& is, User& s)
	{
		// read in individual members of s
		int sizeFollowing;
		vector<User> followingVector;
		is >> s.name >> s.status >> sizeFollowing;
		for (int i=0; i<sizeFollowing; i++){
			string nameFollowing;
			is >> nameFollowing;
			//find the user in g_users and add to following<Users> 

		}
		s.following=followingVector;
		return is;
	}