#include "pch.h"
#include "SQLBlock.h"
#include <cstring>

SQLBlock::SQLBlock()
{
	memset(bytes, 0, sizeof(bytes));
	edited = 0;
}

string SQLBlock::TableName()
{
	char tmp[45];
	memset(tmp, 0, sizeof(tmp));
	memcpy_s(tmp, 40, bytes, 40);
	string ret = tmp;
	return ret;
}

int SQLBlock::SetBlockHead(const table &src)
{
	edited = 1;
	int i, j;
	if (src.name.size() > 40) return BLOCK_FAILED;
	for (i = 0; i < src.name.size(); ++i) bytes[i] = src.name[i];
	int coln = src.keytype.size();
	bytes[40] = coln;
	bytes[41] = bytes[42] = 0;
	int a = 43;
	for (i = 0; i < coln; ++i)
	{
		if (src.keytype[i] == "int") bytes[a] = 0;
		if (src.keytype[i] == "float") bytes[a] = 1;
		if (src.keytype[i] == "char")
		{
			bytes[a] = 2;
			bytes[a + 1] = src.keydata[i].len;
		}
		a += 2;
	}
	return BLOCK_SUCCESSFUL;
}

void SQLBlock::GetFromBlockHead(table &ret)
{
	ret.clear();
	char cstr[45];
	memset(cstr, 0, sizeof(cstr));
	memcpy_s(cstr, 40, bytes, 40);
	ret.name = cstr;
	int coln = bytes[40];
	for (int i = 0; i < coln; ++i)
	{
		BYTE a, b;
		a = bytes[43 + i * 2];
		ret.keyname.push_back("");
		ret.unique.push_back(0);
		b = bytes[44 + i * 2];
		if (a == 0)
		{
			ret.keytype.push_back("int");
			ret.keydata.push_back(datablock());
		}
		if (a == 1)
		{
			ret.keytype.push_back("float");
			ret.keydata.push_back(datablock());
		}
		if (a == 2)
		{
			ret.keytype.push_back("char");
			datablock dat;
			dat.len = b;
			ret.keydata.push_back(dat);
		}
	}
}

void SQLBlock::ShowContent(ostream &out, int l, int r, char otp)
{
	if (otp == 'd')
	{
		for (int i = l; i < r; ++i)
			out << (int)bytes[i] << ",";
	}
	if (otp == 'h')
	{
		for (int i = l; i < r; ++i)
			out << hex << (int)bytes[i] << dec << "," ;
	}
	if (otp == 'c')
	{
		for (int i = l; i < r; ++i)
			out << (char)bytes[i] << dec << ",";
	}
}

int SQLBlock::Edited()
{
	return edited;
}

void SQLBlock::SetBlockID(const string &id)
{
	for (int i = 0; i < BLOCK_DATA_BEGIN - BLOCK_ID_BEGIN; ++i)
	{
		if (i < id.size())
			bytes[BLOCK_ID_BEGIN + i] = id[i];
		else
			bytes[BLOCK_ID_BEGIN + i] = 0;
	}
}

string SQLBlock::GetBlockID()
{
	string ret;
	for (int i = BLOCK_ID_BEGIN; i < BLOCK_DATA_BEGIN; ++i)
	{
		if (bytes[i]) ret.append(1, bytes[i]);
		if (!bytes[i]) break;
	}
	return ret;
}




SQLCatalogBlock::SQLCatalogBlock() : SQLBlock()
{
	
}

void SQLCatalogBlock::GetTableInfo(table &ret)
{
	int i, j;
	ret.clear();
	GetFromBlockHead(ret);
	int pt;
	char tmp[40]; 
	string tmps;
	for (i = 0; i < ret.keyname.size(); ++i)
	{
		pt = BLOCK_DATA_BEGIN + 34 * i;
		memset(tmp, 0, sizeof(tmp));
		memcpy_s(tmp, 32, bytes + pt, 32);
		tmps = tmp;
		ret.keyname[i] = tmps;
		ret.unique[i] = bytes[pt + 32];
	}
	if (bytes[1000])
	{
		ret.prikeyname.push_back(ret.keyname[bytes[1001]]);
	}
}

