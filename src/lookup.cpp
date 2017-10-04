#include "lookup.hpp"
#include <fstream>
#include <streambuf>
#include <iostream>
#include <sstream>
LookUpTable::LookUpTable(std::string filepath[5])
{
	for (int i = 0; i < 5; i++)
	{
		std::ifstream t("assets/tables/" + filepath[i]);
		raw = std::string((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());

		std::stringstream stream(raw);
		for (std::string line; std::getline(stream, line); )
		{
			std::stringstream iss(line);

			wingData temp;
			double ang;
			iss >> ang >> temp.cl >> temp.cd;

			//std::cout << "angle: " << ang << ", cl: " << temp.cl << ", cd: " << temp.cd << std::endl;

			tables[i][ang] = temp;
		}
	}
}

wingData LookUpTable::lookUp(double ang, double Re)
{
	double remainder = (double)((int)(ang * 10000) % 2500) / 10000;
	double remaindern = remainder * 4;
	double angtrunc = ang - remainder; //remove remainder from 0.25 multiple to get a compatible entry for the tables.
	double rratio = 1 - remaindern;

	wingData data;
	data.cd = -2; data.cl = -2;
	/*if (Re < 50000)
	{
		double q = Re / 50000;
		data = tables[0][angtrunc];
		//data.cd * q; data.cl * q;    //I want to do something like this but idk if it will work linear like that.
	}*/
	if (Re < 75000) // Need a way to approximate Cl and Cd between Re ranges and angle intervals. Currently a rough approx of Re ranges and no approx of angle intervals.
	{
		double q = Re / 50000;
		if (tables[0].find(angtrunc) != tables[0].end())
			data = tables[0][angtrunc];
	}
	else if (Re > 75000 && Re < 150000)
	{
		if (tables[1].find(angtrunc) != tables[1].end())
			data = tables[1][angtrunc];
	}
	else if (Re > 150000 && Re < 350000)
	{
		if (tables[2].find(angtrunc) != tables[2].end())
			data = tables[2][angtrunc];
	}
	else if (Re > 350000 && Re < 750000)
	{
		if (tables[3].find(angtrunc) != tables[3].end())
			data = tables[3][angtrunc];
	}
	else
	{
		if (tables[4].find(angtrunc) != tables[4].end())
			data = tables[4][angtrunc];
	}
	std::cout << "angle: " << ang << ", angtranc: " << angtrunc << ", Re: " << Re << ", cl: " << data.cl << ", cd: " << data.cd << std::endl;
	return data;
}