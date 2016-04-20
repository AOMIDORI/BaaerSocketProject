#include <iostream>
#include <string>
#include <vector>
#include <cstring>
 
using namespace std;

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

