#pragma once
#include<string>
#include<vector>
#include<vector>
using namespace std;

struct datablock {
	datablock();
	vector<int> d;
	vector<float> f;
	int len;
	vector<string> s;
	void clear();
};

class table {
public:
	string name;
	vector<string> keyname;
	vector<string> keytype;
	vector<datablock> keydata;
	vector<string> prikeyname;
	vector<int> unique;
	void clear();
	void showinfo() const;
	bool equalType(const table &b);

	void copyTableHead(table &dst);
	void addTableHead(const table &head);
	void copyIthRecord(int ith, table &dst) const;
	void addOneRecord(const table &src);
	int columnOfName(const string &name) const;
	int recordNumber() const;

	void printTableContent() const;
};