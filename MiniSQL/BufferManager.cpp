#include "pch.h"
#include "BufferManager.h"
#include <cstring>
#include <cstdlib>

BufferManager::BufferManager(const string &bufid, int throwTp, int maxn)
{
	BufferManagerIdentifier = bufid;
	memset(locked, 0, sizeof(locked));
	MAXN = maxn;
	for (int i = 0; i < MAXN; ++i) blocks[i] = NULL;
	throwType = throwTp;
	if(throwTp == 1 || throwTp == 0) for (int i = 0; i < MAXN; ++i) L.push_back(i);
}

BufferManager::~BufferManager()
{
	int i, j;
	string str;
	for (i = 0; i < MAXN; ++i)
	{
		if (blocks[i] != nullptr)
		{
			if (blocks[i]->Edited())
			{
				string bid = blocks[i]->GetBlockID();
				WriteBlock(bid, i);
			}
		}
		delete blocks[i];
	}
	fin.close();
	fout.close();
}

int BufferManager::Full()
{
	if (throwType == THROW_EARLIEST_ACCESSED) return QueList.size() + LockedList.size() == MAXN;
	if (throwType == THROW_LATEST_ACCESSED) return StaList.size() + LockedList.size() == MAXN;
}

int BufferManager::ThrowOldBlock()
{
	if (throwType == THROW_EARLIEST_ACCESSED)
	{
		if (QueList.empty()) return EMPTY_BUFFER;
		int tid = QueList.front();
		QueList.pop();
		while (locked[tid])
		{
			LockedList.insert(tid);
			if (QueList.empty()) return ALL_LOCKED;
			tid = QueList.front();
			QueList.pop();
		}
		L.push_back(tid);
		string tblockid = blocks[tid]->GetBlockID();
		BufferPos.erase(tblockid);
		delete blocks[tid];
#ifdef BUFFER_MONITOR
		cout << "\n---------------\n";
		cout << "Buffer ID is : [" << this->BufferManagerIdentifier << "]\n";
		cout << "### Block thrown from [" << tid << "] ###\n---------------" << endl;
#endif
	}
	if (throwType == THROW_LATEST_ACCESSED)
	{
		if (StaList.empty()) return EMPTY_BUFFER;
		int tid = StaList.top();
		StaList.pop();
		while (locked[tid])
		{
			LockedList.insert(tid);
			if (StaList.empty()) return ALL_LOCKED;
			tid = StaList.top();
			StaList.pop();
		}
		L.push_back(tid);
		string tblockid = blocks[tid]->GetBlockID();
		BufferPos.erase(tblockid);
		delete blocks[tid];
#ifdef BUFFER_MONITOR
		cout << "\n---------------\n";
		cout << "Buffer ID is : [" << this->BufferManagerIdentifier << "]\n";
		cout << "### Block thrown from [" << tid << "] ###\n---------------" << endl;
#endif
	}

	return BUFFER_OPERATE_SUCCESS;
}

int BufferManager::GetNewBlockID()
{
	int rid = L.front();
	if (throwType == 0)
	{
		QueList.push(rid);
		L.pop_front();
	}
	if (throwType == 1)
	{
		StaList.push(rid);
		L.pop_front();
	}
	return rid;
}

int BufferManager::AllocFromReadFile(const string &fname, int blockType, const string &blockID, int &alloc)
{
	if (Full())
	{
		int sig = ThrowOldBlock();
		if (sig != BUFFER_OPERATE_SUCCESS) return BUFFER_OPERATE_FAILED;
	}
	int dst = GetNewBlockID();
	switch (blockType)
	{
	case CatalogBlock:
		blocks[dst] = new SQLCatalogBlock;
		break;
	case DataBlock:
		blocks[dst] = new SQLDataBlock;
		break;
	case DataHeadBlock:
		blocks[dst] = new SQLDataHeadBlock;
		break;
	case IndexBlock:
		blocks[dst] = new SQLIndexBlock;
		break;
	case DirectEditBlock:
		blocks[dst] = new SQLDirectEditBlock;
		break;
	}

	fin.open(fname, ios::binary);
	if (!fin.good()) return FILE_NOT_FOUND;
	fin.read((char*)(blocks[dst]->bytes), BLOCK_LENGTH);
	fin.close();
	alloc = dst;
	BufferPos[blockID] = dst;
#ifdef BUFFER_MONITOR
	cout << "\n---------------\n";
	cout << "Buffer ID is : [" << this->BufferManagerIdentifier << "]\n";
	cout << "### Allocate from reading file: allocate to [" << dst << "] ###\n---------------" << endl;
#endif

	return BUFFER_OPERATE_SUCCESS;
}

int BufferManager::AllocFromBlock(const SQLBlock * src, int blockType, const string &blockID, int & alloc)
{
	if (Full())
	{
		int sig = ThrowOldBlock();
		if (sig != BUFFER_OPERATE_SUCCESS) return BUFFER_OPERATE_FAILED;
	}
	int dst = GetNewBlockID();
	switch (blockType)
	{
	case CatalogBlock:
		blocks[dst] = new SQLCatalogBlock;
		break;
	case DataBlock:
		blocks[dst] = new SQLDataBlock;
		break;
	case DataHeadBlock:
		blocks[dst] = new SQLDataHeadBlock;
		break;
	case IndexBlock:
		blocks[dst] = new SQLIndexBlock;
		break;
	case DirectEditBlock:
		blocks[dst] = new SQLDirectEditBlock;
		break;
	}	
	memcpy_s(blocks[dst]->bytes, BLOCK_LENGTH, src->bytes, BLOCK_LENGTH);
	alloc = dst;
	BufferPos[blockID] = dst;
#ifdef BUFFER_MONITOR
	cout << "\n---------------\n";
	cout << "Buffer ID is : [" << this->BufferManagerIdentifier << "]\n";
	cout << "### Allocate from block: allocate to [" << dst << "] ###\n---------------" << endl;
#endif
	return BUFFER_OPERATE_SUCCESS;
}

int BufferManager::WriteBlock(const string & fname, int srcno)
{
	blocks[srcno]->edited = 0;
	fout.open(fname, ios::binary);
	fout.write((char*)(blocks[srcno]->bytes), BLOCK_LENGTH);
	fout.close();
#ifdef BUFFER_MONITOR
	cout << "\n---------------\n";
	cout << "Buffer ID is : [" << this->BufferManagerIdentifier << "]\n";
	cout << "### Write block: write block at [" << srcno << "] ###\n---------------" << endl;
#endif
	return BUFFER_OPERATE_SUCCESS;
}

int BufferManager::DeleteFile(const string & fname)
{
	return remove(fname.c_str());
}

void BufferManager::SetLock(int no)
{
	locked[no] = 1;
}

void BufferManager::SetUnlock(int no)
{
	locked[no] = 0;
	if (LockedList.find(no) != LockedList.end())
	{
		if (throwType == THROW_EARLIEST_ACCESSED)
		{
			QueList.push(no);
		}
		if (throwType == THROW_LATEST_ACCESSED)
		{
			StaList.push(no);
		}
	}
}

SQLBlock* BufferManager::BlockPtr(int no)
{
	return blocks[no];
}

bool BufferManager::FileExists(const string & filename)
{
	fin.close();
	fin.open(filename);
	bool r = fin.good();
	fin.close();
	return r;
}

int BufferManager::InBufferPos(const string & blockID)
{
	if (BufferPos.find(blockID) == BufferPos.end()) return -1;
	return BufferPos[blockID];
}

void BufferManager::SignOutWithID(const string & blockID)
{
	BufferPos.erase(blockID);
}