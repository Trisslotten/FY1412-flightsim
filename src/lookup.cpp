#include "lookup.hpp"
#include <fstream>
#include <streambuf>
#include <iostream>
#include <sstream>
#include <math.h>
#include <algorithm>

float lerp(float v0, float v1, float t) 
{
	return (1 - t) * v0 + t * v1;
}

wingData LookUpTable::interpolatedAngleData(double ang, int array1, int array2, double q)
{
	wingData data;
	double angmod, rest;
	rest = fmod(ang, 0.25);
	angmod = ang - rest;
	double angnext = std::max(angmod + 0.25,end_ang[array2]);

	std::cout << "angle: " << ang << " mod 0.25 = " << angmod << "next = " << angnext << std::endl;

	//angle increments for table1
	int angIt11 = (angmod - start_ang[array1]) * 4;
	int angIt12 = (angnext - start_ang[array1]) * 4;

	//angle increments for table2
	int angIt21 = (angmod - start_ang[array2]) * 4;
	int angIt22 = (angnext - start_ang[array2]) * 4;

	//11 = table 1 angle 1, 12 = table 1 angle 2 ... etc.
	wingData data11, data12, data21, data22;
	data11 = tables[array1][angIt11];
	data12 = tables[array1][angIt12];

	data21 = tables[array2][angIt21];
	data22 = tables[array2][angIt22];

	//lerp cl and cd from both angles for both tables
	double cl1 = lerp(data11.cl, data12.cl, rest);
	double cd1 = lerp(data11.cd, data12.cd, rest);

	double cl2 = lerp(data21.cl, data22.cl, rest);
	double cd2 = lerp(data21.cd, data22.cd, rest);

	//lerp from both tables
	data.cl = lerp(cl1, cl2, q);
	data.cd = lerp(cd1, cd2, q);
	return data;
}

LookUpTable::LookUpTable(std::string filepath[5])
{
	for (int i = 0; i < 5; i++)
	{
		std::ifstream t("assets/tables/" + filepath[i]);
		raw = std::string((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());

		std::stringstream stream(raw);

		double firstAng = -1000000.0;
		double ang;
		int it = 0;
		bool first = true;
		for (std::string line; std::getline(stream, line); )
		{
			std::stringstream iss(line);

			wingData temp;
			iss >> ang >> temp.cl >> temp.cd;
			if (first)
				firstAng = ang; first = false;
			//std::cout << "angle: " << ang << ", at index [" << it <<"],  cl: " << temp.cl << ", cd: " << temp.cd << std::endl;

			tables[i].emplace_back(temp);
			it++;
		}
		start_ang[i] = firstAng; end_ang[i] = ang;
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

	if (Re < 50000) // Need a way to approximate Cl and Cd between Re ranges and angle intervals. Currently a rough approx of Re ranges and no approx of angle intervals.
	{
		double start = start_ang[0];
		int angIt = (angtrunc - start_ang[0]) * 4;
		data = tables[0][angIt];
	}
	else if (Re > 50000 && Re < 100000)
	{
		double q = fmod(Re,50000) / 50000;
		data = interpolatedAngleData(ang, 0, 1, q);
	}
	else if (Re > 100000 && Re < 200000)
	{
		double q = fmod(Re, 100000) / 100000;
		data = interpolatedAngleData(ang, 1, 2, q);
	}
	else if (Re > 200000 && Re < 500000)
	{
		double q = fmod(Re, 200000) / 300000;
		data = interpolatedAngleData(ang, 2, 3, q);
	}
	else if (Re > 500000 && Re < 1000000)
	{
		double q = fmod(Re, 500000) / 500000;
		data = interpolatedAngleData(ang, 3, 4, q);
	}
	else
	{
		int angIt = (angtrunc - start_ang[4]) * 4;
		data = tables[4][angIt];
	}
	std::cout << "angle: " << ang << ", angtranc: " << angtrunc << ", Re: " << Re << ", cl: " << data.cl << ", cd: " << data.cd << std::endl;
	return data;
}