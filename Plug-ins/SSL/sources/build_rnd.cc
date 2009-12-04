#include <iostream>
#include <fstream>

const char* tohex = "0123456789ABCDEF";

main()
{
	ifstream fin(".rnd");

	cout << "\tconst unsigned char rsrc[1024] = {" << endl << "\t";
	for(int i = 0; i < 1024; i++)
	{
		unsigned char c;
		fin >> c;
		if (i)
			cout << ", ";
		if (i && !(i % 16))
			cout << endl << "\t";
		cout << "0x" << tohex[(c & 0xF0) >> 4] << tohex[c & 0x0F];
	}
	cout << endl << "\t};" << endl;
}
