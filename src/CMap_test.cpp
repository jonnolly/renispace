


#include "CMap.h"
#include<iostream>
#include<fstream>


int main()
{
	std::ofstream myfile;
	myfile.open("example.txt");
	if(myfile.is_open())
	{ 
	myfile << "Writing this to a file.\n";
	}
	else 
	{
		std::cout << "Unable to open files \n";
	}

	myfile.close();

	std::string filepath = "5x5testmap1.txt";
	CMap::CMap(filepath);

	system("pause");
}
