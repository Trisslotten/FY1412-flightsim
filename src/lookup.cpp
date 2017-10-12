#include "lookup.hpp"
#include <fstream>
#include <streambuf>
#include <iostream>
#include <sstream>
#include <math.h>
#include <algorithm>
#include <glm/glm.hpp>

double lerp(double v0, double v1, double t)
{
	return (1.0 - t) * v0 + t * v1;
}

wingData LookUpTable::interpolatedAngleData(double ang, int array1, int array2, double q)
{
	wingData data{0,0,0};
	
	if (ang >= end_ang[array1] || ang >= end_ang[array2] || ang <= start_ang[array1] || ang <= start_ang[array2])
		return data;
	
	int next1 = 1;
	// could probably do binary search here meh
	for (int i = 1; i < tables[array1].size(); i++)
	{
		next1 = i;
		if (tables[array1][i].ang > ang)
		{
			break;
		}
	}
	double next = tables[array1][next1].ang;
	double prev = tables[array1][next1 - 1].ang;
	double t1 = (ang - prev) / (next - prev);


	int next2 = 1;
	for (int i = 1; i < tables[array2].size(); i++)
	{
		next2 = i;
		if (tables[array2][i].ang > ang)
		{
			break;
		}
	}
	next = tables[array2][next2].ang;
	prev = tables[array2][next2 - 1].ang;
	double t2 = (ang - prev) / (next - prev);


	//11 = table 1 angle 1, 12 = table 1 angle 2 ... etc.
	int index11 = next1-1;
	int index12 = next1;

	int index21 = next2-1;
	int index22 = next2;


	wingData data11, data12, data21, data22;
	data11 = tables[array1][index11];
	data12 = data11;
	if (index12 < tables[array1].size())
		data12 = tables[array1][index12];
		

	data21 = tables[array2][index21];
	data22 = data21;
	if(index22 < tables[array2].size())
		data22 = tables[array2][index22];

	//lerp cl and cd from both angles for both tables
	double cl1 = lerp(data11.cl, data12.cl, t1);
	double cd1 = lerp(data11.cd, data12.cd, t1);

	double cl2 = lerp(data21.cl, data22.cl, t2);
	double cd2 = lerp(data21.cd, data22.cd, t2);

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
			temp.ang = ang;
			if (first)
			{
				firstAng = ang; 
				first = false;
			}
				
			//std::cout << "angle: " << ang << ", at index [" << it <<"],  cl: " << temp.cl << ", cd: " << temp.cd << std::endl;

			tables[i].emplace_back(temp);
			it++;
		}
		start_ang[i] = firstAng; 
		end_ang[i] = ang;

		

		t.close();
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
		data = interpolatedAngleData(ang, 0, 0, 0);
	}
	else if (Re < 100000)
	{
		double q = (Re - 50000) / (100000 - 50000);
		data = interpolatedAngleData(ang, 0, 1, q);
	}
	else if (Re < 200000)
	{
		double q = (Re - 100000) / (200000 - 100000);
		data = interpolatedAngleData(ang, 1, 2, q);
	}
	else if (Re < 500000)
	{
		double q = (Re - 200000) / (500000 - 200000);
		data = interpolatedAngleData(ang, 2, 3, q);
	}
	else if (Re < 1000000)
	{
		double q = (Re - 500000) / (1000000 - 500000);
		data = interpolatedAngleData(ang, 3, 4, q);
	}
	else
	{
		data = interpolatedAngleData(ang, 4, 4, 0);
	}
	//std::cout << "angle: " << ang << ", angtranc: " << angtrunc << ", Re: " << Re << ", cl: " << data.cl << ", cd: " << data.cd << std::endl;
	return data;
}

double LookUpTable::minAngle(double Re)
{
	if (Re < 50000) // Need a way to approximate Cl and Cd between Re ranges and angle intervals. Currently a rough approx of Re ranges and no approx of angle intervals.
	{
		return start_ang[0];
	}
	else if (Re < 100000)
	{
		return glm::max(start_ang[0], start_ang[1]);
	}
	else if (Re < 200000)
	{
		return glm::max(start_ang[1], start_ang[2]);
	}
	else if (Re < 500000)
	{
		return glm::max(start_ang[2], start_ang[3]);
	}
	else if (Re < 1000000)
	{
		return glm::max(start_ang[3], start_ang[4]);
	}
	else
	{
		return start_ang[4];
	}
}

double LookUpTable::maxAngle(double Re)
{
	if (Re < 50000) // Need a way to approximate Cl and Cd between Re ranges and angle intervals. Currently a rough approx of Re ranges and no approx of angle intervals.
	{
		return end_ang[0];
	}
	else if (Re < 100000)
	{
		return glm::min(end_ang[0], end_ang[1]);
	}
	else if (Re < 200000)
	{
		return glm::min(end_ang[1], end_ang[2]);
	}
	else if (Re < 500000)
	{
		return glm::min(end_ang[2], end_ang[3]);
	}
	else if (Re < 1000000)
	{
		return glm::min(end_ang[3], end_ang[4]);
	}
	else
	{
		return end_ang[4];
	}
}