int SQLCatalogBlock::SetFromTable(const table &src)
{
	edited = 1;
	SetBlockHead(src);
	int i;
	int pt = BLOCK_DATA_BEGIN;
	for (i = 0; i < src.keyname.size(); ++i)
	{
		pt = BLOCK_DATA_BEGIN + 34 * i;
		memcpy_s(bytes + pt, 32, src.keyname[i].c_str(), src.keyname[i].size());
		bytes[pt + 32] = src.unique[i];
	}
	if (src.prikeyname.size())
	{
		bytes[1000] = 1;
		bytes[1001] = (find(src.keyname.begin(), src.keyname.end(), src.prikeyname[0]) - src.keyname.begin());
	}
	return 0;
}

int SQLCatalogBlock::Indexed(int no)
{
	return bytes[BLOCK_DATA_BEGIN + 34 * no + 33];
}

void SQLCatalogBlock::setIndexStatus(int no, int status)
{
	edited = 1;
	bytes[BLOCK_DATA_BEGIN + 34 * no + 33] = !!(status);
}





SQLDataBlock::SQLDataBlock() : SQLBlock()
{

}

int SQLDataBlock::recnum()
{
	return (bytes[41] << 8) + bytes[42];
}

void SQLDataBlock::GetTableInfo(table &ret)
{
	ret.clear();
	GetFromBlockHead(ret);
}

void SQLDataBlock::GetData(table &ret)
{
	ret.clear();
	char tmpch[50];
	memset(tmpch, 0, sizeof(tmpch));
	memcpy_s(tmpch, 40, bytes, 40);
	ret.name = tmpch;
	int coln = bytes[40];
	int reclen = 0;
	for (int i = 0; i < coln; ++i)
	{
		BYTE a, b;
		a = bytes[43 + i * 2];
		b = bytes[44 + i * 2];
		ret.keyname.push_back("NULL");
		if (a == 0)
		{
			ret.keytype.push_back("int");
			ret.keydata.push_back(datablock());
			reclen += 4;
		}
		if (a == 1)
		{
			ret.keytype.push_back("float");
			ret.keydata.push_back(datablock());
			reclen += 4;
		}
		if (a == 2)
		{
			ret.keytype.push_back("char");
			datablock dat;
			dat.len = b;
			ret.keydata.push_back(dat);
			reclen += b;
		}
	}
	int recn = (bytes[41] << 8) + bytes[42];
	int tmpint;
	float tmpfloat;
	string tmpstr;
	int pt = BLOCK_LENGTH;
	for (int i = 0; i < recn; ++i)
	{
		for (int j = 0; j < coln; ++j)
		{
			if (ret.keytype[j] == "int")
			{
				unsigned int tmp = (bytes[pt - 4] << 24) + (bytes[pt - 3] << 16) + (bytes[pt - 2] << 8) + (bytes[pt - 1]);
				int tmp2;
				memcpy_s(&tmp2, 4, &tmp, 4);
				ret.keydata[j].d.push_back(tmp2);
				pt -= 4;
			}
			if (ret.keytype[j] == "float")
			{
				unsigned int tmp = (bytes[pt - 4] << 24) + (bytes[pt - 3] << 16) + (bytes[pt - 2] << 8) + (bytes[pt - 1]);
				float tmp2;
				memcpy_s(&tmp2, 4, &tmp, 4);
				ret.keydata[j].f.push_back(tmp2);
				pt -= 4;
			}
			if (ret.keytype[j] == "char")
			{
				tmpstr.clear();
				for (int k = 0; k < ret.keydata[j].len; ++k)
				{
					tmpstr += bytes[pt - 1];
					--pt;
				}
				ret.keydata[j].s.push_back(tmpstr);
			}
		}
	}
}

