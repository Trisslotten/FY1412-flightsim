#pragma once
#include <string>
#include <unordered_map>
#include <vector>

struct wingData
{
	double ang;
	double cl;
	double cd;
};

class LookUpTable
{
private:
	std::string raw;
	double start_ang[5], end_ang[5];
	std::vector<wingData> tables[5];
	wingData interpolatedAngleData(double ang, int array1, int array2, double q);
	//std::unordered_map<double, wingData> tables[5]; //tables for Re 50k, 100k, 200k, 500k, 1000k

public:
	LookUpTable(std::string filepath[5]);
	wingData lookUp(double ang, double Re);
	double minAngle(double Re);
	double maxAngle(double Re);
};