#pragma once
#include "tableinfo.h"
#include <iostream>
#include "BPTreeBasics.h"

typedef unsigned char BYTE;
#define BLOCK_LENGTH 4096
#define BLOCK_DATA_BEGIN 200
#define BLOCK_ID_BEGIN 160
#define BLOCK_INDEX_RIGHT_PTR 400
#define BLOCK_INDEX_REDIR_BEGIN 500
#define BLOCK_INDEX_KEYN_AT 1000
#define BLOCK_INDEX_KEY_BEGIN 1024

#define TABLE_TYPE_ERROR 111111
#define BLOCK_FULL 222222
#define BLOCK_SUCCESSFUL 0
#define NOT_FOUND 333333
#define BLOCK_FAILED -1000


#define CatalogBlock 1
#define DataBlock 2
#define DataHeadBlock 3
#define IndexBlock 4
#define DirectEditBlock 5

class SQLBlock
{
	friend class BufferManager;
protected:
	BYTE bytes[BLOCK_LENGTH];
	int edited;
public:
	SQLBlock();
	int Edited();
	string TableName();
	int SetBlockHead(const table &src);
	void GetFromBlockHead(table &ret);
	void SetBlockID(const string &id);
	string GetBlockID();
	void ShowContent(ostream &out, int l = 0, int r = BLOCK_LENGTH, char otp = 'd');
};

class SQLCatalogBlock : public SQLBlock {

public:
	SQLCatalogBlock();
	void GetTableInfo(table &ret);
	int SetFromTable(const table &src);
	int Indexed(int no);
	void setIndexStatus(int no, int status);

};

class SQLDataBlock : public SQLBlock {
public:
	SQLDataBlock();
	void GetData(table &ret);
	void GetTableInfo(table &ret);
	int InsertData(const table &rec);
	int DeleteData(int no);
	int recnum();
};

class SQLDataHeadBlock : public SQLBlock {

public:
	SQLDataHeadBlock();
	int DataFileNumber();
	void IncreaseFileNumber();
	void DecreaseFileNumber();

};

class SQLIndexBlock : public SQLBlock {

public:
	int SetBlockHead(const table &src) = delete;
	void GetFromBlockHead(table &ret) = delete;
	void SetFromBPTreeNode(const BPTreeNode &src);
	void ExportToBPTreeNode(BPTreeNode &dst);
};

class SQLDirectEditBlock : public SQLBlock {
public:
	BYTE *beg = bytes;
};