#pragma once
#include<cstring>
#include"BufferManager.h"
#include"tableinfo.h"
#include<fstream>
#include<set>
#include"Restriction.h"
#define RECORD_NOT_FOUND 23333
#define RECORD_SUCCESSFUL 0
#define RECORD_ALREADY_EXIST 233333



class RecordManager
{
private:
	BufferManager buffer;
	string RecordFileName(const string &tablename, int no);
	string RecordHeadFileName(const string &tablename);
	int curPos;
	SQLDataHeadBlock *curPt;
	string curTableName;
	table curTableInfo;

	int curDataPos;
	SQLDataBlock *curDataPt;

	int JustInserted;

	int loaded_in_set;
	set<int> intset[40];
	set<string> charset[40];
	set<float> floatset[40];

	table result;
	table delete_record;
	int insert_position;
	
	int delete_number;

public:
	RecordManager();
	~RecordManager();

	void CloseDataFile();

	int CreateNewRecord(const table &tablename);
	int DropAllRecord(const string &tablename);
	
	int AppointTable(const string &tablename);

	int AlreadyExist(int colno, const string &dataType, datablock data);
	int GetSelectResult(const Restriction &limits, int target_file = -1);
	int InsertRecord(const table &src);
	int DeleteRecord(const Restriction &limits, int target_file = -1);

	int DataFileNumber();

	const table &res = result;
	const int &insert_pos = insert_position;
	const table &delete_rec = delete_record;
	const int &delete_num = delete_number;
};

