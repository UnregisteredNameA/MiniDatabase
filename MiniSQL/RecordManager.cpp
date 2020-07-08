#include "pch.h"
#include "RecordManager.h"


RecordManager::RecordManager() : buffer("RecordManager's buffer", THROW_EARLIEST_ACCESSED, 2000), JustInserted(0)
{
	loaded_in_set = 0;
	curPos = curDataPos = -1;
	curDataPt = nullptr;
	curPt = nullptr;
}

RecordManager::~RecordManager()
{
}

string RecordManager::RecordFileName(const string & tablename, int no)
{
	string ret = tablename;
	ret += "_d_";
	ret += to_string(no);
	ret += ".dat";
	return string("DBFile\\") + ret;
}

string RecordManager::RecordHeadFileName(const string &tablename)
{
	string ret = tablename;
	ret += "_d_h.dat";
	return string("DBFile\\") + ret;
}

int RecordManager::DataFileNumber()
{
	return curPt->DataFileNumber();
}

int RecordManager::CreateNewRecord(const table &src)
{
	string dathfn = RecordHeadFileName(src.name);
	if (buffer.FileExists(dathfn)) return RECORD_ALREADY_EXIST;

	SQLDataHeadBlock dh_b;
	int dh_b_pos;
	dh_b.SetBlockHead(src);
	buffer.AllocFromBlock(&dh_b, DataHeadBlock, dathfn, dh_b_pos);
	buffer.WriteBlock(dathfn, dh_b_pos);
	return RECORD_SUCCESSFUL;
}

int RecordManager::DropAllRecord(const string &tablename)
{
	string dathfn, datfn;
	dathfn = RecordHeadFileName(tablename);
	if (!buffer.FileExists(dathfn)) return RECORD_NOT_FOUND;
	
	int hpos;
	buffer.AllocFromReadFile(dathfn, DataHeadBlock, dathfn, hpos);
	SQLDataHeadBlock *hpt = (SQLDataHeadBlock*)buffer.BlockPtr(hpos);
	int dfnum = hpt->DataFileNumber();
	for (int i = 1; i <= dfnum; ++i)
	{
		datfn = RecordFileName(tablename, i);
		buffer.SignOutWithID(datfn);
		buffer.DeleteFile(datfn);
	}
	buffer.SignOutWithID(dathfn);
	buffer.DeleteFile(dathfn);
	return RECORD_SUCCESSFUL;
}

int RecordManager::AppointTable(const string &tablename)
{
	if (tablename != curTableName)
	{
		CloseDataFile();
		loaded_in_set = 0;
	}
	if (curPt != nullptr) buffer.SetUnlock(curPos);
	string dathfn = RecordHeadFileName(tablename);
	int pos = buffer.InBufferPos(dathfn);
	if (pos == -1)
	{
		if (!buffer.FileExists(dathfn))
		{
			return RECORD_NOT_FOUND;
		}
		buffer.AllocFromReadFile(dathfn, DataHeadBlock, dathfn, curPos);
	}
	else
	{
		curPos = pos;
	}
	curPt = (SQLDataHeadBlock*)buffer.BlockPtr(curPos);
	curPt->GetFromBlockHead(curTableInfo);
	curTableName = curTableInfo.name;
	buffer.SetLock(curPos);
	return RECORD_SUCCESSFUL;
}

void RecordManager::CloseDataFile()
{
	loaded_in_set = 0;
	if (curDataPt != nullptr && curDataPt->Edited())
	{
		buffer.WriteBlock(curDataPt->GetBlockID(), curDataPos);
	}
	if (curPt != nullptr && curPt->Edited())
	{
		buffer.WriteBlock(curPt->GetBlockID(), curPos);
	}
}

int RecordManager::InsertRecord(const table &src)
{
	int resultcode = BLOCK_FAILED;

	if (loaded_in_set)
	{
		for (int coli = 0; coli < curTableInfo.keyname.size(); ++coli)
		{
			if (curTableInfo.keytype[coli] == "int")
				intset[coli].insert(src.keydata[coli].d[0]);

			if (curTableInfo.keytype[coli] == "float")
				floatset[coli].insert(src.keydata[coli].f[0]);

			if (curTableInfo.keytype[coli] == "char")
				charset[coli].insert(src.keydata[coli].s[0].c_str());
		}
	}

	string datfn;
	if (JustInserted)
	{
		resultcode = curDataPt->InsertData(src);
	}
	else
	{
		for (int i = 1; i <= curPt->DataFileNumber(); ++i)
		{
			datfn = RecordFileName(src.name, i);
			curDataPos = buffer.InBufferPos(datfn);
			if (curDataPos == -1)
			{
				buffer.AllocFromReadFile(datfn, DataBlock, datfn, curDataPos);
				curDataPt = (SQLDataBlock*)buffer.BlockPtr(curDataPos);
			}
			resultcode = curDataPt->InsertData(src);
			if (resultcode == BLOCK_SUCCESSFUL)
			{
				JustInserted = i;
				insert_position = i;
				break;
			}
		}
	}

 	if(!(resultcode == BLOCK_SUCCESSFUL))
	{
		CloseDataFile();
		loaded_in_set = 1;
		JustInserted = curPt->DataFileNumber() + 1;
		curPt->IncreaseFileNumber();
		buffer.WriteBlock(RecordHeadFileName(curTableName), curPos);

		SQLDataBlock newblock;
		newblock.SetBlockHead(curTableInfo);
		newblock.SetBlockID(RecordFileName(curTableName, curPt->DataFileNumber()));
		newblock.InsertData(src);
		datfn = RecordFileName(src.name, JustInserted);
		buffer.AllocFromBlock(&newblock, DataBlock, datfn, curDataPos);
		buffer.WriteBlock(datfn, curDataPos);
		curDataPt = (SQLDataBlock*)buffer.BlockPtr(curDataPos);

		insert_position = JustInserted;
	}

	return BLOCK_SUCCESSFUL;
}

