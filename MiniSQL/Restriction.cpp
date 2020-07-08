#include "pch.h"
#include "Restriction.h"
#include<iostream>

RestricionItem::RestricionItem() : op(0), inLR(RestrictionOP::Left), oprd(0), oprf(0), isConstant(0), constType(-1)
{
}

string RestrictionOP::strofOP(int op)
{
	if (op == RestrictionOP::EqualTo) return string("=");
	if (op == RestrictionOP::LessThan) return string("<");
	if (op == RestrictionOP::GreaterThan) return string(">");
	if (op == RestrictionOP::LessOrEqualToThan) return string("<=");
	if (op == RestrictionOP::GreaterOrEqualToThan) return string(">=");
	if (op == RestrictionOP::NotEqualTo) return string("<>");
}

bool RestricionItem::result(const table &in) const
{
	if (isConstant) return constant;
	//int inColno = in.columnOfName(inColName);
	bool left = (inLR == RestrictionOP::Left);
	switch (op)
	{
	case RestrictionOP::EqualTo:
		if(in.keytype[inColno] == "int") return (in.keydata[inColno].d[0] == oprd);
		if (in.keytype[inColno] == "float") return (in.keydata[inColno].f[0] == oprf);
		if (in.keytype[inColno] == "char") return (strcmp(in.keydata[inColno].s[0].c_str(), oprs.c_str()) == 0);
		break;
	case RestrictionOP::NotEqualTo:
		if (in.keytype[inColno] == "int") return (in.keydata[inColno].d[0] != oprd);
		if (in.keytype[inColno] == "float") return (in.keydata[inColno].f[0] != oprf);
		if (in.keytype[inColno] == "char") return (strcmp(in.keydata[inColno].s[0].c_str(), oprs.c_str()) != 0);
		break;
	case RestrictionOP::LessThan:
		if (in.keytype[inColno] == "int") return (left && in.keydata[inColno].d[0] < oprd) || (!left && oprd < in.keydata[inColno].d[0]);
		if (in.keytype[inColno] == "float") return (left && in.keydata[inColno].f[0] < oprf) || (!left && oprf < in.keydata[inColno].f[0]);
		if (in.keytype[inColno] == "char") return (left && strcmp(in.keydata[inColno].s[0].c_str(), oprs.c_str()) < 0) || (!left && strcmp(oprs.c_str(), in.keydata[inColno].s[0].c_str()) < 0);
		break;
	case RestrictionOP::GreaterThan:
		if (in.keytype[inColno] == "int") return (left && in.keydata[inColno].d[0] > oprd) || (!left && oprd > in.keydata[inColno].d[0]);
		if (in.keytype[inColno] == "float") return (left && in.keydata[inColno].f[0] > oprf) || (!left && oprf > in.keydata[inColno].f[0]);
		if (in.keytype[inColno] == "char") return (left && strcmp(in.keydata[inColno].s[0].c_str(), oprs.c_str()) > 0) || (!left && strcmp(oprs.c_str(), in.keydata[inColno].s[0].c_str()) > 0);
		break;
	case RestrictionOP::LessOrEqualToThan:
		if (in.keytype[inColno] == "int") return (left && in.keydata[inColno].d[0] <= oprd) || (!left && oprd <= in.keydata[inColno].d[0]);
		if (in.keytype[inColno] == "float") return (left && in.keydata[inColno].f[0] <= oprf) || (!left && oprf <= in.keydata[inColno].f[0]);
		if (in.keytype[inColno] == "char") return (left && strcmp(in.keydata[inColno].s[0].c_str(), oprs.c_str()) <= 0) || (!left && strcmp(oprs.c_str(), in.keydata[inColno].s[0].c_str()) <= 0);
		break;
	case RestrictionOP::GreaterOrEqualToThan:
		if (in.keytype[inColno] == "int") return (left && in.keydata[inColno].d[0] >= oprd) || (!left && oprd >= in.keydata[inColno].d[0]);
		if (in.keytype[inColno] == "float") return (left && in.keydata[inColno].f[0] >= oprf) || (!left && oprf >= in.keydata[inColno].f[0]);
		if (in.keytype[inColno] == "char") return (left && strcmp(in.keydata[inColno].s[0].c_str(), oprs.c_str()) >= 0) || (!left && strcmp(oprs.c_str(), in.keydata[inColno].s[0].c_str()) >= 0);
		break;
	}
}

bool Restriction::result(const table & in) const
{
	for (auto it = List.begin(); it != List.end(); ++it) 
		if (!it->result(in)) return 0;
	return 1;
}

void RestricionItem::showinfo() const
{
	cout << "oprd: [" << oprd << "], oprf: [" << oprf << "], oprs: [" << oprs << "]" << endl;
	cout << "op: [" << RestrictionOP::strofOP(op) << "]\n";
	cout << "column name: [" << inColName << "]\n";
	cout << (inLR == RestrictionOP::Left ? "Left" : "Right") << endl;
}