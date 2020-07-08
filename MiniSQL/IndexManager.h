#pragma once
#include<string>
#include<vector>
#include"BufferManager.h"
#include"tableinfo.h"
using namespace std;
namespace index {
	constexpr int INDEX_NOT_FOUND = 1733;
	constexpr int INDEX_ALREADY_EXIST = 2456;

	const int NULLi = 0;
	const float NULLf = 0;
	const string NULLc = "";
}
#define BPTREE_MAX_DEGREE 100
#define BPTREE_MIN_DEGREE 4

class IndexManager
{
private:
	BufferManager BUFFER;
	string curTableName;
	string curColName;
	int curPos;
	SQLIndexBlock *curPt;
	BPTreeNode curNode;
	string IndexFileName(const string &tablename, const string &colname, const string &id);

	int fatherPos;
	SQLIndexBlock *fatherPt;
	BPTreeNode fatherNode;
	int siblingPos;
	SQLIndexBlock *siblingPt;
	BPTreeNode siblingNode;
	int cur_id, father_id, sibling_id;

	BufferManager MAP_BUFFER;
	string IndexMapFileName(int ith);

	int allocPos;
	SQLDirectEditBlock* allocPtr;
	BufferManager ID_ALLOC_BUFFER;
	string IndexAllocFileName(const string &tablename, const string &colname);
	void save_newnode_to_file(const BPTreeNode &src, int id);
	void import_into_current(int id);
	void import_into_father(int id);
	void import_into_sibling(int id);
	void save_cur_father_sibling(int curen = 1, int fatheren = 1, int siblingen = 1);

	void real_drop_index(const string &tablename, const string &colname);
	string dropped_tablename, dropped_colname;

public:
	IndexManager();
	~IndexManager();

	bool IndexExistsNamed(const string &indexname);
	bool IndexExistsOn(const string &tablename, const string &colname);
	void CreateNewIndex(const string &indexname, const table &src, const string &colname);

	void AppointIndex(const string &tablename, const string &colname);

	void InsertIntoIndex(int isrc, float fsrc, const string &csrc, int redir);
	int SearchKey(int itarget, float ftarget, const string &ctarget);
	void DeleteFromIndex(int ival, float fval, const string &cval);

	void DropIndex(const string &indexname);
	void DropIndex(const string &tablename, const string &colname);

	const string &IndexDroppedTable = dropped_tablename;
	const string &IndexDroppedColumn = dropped_colname;
};