int RecordManager::AlreadyExist(int colno, const string & dataType, datablock data)
{
	//CloseDataFile();
	int i, j;
	string dfn;
	if (!loaded_in_set)
	{
		for (i = 0; i < curTableInfo.keyname.size(); ++i)
		{
			intset[i].clear();
			floatset[i].clear();
			charset[i].clear();
		}
		for (i = 1; i <= curPt->DataFileNumber(); ++i)
		{
			table curFileData;
			dfn = RecordFileName(curTableInfo.name, i);
			curDataPos = buffer.InBufferPos(dfn);
			if (curDataPos == -1)
			{
				buffer.AllocFromReadFile(dfn, DataBlock, dfn, curDataPos);
			}
			curDataPt = (SQLDataBlock*)buffer.BlockPtr(curDataPos);
			curDataPt->GetData(curFileData);

			for (int coli = 0; coli < curTableInfo.keyname.size(); ++coli)
			{
				if (curTableInfo.keytype[coli] == "int")
					for (auto it = curFileData.keydata[coli].d.begin(); it != curFileData.keydata[coli].d.end(); ++it)
						intset[coli].insert(*it);

				if (curTableInfo.keytype[coli] == "float")
					for (auto it = curFileData.keydata[coli].f.begin(); it != curFileData.keydata[coli].f.end(); ++it)
						floatset[coli].insert(*it);

				if (curTableInfo.keytype[coli] == "char")
					for (auto it = curFileData.keydata[coli].s.begin(); it != curFileData.keydata[coli].s.end(); ++it)
						charset[coli].insert(it->c_str());
			}
		}
		loaded_in_set = 1;
	}
	if (dataType == "int")
		return intset[colno].find(data.d[0]) != intset[colno].end();
	if (dataType == "float")
		return floatset[colno].find(data.f[0]) != floatset[colno].end();
	if (dataType == "char")
		return charset[colno].find(data.s[0].c_str()) != charset[colno].end();

	return 0;
}

int RecordManager::GetSelectResult(const Restriction & limits, int target_file)
{
	CloseDataFile();
	JustInserted = 0;
	int i;
	string dfn;
	result.clear();
	curTableInfo.copyTableHead(result);
	int L = (target_file == -1 ? 1 : target_file);
	int R = (target_file == -1 ? curPt->DataFileNumber() : target_file);
	for (i = L; i <= R; ++i)
	{
		table curFileData;
		dfn = RecordFileName(curTableInfo.name, i);
		curDataPos = buffer.InBufferPos(dfn);
		if (curDataPos == -1)
		{
			buffer.AllocFromReadFile(dfn, DataBlock, dfn, curDataPos);
		}
		curDataPt = (SQLDataBlock*)buffer.BlockPtr(curDataPos);
		curDataPt->GetData(curFileData);

		table tmp;
		curDataPt->GetTableInfo(tmp);
		for (int j = 0; j < curDataPt->recnum(); ++j)
		{
			curFileData.copyIthRecord(j, tmp);
			if (limits.result(tmp))
			{
				result.addOneRecord(tmp);
			}
		}
	}
	return RECORD_SUCCESSFUL;
}

int RecordManager::DeleteRecord(const Restriction & limits, int target_file)
{
	CloseDataFile();
	delete_number = 0;
	JustInserted = 0;
	int i;
	string dfn;
	vector<int> deleteno;
	result.clear();
	delete_record.clear();
	curTableInfo.copyTableHead(result);
	curTableInfo.copyTableHead(delete_record);
	int L = (target_file == -1 ? 1 : target_file);
	int R = (target_file == -1 ? curPt->DataFileNumber() : target_file);
	for (i = L; i <= R; ++i)
	{
		table curFileData;
		dfn = RecordFileName(curTableInfo.name, i);
		curDataPos = buffer.InBufferPos(dfn);
		if (curDataPos == -1)
		{
			buffer.AllocFromReadFile(dfn, DataBlock, dfn, curDataPos);
		}
		curDataPt = (SQLDataBlock*)buffer.BlockPtr(curDataPos);
		curDataPt->GetData(curFileData);
		deleteno.clear();

		table tmp;
		curFileData.copyTableHead(tmp);
		for (int j = curDataPt->recnum() - 1; j >= 0; --j)
		{
			curFileData.copyIthRecord(j, tmp);
			if (limits.result(tmp))
			{
				deleteno.push_back(j);
				delete_record.addOneRecord(tmp);
			}
		}
		delete_number += deleteno.size();
		for (auto it = deleteno.begin(); it != deleteno.end(); ++it)
		{
			curDataPt->DeleteData(*it);
		}
		buffer.DeleteFile(dfn);
		buffer.WriteBlock(dfn, curDataPos);
	}
	return RECORD_SUCCESSFUL;
}