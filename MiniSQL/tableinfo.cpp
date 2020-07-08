#include "pch.h"
#include "tableinfo.h"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <algorithm>
#include <cstring>
using namespace std;

datablock::datablock()
{
	len = 0;
}

void datablock::clear()
{
	d.clear();
	s.clear();
	f.clear();
	len = 0;
}

void table::clear()
{
	name.clear();
	keyname.clear();
	keytype.clear();
	keydata.clear();
	prikeyname.clear();
}

void table::showinfo() const
{
	cout << "Table name: " << name << endl;
	for (int i = 0; i < keyname.size(); ++i)
	{
		cout << "Key #" << i << ": "; 
		cout << "name:[" << keyname[i] << "], " << "type:[" << keytype[i];
		if (keytype[i] == "char") cout << "(" << keydata[i].len << ")";
		cout << "]";
		cout << (unique[i] ? ", unique;" : ";") << endl;
	}
	if (prikeyname.size())
	{
		cout << "Primary key: " << prikeyname[0] << endl;
	}
}

bool table::equalType(const table &b) //*this is the standard. check if b can be saved into *this.
{
	if (this->keytype.size() != b.keytype.size()) return 0;
	for (int i = 0; i < this->keytype.size(); ++i)
	{
		if (this->keytype[i] != b.keytype[i] && !(this->keytype[i] == "float" && b.keytype[i] == "int")) return 0; 
		if (this->keydata[i].len < b.keydata[i].len) return 0;
	}
	return 1;
}

void table::copyTableHead(table & dst)
{
	dst.clear();
	dst.keyname = this->keyname;
	dst.keytype = this->keytype;
	datablock blank;
	for (int i = 0; i < this->keyname.size(); ++i)
	{
		dst.keydata.push_back(blank);
		if (this->keytype[i] == "char") dst.keydata[i].len = this->keydata[i].len;
	}
	dst.prikeyname = this->prikeyname;
}

void table::addTableHead(const table &head)
{
	this->keyname = head.keyname;
	for (int i = 0; i < head.keyname.size(); ++i) this->keydata[i].len = head.keydata[i].len;
	this->prikeyname = head.prikeyname;
}

void table::copyIthRecord(int ith, table &dst) const
{
	for (int i = 0; i < this->keyname.size(); ++i)
	{
		dst.keydata[i].clear();
		if (this->keytype[i] == "int")
		{
			if (dst.keydata[i].d.size() == 0) dst.keydata[i].d.push_back(0);
			dst.keydata[i].d[0] = this->keydata[i].d[ith];
		}
		if (this->keytype[i] == "float")
		{
			if (dst.keydata[i].f.size() == 0) dst.keydata[i].f.push_back(0);
			dst.keydata[i].f[0] = this->keydata[i].f[ith];
		}
		if (this->keytype[i] == "char")
		{
			if (dst.keydata[i].s.size() == 0) dst.keydata[i].s.push_back(string());
			dst.keydata[i].s[0] = this->keydata[i].s[ith];
			dst.keydata[i].len = this->keydata[i].len;
		}
	}
}

void table::addOneRecord(const table &src)
{
	for(int i = 0; i < this->keyname.size(); ++i)
	{
		if (this->keytype[i] == "int")
		{
			this->keydata[i].d.push_back(src.keydata[i].d[0]);
		}
		if (this->keytype[i] == "float")
		{
			this->keydata[i].f.push_back(src.keydata[i].f[0]);
		}
		if (this->keytype[i] == "char")
		{
			this->keydata[i].s.push_back(src.keydata[i].s[0]);
		}
	}
}

int table::columnOfName(const string & name) const
{
	for (int i = 0; i < this->keyname.size(); ++i)
	{
		if (name == keyname[i]) return i;
	}
	return -1;
}

int table::recordNumber() const
{
	if (keytype.size() == 0)
	{
		return 0;
	}
	if (keytype[0] == "int")
	{
		return keydata[0].d.size();
	}
	if (keytype[0] == "float")
	{
		return keydata[0].f.size();
	}
	if (keytype[0] == "char")
	{
		return keydata[0].s.size();
	}
	return 0;
}

void table::printTableContent() const
{
	if (this->recordNumber() == 0)
	{
		cout << "Empty set." << endl;
		return;
	}
	cout << this->recordNumber() << " tuples in total." << endl;

	vector<int> space;
	for (int i = 0; i < keyname.size(); ++i)
	{
		if (keytype[i] == "int" || keytype[i] == "float")
		{
			space.push_back(max(10U, keyname[i].size() + 1));
		}
		if (keytype[i] == "char")
		{
			int maxlen = strlen(keyname[i].c_str()) + 1;
			for(int j = 0; j < keydata[i].s.size(); ++j)
			{
				int tt = strlen(keydata[i].s[j].c_str());
				if (tt + 1 > maxlen) maxlen = tt + 1;
			}
			space.push_back(max(maxlen, 10));
		}
	}

	cout << '+';
	for (int i = 0; i < keyname.size(); ++i) 
		cout << string(space[i], '-') << '+';
	cout << endl;
	cout << '|';
	for (int i = 0; i < keyname.size(); ++i)
	{
		cout << setw(space[i]) << right << keyname[i] << '|';
	}
	cout << endl;
	cout << '+';
	for (int i = 0; i < keyname.size(); ++i)
		cout << string(space[i], '-') << '+';
	cout << endl;
	int recn = max(keydata[0].d.size(), keydata[0].f.size());
	recn = max((unsigned)recn, keydata[0].s.size());
	for (int i = 0; i < recn; ++i)
	{
		cout << "|";
		for (int j = 0; j < keyname.size(); ++j)
		{
			if (keytype[j] == "int")
			{
				cout << setw(space[j]) << right << keydata[j].d[i];
			}
			if (keytype[j] == "float")
			{
				cout << setw(space[j]) << right << keydata[j].f[i];
			}
			if (keytype[j] == "char")
			{
				cout << setw(space[j]) << right << keydata[j].s[i].c_str();
			}
			cout << '|';
		}
		cout << endl;
		cout << '+';
		for (int i = 0; i < keyname.size(); ++i)
			cout << string(space[i], '-') << '+';
		cout << endl;
	}

}

