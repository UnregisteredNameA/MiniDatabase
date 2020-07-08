#pragma once
#include<string>
#include<algorithm>
#include<iostream>
#include<fstream>
#include"API.h"
#include"Restriction.h"
#include"CatalogManager.h" 
using namespace std;
#define SET_STRING_SIGN 2147483111

class interpreter
{
private:
	static char string_sign;
	static void set_string_sign(char sgn);

	bool info_display;
	bool instant_save;
	bool input_from_screen;

	CatalogManager CM;
	SQLAPI API;

	ifstream fin;
	istream in;
	string sql, rawsql, error_info;
	void readsql();
	void readsql_fromfile();
	int checksqltype();	

	int whereCheck(const string &whereclause);
	int setColNo();

	int createTable();
	int insertSQL();
	int selectSQL();
	int deleteSQL();
	int dropTable();
	int createIndex();
	int dropIndex();
	int execfile();

	table res, tmp;
	Restriction lim;
	int cmdType;
	string indexName, colName;
	void run();

public:
	const table &results = res;
	const Restriction &limits = lim;
	const int &SQLcmdType = cmdType;
	interpreter();
	~interpreter();
	void minisql_run();
	static char current_string_sign();

};

