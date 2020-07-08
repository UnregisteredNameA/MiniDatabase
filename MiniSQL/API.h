#pragma once
#include<fstream>
#include<vector>
#include<string>
#include"tableinfo.h"
#include"CatalogManager.h"
#include"RecordManager.h"
#include"Restriction.h"
#include"IndexManager.h"
using namespace std;
#define IDLE 500
#define IN_STATEMENT 501
#define IN_STRING 502
#define CREATE_TABLE 100
#define DROP_TABLE 101
#define CREATE_INDEX 102
#define DROP_INDEX 103
#define SELECT 104
#define INSERT 105
#define DELETE 106
#define QUIT 107
#define EXECFILE 108
#define INVALID 1000
#define SUCCESSFUL 2000
#define FAILED 2001

class SQLError {
private:
	string info_str;
public:
	SQLError(const string &info);
	string err_info();
};

class SQLAPI
{
	friend class interpreter;
private:
	table res;
	RecordManager rec_manager;
	CatalogManager cat_manager;
	IndexManager ind_manager;

	string sql, rawsql;
	string error_info;
	int status = IDLE;

public:

	void createTable(const table &src);
	void dropTable(const string &tablename);

	void createIndex(const string &indexname, const table &src, const string &colname);
	void dropIndex(const string &indexname);

	void selectSQL(const string& tablename, const Restriction &lim);
	void insertSQL(const table &src, int instant_save);
	void deleteSQL(const string &tablename, const Restriction &lim);
	void insertionEnd();

	const table &results = res;

	int delete_num;
};

