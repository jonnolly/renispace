/*
 * CSVRow.cpp
 *
 *  Created on: 8 Feb 2016
 *      Author: Jack Buckingham
 *
 *  Credit goes to http://stackoverflow.com/users/14065/loki-astari
 *  The class has been lifted from
 *  http://stackoverflow.com/questions/1120140/how-can-i-read-and-parse-csv-files-in-c
 */

/*
 *  Example Usage:

int main()
{
    std::ifstream file("plop.csv");

    CSVRow row;
    while(file >> row)
    {
        std::cout << "4th Element(" << row[3] << ")\n";
    }
}

 */

#include "CSVRow.h"
using namespace std;

void CSVRow::readNextRow(istream& str)
{
	string line;
	getline(str, line);

	stringstream lineStream(line);
	string cell;

	m_data.clear();
	while (getline(lineStream, cell, ','))
	{
		m_data.push_back(cell);
	}
}
