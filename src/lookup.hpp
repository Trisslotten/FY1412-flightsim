#pragma once
#include <string>
#include <unordered_map>

struct wingData
{
	double cl;
	double cd;
};

class LookUpTable
{
private:
	std::string raw;
	double start_ang, end_ang;
	std::unordered_map<double, wingData> table;
public:
	LookUpTable(std::string filepath);
	inline wingData lookUp(double ang)
	{
		return table[ang];
	}
};