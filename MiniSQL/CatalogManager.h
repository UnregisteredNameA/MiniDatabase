#pragma once
#include<cstring>
#include"BufferManager.h"
#include"tableinfo.h"
#include<fstream>
#include<set>
using namespace std;
#define CATALOG_NOT_FOUND 555577
#define CATALOG_SUCCESSFUL 0
#define CATALOG_ALREADY_EXIST 555666

class CatalogManager
{
private:
	BufferManager buf;
	string CatalogFileName(const string &tablename);
	int curTablePt;
	SQLCatalogBlock *curPt;
	string curTableName;
	table curTableInfo;

public:
	int CreateCatalog(const table &src);
	bool CatalogExists(const string &tablename);
	int RemoveCatalog(const string &tablename);

	int AppointTable(const string &tablename);
	void Close();
	int StandardTableForm(table &std);
	int Unique(const string &col);
	int Unique(int no);
	int Indexed(const string &col);
	int Indexed(int no);
	void SetIndexStatus(int colno, int indexed);
	string PrimaryKeyName();
	int PrimaryKeyNo();

	CatalogManager();
	~CatalogManager();
};