int SQLDataBlock::InsertData(const table &rec)
{
	edited = 1;
	int i, j;
	table std;
	GetTableInfo(std); 
	if (!std.equalType(rec)) return TABLE_TYPE_ERROR;
	int len = 0;
	for (i = 0; i < std.keytype.size(); ++i)
	{
		if (std.keytype[i] == "int") len += 4;
		if (std.keytype[i] == "float") len += 4;
		if (std.keytype[i] == "char") len += std.keydata[i].len;
	}
	int recn = recnum();
	int data_size = BLOCK_LENGTH - BLOCK_DATA_BEGIN;
	if (data_size / len < recn + 2) return BLOCK_FULL;

	int pt = BLOCK_LENGTH - len * recn;
	for (int j = 0; j < std.keytype.size(); ++j)
	{
		if (std.keytype[j] == "int")
		{
			unsigned int tmp;
			memcpy_s(&tmp, 4, &rec.keydata[j].d[0], 4);
			bytes[pt - 4] = (tmp >> 24)& 0xFF;
			bytes[pt - 3] = (tmp >> 16)& 0xFF;
			bytes[pt - 2] = (tmp >> 8) & 0xFF;
			bytes[pt - 1] = (tmp >> 0) & 0xFF;
			pt -= 4;
		}
		if (std.keytype[j] == "float")
		{
			unsigned int tmp;
			memcpy_s(&tmp, 4, &rec.keydata[j].f[0], 4);
			bytes[pt - 4] = (tmp >> 24) & 0xFF;
			bytes[pt - 3] = (tmp >> 16) & 0xFF;
			bytes[pt - 2] = (tmp >> 8) & 0xFF;
			bytes[pt - 1] = (tmp >> 0) & 0xFF;
			pt -= 4;
		}
		if (std.keytype[j] == "char")
		{
			for (int k = 0; k < std.keydata[j].len; ++k)
			{
				if (k < rec.keydata[j].s[0].size())
					bytes[pt - 1] = rec.keydata[j].s[0].at(k);
				else
					bytes[pt - 1] = 0;
				--pt;
			}
		}
	}
	recn++;
	bytes[41] = recn >> 8;
	bytes[42] = recn & 0xFF;
	return BLOCK_SUCCESSFUL;
}

int SQLDataBlock::DeleteData(int no)
{
	edited = 1;
	int i, j;
	table std; 
	GetTableInfo(std);
	int recn = recnum();
	if (no >= recn || no < 0) return BLOCK_FAILED;
	int len = 0;
	no++;
	for (i = 0; i < std.keytype.size(); ++i)
	{
		if (std.keytype[i] == "int") len += 4;
		if (std.keytype[i] == "float") len += 4;
		if (std.keytype[i] == "char") len += std.keydata[i].len;
	}
	/*
	0                 15
	0000,0000,0000,0000
	*/
	memcpy_s(bytes + (BLOCK_LENGTH - no*len), len, bytes + (BLOCK_LENGTH - recn*len), len);

	recn--;	
	bytes[41] = recn >> 8;
	bytes[42] = recn & 0xFF;
	return BLOCK_SUCCESSFUL;
}





SQLDataHeadBlock::SQLDataHeadBlock() : SQLBlock()
{
}

int SQLDataHeadBlock::DataFileNumber()
{
	return (bytes[200] << 24) + (bytes[201] << 16) + (bytes[202] << 8) + (bytes[203]);
}

void SQLDataHeadBlock::IncreaseFileNumber()
{
	edited = 1;
	int n = DataFileNumber();
	n++;
	bytes[200] = (n >> 24) & 0xFF;
	bytes[201] = (n >> 16) & 0xFF;
	bytes[202] = (n >> 8) & 0xFF;
	bytes[203] = (n) & 0xFF;
}

void SQLDataHeadBlock::DecreaseFileNumber()
{
	edited = 1;
	int n = DataFileNumber();
	if (n > 0) --n;
	bytes[200] = (n >> 24) & 0xFF;
	bytes[201] = (n >> 16) & 0xFF;
	bytes[202] = (n >> 8) & 0xFF;
	bytes[203] = (n) & 0xFF;
}



