#include "pch.h"
#include "CatalogManager.h"
#include <cstdlib>


CatalogManager::CatalogManager() : buf("Catalog_buf", 0, 200), curTablePt(-1), curPt(nullptr)
{
	int i;
}

CatalogManager::~CatalogManager()
{
	if (curPt != nullptr && curPt->Edited())
	{
		buf.WriteBlock(CatalogFileName(curTableInfo.name), curTablePt);
	}
	
}

string CatalogManager::CatalogFileName(const string &tablename)
{
	string ret;
	ret = tablename + "_ca.catalog";
	return string("DBFile\\") + ret;
}

int CatalogManager::CreateCatalog(const table &src)
{
	string tgtfn = CatalogFileName(src.name);
	if (buf.FileExists(tgtfn)) return CATALOG_ALREADY_EXIST;
	int pt;
	SQLCatalogBlock cata;
	cata.SetFromTable(src);
	buf.AllocFromBlock(&cata, CatalogBlock, tgtfn, pt);
	buf.WriteBlock(tgtfn, pt);
	return CATALOG_SUCCESSFUL;
}

int CatalogManager::RemoveCatalog(const string & tablename)
{
	if (curTableName == tablename) curTableName = "";
	string tgtfn = CatalogFileName(tablename);
	if (!buf.FileExists(tgtfn)) return CATALOG_NOT_FOUND;
	int tbpt = buf.InBufferPos(tgtfn);
	if (tbpt != -1) buf.SignOutWithID(tgtfn);
	buf.DeleteFile(tgtfn);
	return CATALOG_SUCCESSFUL;
}

int CatalogManager::AppointTable(const string & tablename)
{
	if (curTableName.size() && curTableName == tablename) return CATALOG_SUCCESSFUL;
	string tgtfn = CatalogFileName(tablename);
	if (curPt != nullptr && curPt->Edited())
	{
		buf.SetUnlock(curTablePt);
		if (curPt->Edited())
		{
			buf.WriteBlock(CatalogFileName(curTableInfo.name), curTablePt);
		}
	}

	int inbufpos = buf.InBufferPos(tgtfn);
	if (inbufpos == -1)
	{
		if (!buf.FileExists(tgtfn)) return CATALOG_NOT_FOUND;
		buf.AllocFromReadFile(CatalogFileName(tablename), CatalogBlock, tgtfn, curTablePt);
	}
	if (inbufpos != -1) curTablePt = inbufpos;
	buf.SetLock(curTablePt);
	curTableName = tablename;
	curPt = (SQLCatalogBlock*)buf.BlockPtr(curTablePt);
	curPt->GetTableInfo(curTableInfo);
	return CATALOG_SUCCESSFUL;
}

bool CatalogManager::CatalogExists(const string & tablename)
{
	return buf.FileExists(CatalogFileName(tablename));
}

void CatalogManager::Close()
{
	if (curPt != NULL && curPt->Edited())
	{
		buf.WriteBlock(CatalogFileName(curTableInfo.name), curTablePt);
	}
	//curPt = NULL;
	//curTablePt = -1;
}

int CatalogManager::StandardTableForm(table &std)
{
	if (curPt == NULL) return CATALOG_NOT_FOUND;
	curPt->GetTableInfo(std);
	return 0;
}

int CatalogManager::Unique(const string & col)
{
	if (curPt == nullptr) return 0;
	int i = (find(curTableInfo.keyname.begin(), curTableInfo.keyname.end(), col) - curTableInfo.keyname.begin());
	return Unique(i);
}

int CatalogManager::Unique(int no)
{
	if (curPt == nullptr) return 0;
	return curTableInfo.unique[no];
}

int CatalogManager::Indexed(int no)
{
	if (curPt == nullptr) return 0;
	return curPt->Indexed(no);
}

int CatalogManager::Indexed(const string & col)
{
	if (curPt == nullptr) return 0;
	int i = (find(curTableInfo.keyname.begin(), curTableInfo.keyname.end(), col) - curTableInfo.keyname.begin());
	return Indexed(i);
}

void CatalogManager::SetIndexStatus(int colno, int indexed)
{
	if (curPt == nullptr) return;
	curPt->setIndexStatus(colno, indexed);
}

string CatalogManager::PrimaryKeyName()
{
	string a0;
	if (curPt == nullptr) return a0;
	string ret;
	ret.clear();
	if (curTableInfo.prikeyname.size() == 0) return ret;
	return curTableInfo.prikeyname[0];
}

int CatalogManager::PrimaryKeyNo()
{
	if (curPt == nullptr) return -1;
	if (curTableInfo.prikeyname.size() == 0) return -1;
	return (find(curTableInfo.keyname.begin(), curTableInfo.keyname.end(), curTableInfo.prikeyname[0]) - curTableInfo.keyname.begin());
}

