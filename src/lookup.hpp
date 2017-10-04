#pragma once
#include <string>
#include <unordered_map>

struct wingData
{
	double cl;
	double cd;
};

enum ReLimits
{
	RE_SUB50 = 0,
	RE_50100,
	RE_100200,
	RE_200500,
	RE_5001000
};

class LookUpTable
{
private:
	std::string raw;
	double start_ang, end_ang;
	std::unordered_map<double, wingData> tables[5]; //tables for Re 50k, 100k, 200k, 500k, 1000k

public:
	LookUpTable(std::string filepath[5]);
	wingData lookUp(double ang, double Re);
};