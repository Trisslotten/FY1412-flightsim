#include "lookup.hpp"
#include <fstream>
#include <streambuf>
#include <iostream>
#include <sstream>
LookUpTable::LookUpTable(std::string filepath)
{
	std::ifstream t(filepath);
	raw = std::string((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());
	
	std::stringstream stream(raw);
	for (std::string line; std::getline(stream, line); )
	{

		std::stringstream iss(line);
		
		wingData temp;
		double ang;
		iss >> ang >> temp.cl >> temp.cd;

		std::cout << "angle: " << ang << ", cl: " << temp.cl << ", cd: " << temp.cd <<  std::endl;

		table[ang] = temp;
	}
	
}