void SQLIndexBlock::SetFromBPTreeNode(const BPTreeNode &src)
{
	edited = 1;

	memset(bytes, 0, BLOCK_LENGTH);

	bytes[BLOCK_DATA_BEGIN + 0] = src.degree;
	bytes[BLOCK_DATA_BEGIN + 1] = src.nodeType;
	bytes[BLOCK_DATA_BEGIN + 2] = src.keyType;
	bytes[BLOCK_DATA_BEGIN + 3] = src.keyLength >> 8;
	bytes[BLOCK_DATA_BEGIN + 4] = src.keyLength & 0xFF;

	//memcpy_s(bytes + BLOCK_INDEX_RIGHT_PTR, 100, &src.LeafRightPtrID.c_str(), src.LeafRightPtrID.size());
	bytes[BLOCK_INDEX_RIGHT_PTR + 0] = (src.LeafRightPtrID >> 24) & 0xFF;
	bytes[BLOCK_INDEX_RIGHT_PTR + 1] = (src.LeafRightPtrID >> 16) & 0xFF;
	bytes[BLOCK_INDEX_RIGHT_PTR + 2] = (src.LeafRightPtrID >>  8) & 0xFF;
	bytes[BLOCK_INDEX_RIGHT_PTR + 3] = (src.LeafRightPtrID >>  0) & 0xFF;

	int keyn = src.keyNum();
	bytes[BLOCK_INDEX_KEYN_AT] = keyn & 0xFF;
	bytes[BLOCK_INDEX_KEYN_AT + 1] = src.leafRedir.size();

	int pt = BLOCK_INDEX_KEY_BEGIN;
	for (int i = 0; i < keyn; ++i)
	{
		switch (src.keyType)
		{
		case 0: case 1:
			bytes[pt + 0] = src.key[i].value[0];
			bytes[pt + 1] = src.key[i].value[1];
			bytes[pt + 2] = src.key[i].value[2];
			bytes[pt + 3] = src.key[i].value[3];
			pt += 4;
			break;

		case 2:
			string ckey = src.key[i].interpretAsChar();
			memcpy_s(bytes + pt, src.keyLength, ckey.c_str(), ckey.size());
			pt += src.keyLength;
			break;
		}
	}

	pt = BLOCK_INDEX_REDIR_BEGIN;
	for (int i = 0; i < src.leafRedir.size(); ++i)
	{
		unsigned int tmp = src.leafRedir[i];
		bytes[pt + 0] = (tmp >> 24) & 0xFF;
		bytes[pt + 1] = (tmp >> 16) & 0xFF;
		bytes[pt + 2] = (tmp >> 8) & 0xFF;
		bytes[pt + 3] = (tmp >> 0) & 0xFF;
		pt += 4;
	}
}

void SQLIndexBlock::ExportToBPTreeNode(BPTreeNode & dst)
{
	dst.clearAll();
	dst.degree = bytes[BLOCK_DATA_BEGIN + 0];
	dst.nodeType = bytes[BLOCK_DATA_BEGIN + 1];
	dst.keyType = bytes[BLOCK_DATA_BEGIN + 2];
	dst.keyLength = (bytes[BLOCK_DATA_BEGIN + 3] << 8) + bytes[BLOCK_DATA_BEGIN + 4];

	int keyn = bytes[BLOCK_INDEX_KEYN_AT];
	int pt = BLOCK_INDEX_KEY_BEGIN;
	char tmps[257];
	for (int i = 0; i < keyn; ++i)
	{
		ValueInBytes val(dst.keyType);
		switch (dst.keyType)
		{
		case 0: case 1:
			val.SetFromBytes(bytes+pt, 4);
			val.len = 4;
			val.Tp = dst.keyType;
			dst.key.push_back(val);
			pt += 4;
			break;

		case 2:
			val.len = dst.keyLength;
			val.SetFromBytes(bytes + pt, val.len);
			val.Tp = dst.keyType;
			dst.key.push_back(val);
			pt += dst.keyLength;
			break;
		}
	}

	memset(tmps, 0, sizeof(tmps));
	memcpy_s(tmps, 100, bytes + BLOCK_INDEX_RIGHT_PTR, 100);
	dst.LeafRightPtrID = (bytes[BLOCK_INDEX_RIGHT_PTR] << 24) 
					   + (bytes[BLOCK_INDEX_RIGHT_PTR + 1] << 16) 
		               + (bytes[BLOCK_INDEX_RIGHT_PTR + 2] << 8)
		               + (bytes[BLOCK_INDEX_RIGHT_PTR + 3]);


	pt = BLOCK_INDEX_REDIR_BEGIN;
	int x; 
	int rn = bytes[BLOCK_INDEX_KEYN_AT + 1];
	for (int i = 0; i < rn; ++i)
	{
		x = (bytes[pt + 0] << 24) + (bytes[pt + 1] << 16) + (bytes[pt + 2] << 8) + (bytes[pt + 3] << 0);
		pt += 4;
		dst.leafRedir.push_back(x);
	}
}

