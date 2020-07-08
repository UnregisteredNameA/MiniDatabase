#pragma once
#include <fstream>
#include "SQLBlock.h"
#include <queue>
#include <stack>
#include <cstdlib>
#include <list>
#include <set>
#include <map>
using namespace std;
#define MAX_MAXN 5000
#define EMPTY_BUFFER 777
#define BUFFER_OPERATE_SUCCESS 0
#define ALL_LOCKED 666
#define BUFFER_OPERATE_FAILED -1
#define FILE_NOT_FOUND 555
#define THROW_EARLIEST_ACCESSED 0
#define THROW_LATEST_ACCESSED 1

class BufferManager {
private:
	string BufferManagerIdentifier;

	ifstream fin;
	ofstream fout;
	SQLBlock *blocks[MAX_MAXN];
	int locked[MAX_MAXN];

	int throwType; //0: Throw the earlist accessed one  1: Throw the latest accessed one  2: Throw one randomly
	int MAXN;
	queue<int> QueList; 
	stack<int> StaList;
	set<int> LockedList;

	int Full();
	int GetNewBlockID();
	int ThrowOldBlock();

	list<int> L;
	map<string, int> BufferPos;

public:
	BufferManager(const string &bufid, int throwTp, int maxn);
	~BufferManager();
	int AllocFromReadFile(const string &filename, int blockType, const string &blockID, int &alloc);
	int AllocFromBlock(const SQLBlock *src, int blockType, const string &blockID, int &alloc);
	int WriteBlock(const string &filename, int srcno);
	int DeleteFile(const string &filename);
	void SetLock(int no);
	void SetUnlock(int no);
	void SignOutWithID(const string &blockID);
	SQLBlock *BlockPtr(int no);
	bool FileExists(const string &filename);
	int InBufferPos(const string &blockID);
};