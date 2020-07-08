#include "pch.h"
#include "API.h"
#include "SQLBlock.h"
#include <iostream>
#include "interpreter.h"
#include "BufferManager.h"
#include "CatalogManager.h"
#include "RecordManager.h"
#include <Windows.h>
#include "IndexManager.h"
#include "ValueInBytes.h"
#include "BPTreeBasics.h"
#include <random>
#include <cstdlib>
#include <algorithm>
#include <ctime>
#include <cassert>
using namespace std;

struct pack {
	int k, p, rid;
};
bool operator<(pack a, pack b)
{
	return  a.rid < b.rid;
}

int main()
{
	interpreter minisql;	
	/*BufferManager buf("abc",0,4);
		int p;
		buf.AllocFromReadFile("DBFile//person_ca.catalog", IndexBlock, "Ab", p);

		SQLCatalogBlock *pt = (SQLCatalogBlock*)buf.BlockPtr(p);
		table std;
		pt->ShowContent(cout,0,4096,'d');
		std.showinfo();*/

		minisql.minisql_run();
}

/*int main()
{
	int NN = 100;
	WinExec("debugfiledeleter.exe", NULL);
	interpreter minisql;

	size_t seed = time(NULL);
	srand(seed);

	cout << "Seed: " << seed << endl;

	IndexManager indexm;
	BufferManager buf(1, 50);
	
	vector<pack> vec;
	vec.push_back(pack());
	vec[0].k = 1;
	vec[0].p = 1;
	vec[0].rid = 500;

	int i;
	ifstream fin("test.txt");
	for (int i = 1; i < NN; ++i)
	{
		vec.push_back(pack());
		vec[i].k = rand() % 10 + vec[i - 1].k + 1;
		vec[i].p = rand() % 200;
		vec[i].rid = rand() % 1000;
	}
	sort(vec.begin(), vec.end());

	for (i = 0; i < NN; ++i)
	{
		cout << vec[i].k << "--->" << vec[i].p << endl;
	}
	
	minisql.run();
	indexm.CreateNewIndex("testindex", minisql.results, "a1");

	indexm.AppointIndex("abc1", "a1");
	for (i = 0; i < NN; ++i)
	{
		//cout << "insert " << vec[i].k << endl;
		indexm.InsertIntoIndex(vec[i].k, 0, string(), vec[i].p);
		//cout << "----------------------------------" << endl;
 	}

	cout << "*******************************" << endl << endl << endl;
	
	for (int i = 0; i <= 50; ++i)
	{
		cout << "i:" << i << endl;
		int p;
		int xxx = buf.AllocFromReadFile(string("abc1_a1_i") + to_string(i) + ".index", IndexBlock, "1", p);
		if (xxx != 0)continue;
		SQLIndexBlock *pt = (SQLIndexBlock*)buf.BlockPtr(p);
		BPTreeNode x;
		pt->ExportToBPTreeNode(x);
		x.printInfo();
		cout << "\n------------------------------\n";
	}
	

	cout << "*******************************" << endl << endl << endl;
	indexm.AppointIndex("abc1", "a1");
	int l = 10, r = 40;
	for (int i = l; i <= r; ++i)
	{
		cout << "-" << i << '-' << "delete " << vec[i].k << endl;
		indexm.DeleteFromIndex(vec[i].k, 0, string());
	}
	for (int i = 0; i <= 50; ++i)
	{
		cout << "i:" << i << endl;
		int p;
		int xxx = buf.AllocFromReadFile(string("abc1_a1_i") + to_string(i) + ".index", IndexBlock, "1", p);
		if (xxx != 0)continue;
		SQLIndexBlock *pt = (SQLIndexBlock*)buf.BlockPtr(p);
		BPTreeNode x;
		pt->ExportToBPTreeNode(x);
		x.printInfo();
		cout << "\n------------------------------\n";
	}
	for (int i = 0; i < NN; ++i)
	{
		cout << "[" << i << "," << vec[i].k << "]" << "---|";
		int w = indexm.SearchKey(vec[i].k, 0, string());
		cout << w << endl;
		if (l <= i && i <= r)
			assert(-1 == w);
		else
			assert(vec[i].p == w);
		//cout << vec[i].k << '[' << vec[i].p << ']' << "----->" << indexm.SearchKey(vec[i].k, 0, string()) << endl;
	}

	return 0;
}*/

/*
199--->130
54--->61
87--->21
70--->190
195--->136
159--->195
124--->119
167--->32
17--->154
170--->72
181--->101
134--->107
78--->115
37--->19
157--->38
138--->51
139--->194
47--->161
81--->43
191--->14
*/