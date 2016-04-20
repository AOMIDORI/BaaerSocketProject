#include "stdafx.h"

using namespace std;

int read_positive_integer()
{
  string input;
  getline(cin, input);
  stringstream my_stream(input);

  int a_number;
  if (my_stream >> a_number) return a_number;
  return -1;
}

/*
 * Reads a string from stdin, display a message before.
 * Optionally it can convert the string to lower case before returning it.
 */
string read_string(const string& message)
{
  string a_string;

  cout << message;
  getline(cin, a_string);

  return a_string;
}

vector<string> packageString(string input){
	//take a big string and divide it into packages each with length DEFAULT_BUFLEN
	vector<string> packages;
	int length=input.size();
	int nPackages = length/DEFAULT_BUFLEN +1;
	for(int i=0; i<nPackages; i++){
		string thisPackage=input.substr(i*DEFAULT_BUFLEN,DEFAULT_BUFLEN);
		packages.push_back(thisPackage);
	}
	return packages;
}


vector<string> makeTolkens(char* input, char knife){
	//take input and tolkenize by dividing at : char
	//std::cout<<"tolkenizing "<<input<<std::endl;
	vector<string> result;

	for(int i=0; i<strlen(input); i++){
		std::string thisTolken="";
		bool endTolken=false;
		while(!endTolken){ 
			char c=input[i];
			if(c!=knife && i<strlen(input)){
				thisTolken +=c;
				i++;
			}else{
				endTolken=true;
			}
		}
		result.push_back(thisTolken);
		//cout<<thisTolken;
	}
	//cout<<"done";
	return result;
}

