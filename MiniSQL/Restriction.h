#pragma once
#include<vector>
#include"tableinfo.h"
using namespace std;

namespace RestrictionOP {
	const int Left = 100;
	const int Right = 101;
	const int NotEqualTo = 1;
	const int EqualTo = 2;
	const int LessThan = 3;
	const int GreaterThan = 4;
	const int LessOrEqualToThan = 5;
	const int GreaterOrEqualToThan = 6;
	string strofOP(int op);
}

struct RestricionItem {

	int isConstant;
	bool constant;

	RestricionItem();
	int op;
	string inColName;
	int inColno;
	int inLR;
	int constType;
	
	int oprd;
	float oprf;
	string oprs;

	bool result(const table &in) const;
	void showinfo() const;
};

class Restriction
{
public:
	vector<RestricionItem> List;
	bool result(const table &in) const;
};

