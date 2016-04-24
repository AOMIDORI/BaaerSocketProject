#include <fstream>
#include <iostream>

//class Something
//{
//public:
//	int weight;
//	int size;
//
//	// Insertion operator
//	friend std::ostream& operator<<(std::ostream& os, const Something& s)
//	{
//		// write out individual members of s with an end of line between each one
//		os << s.weight << '\n';
//		os << s.size;
//		return os;
//	}
//
//	// Extraction operator
//	friend std::istream& operator>>(std::istream& is, Something& s)
//	{
//		// read in individual members of s
//		is >> s.weight >> s.size;
//		return is;
//	}
//};
//
//int main()
//{
//	Something s1;
//	Something s2;
//
//	s1.weight = 4;
//	s1.size = 9;
//
//	std::ofstream ofs("saved.txt");
//
//	ofs << s1; // store the object to file
//	ofs.close();
//
//	std::ifstream ifs("saved.txt");
//
//	// read the object back in
//	if(ifs >> s2)
//	{
//		// read was successful therefore s2 is valid
//		std::cout << s2 << '\n'; // print s2 to console
//	}
//
//	return 0;
//}