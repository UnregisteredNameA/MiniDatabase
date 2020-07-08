#include "pch.h"
#include <iostream>
#include <sstream>
#include "API.h"
#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <set>

SQLError::SQLError(const string &info)
{
	info_str = info;
}

string SQLError::err_info()
{
	return info_str;
}

void SQLAPI::createTable(const table & src)
{
	int retc;
	retc = cat_manager.CatalogExists(src.name);
	if (retc)
	{
		throw SQLError("The table with this name already exists");
	}
	retc = cat_manager.CreateCatalog(src);
	if (retc != CATALOG_SUCCESSFUL)
	{
		throw SQLError("Table Creation Failed");
	}
	retc = rec_manager.CreateNewRecord(src);
	if (retc != CATALOG_SUCCESSFUL)
	{
		throw SQLError("Table Creation Failed");
	}
	if (src.prikeyname.size())
	{
		ind_manager.CreateNewIndex(string("index_prikey_") + src.name, src, src.prikeyname[0]);
		cat_manager.AppointTable(src.name);
		cat_manager.SetIndexStatus(src.columnOfName(src.prikeyname[0]), 1);
	}
	cat_manager.Close();
}

void SQLAPI::dropTable(const string &tablename)
{
	int retc;
	retc = cat_manager.CatalogExists(tablename);
	if (!retc)
	{
		throw SQLError(string("The table named [") + tablename + "] doesn't exist");
	}
	cat_manager.AppointTable(tablename);
	table std;
	cat_manager.StandardTableForm(std);
	for (int col = 0; col < std.keyname.size(); ++col)
	{
		if (cat_manager.Indexed(col))
		{
			ind_manager.DropIndex(tablename, std.keyname[col]);
		}
	}

	cat_manager.RemoveCatalog(tablename);
	rec_manager.DropAllRecord(tablename);
}

void SQLAPI::selectSQL(const string& tablename, const Restriction &lim)
{
	int retc = cat_manager.CatalogExists(tablename);
	if (!retc)
	{
		throw SQLError(string("The table named [") + tablename + "] doesn't exist");
	}
	int dest = -100;

	cat_manager.AppointTable(tablename);
	for (auto it = lim.List.begin(); it != lim.List.end(); ++it)
	{
		if (it->op == RestrictionOP::EqualTo && cat_manager.Indexed(it->inColno))
		{
			table std;
			cat_manager.StandardTableForm(std);
			ind_manager.AppointIndex(tablename, std.keyname[it->inColno]);

			if (std.keytype[it->inColno] == "int")
				dest = ind_manager.SearchKey(it->oprd, 0, string());
			if (std.keytype[it->inColno] == "float")
				dest = ind_manager.SearchKey(0, it->oprf, string());
			if (std.keytype[it->inColno] == "char")
				dest = ind_manager.SearchKey(0, 0, it->oprs);
		}
		if (dest != -100) break;
	}

	if (dest == -1)
	{
		res.clear();
		return;
	}

	try
	{
		rec_manager.AppointTable(tablename);
		rec_manager.GetSelectResult(lim, (dest == -100 ? -1 : dest));
		res = rec_manager.res;
		cat_manager.AppointTable(tablename);
		table stdhead;
		cat_manager.StandardTableForm(stdhead);
		res.addTableHead(stdhead);
	}
	catch (...)
	{
		throw SQLError("Operation Failed");
	}
}

void SQLAPI::insertSQL(const table &src, int instant_save)
{
	table std;
	cat_manager.AppointTable(src.name);
	cat_manager.StandardTableForm(std);

	int retc = cat_manager.CatalogExists(src.name);
	if (!retc)
	{
		throw SQLError(string("The table named [") + src.name + "] doesn't exist");
	}
	/*Unique Check*/
	for (int eachCol = 0; eachCol < std.keyname.size(); ++eachCol)
	{
		if (cat_manager.Unique(eachCol) || std.prikeyname.size() && std.prikeyname[0] == std.keyname[eachCol])
		{
			/*if (cat_manager.Indexed(eachCol))
			{
				ind_manager.AppointIndex(std.name, std.keyname[eachCol]);
				int findres;
				if (std.keytype[eachCol] == "int")
					findres = ind_manager.SearchKey(src.keydata[eachCol].d[0], 0, string());
				if (std.keytype[eachCol] == "float")
					findres = ind_manager.SearchKey(0, src.keydata[eachCol].f[0], string());
				if (std.keytype[eachCol] == "char")
					findres = ind_manager.SearchKey(0, 0, src.keydata[eachCol].s[0]);
				if (findres != -1)
				{
					throw SQLError(string("Unique check on [") + std.keyname[eachCol] + "] failed!");
				}
			}
			else
			{*/
			rec_manager.AppointTable(src.name);
			int findres = rec_manager.AlreadyExist(eachCol, std.keytype[eachCol], src.keydata[eachCol]);
			if (findres)
			{
				throw SQLError(string("Unique check on [") + std.keyname[eachCol] + "] failed!");
			}
			//}
		}
	}
	//insert into record
	rec_manager.AppointTable(src.name);
	cat_manager.AppointTable(src.name);
	rec_manager.InsertRecord(src);
	if (instant_save) rec_manager.CloseDataFile();

	//insert into index
	int pos = rec_manager.insert_pos;
	cat_manager.AppointTable(src.name);
	for (int eachCol = 0; eachCol < src.keyname.size(); ++eachCol)
	{
		if (cat_manager.Indexed(eachCol) || src.prikeyname.size() && src.prikeyname[0] == src.keyname[eachCol])
		{
			ind_manager.AppointIndex(std.name, std.keyname[eachCol]);
			if (std.keytype[eachCol] == "int")
				ind_manager.InsertIntoIndex(src.keydata[eachCol].d[0], 0, string(), pos);
			if (std.keytype[eachCol] == "float")
				ind_manager.InsertIntoIndex(0, src.keydata[eachCol].f[0], string(), pos);
			if (std.keytype[eachCol] == "char")
				ind_manager.InsertIntoIndex(0, 0, src.keydata[eachCol].s[0], pos);
		}
	}
}

void SQLAPI::insertionEnd()
{
	rec_manager.CloseDataFile();
}

void SQLAPI::deleteSQL(const string &tablename, const Restriction &lim)
{
	int retc = cat_manager.CatalogExists(tablename);
	if (!retc)
	{
		throw SQLError(string("The table named [") + tablename + "] doesn't exist");
	}	
	//if indexed
	int dest = -100;
	for (auto it = lim.List.begin(); it != lim.List.end(); ++it)
	{
		if (it->op == RestrictionOP::EqualTo && cat_manager.Indexed(it->inColno))
		{
			cat_manager.AppointTable(tablename);
			table std;
			cat_manager.StandardTableForm(std);
			ind_manager.AppointIndex(tablename, std.keyname[it->inColno]);

			if (std.keytype[it->inColno] == "int")
				dest = ind_manager.SearchKey(it->oprd, 0, string());
			if (std.keytype[it->inColno] == "float")
				dest = ind_manager.SearchKey(0, it->oprf, string());
			if (std.keytype[it->inColno] == "char")
				dest = ind_manager.SearchKey(0, 0, it->oprs);
		}
	}
	if (dest == -1)
	{
		delete_num = 0;
		return;
	}
	//delete record
	rec_manager.AppointTable(tablename);
	rec_manager.DeleteRecord(lim, (dest < 0 ? -1 : dest));
	
	//delete from index
	int deln = rec_manager.delete_rec.recordNumber();
	table one, std;
	cat_manager.AppointTable(tablename);
	cat_manager.StandardTableForm(std);
	std.copyTableHead(one);
	for (int eachDeletion = 0; eachDeletion < deln; ++eachDeletion)
	{
		rec_manager.delete_rec.copyIthRecord(eachDeletion, one);
		for (int eachCol = 0; eachCol < std.keyname.size(); ++eachCol)
		{
			if (cat_manager.Indexed(eachCol))
			{
				ind_manager.AppointIndex(std.name, std.keyname[eachCol]);

				if (std.keytype[eachCol] == "int")
					ind_manager.DeleteFromIndex(one.keydata[eachCol].d[0], 0, string());
				if (std.keytype[eachCol] == "float")
					ind_manager.DeleteFromIndex(0, one.keydata[eachCol].f[0], string());
				if (std.keytype[eachCol] == "char")
					ind_manager.DeleteFromIndex(0, 0, one.keydata[eachCol].s[0]);
			}
		}
	}
	delete_num = rec_manager.delete_num;
}

void SQLAPI::createIndex(const string &indexname, const table &src, const string &colname)
{
	if (ind_manager.IndexExistsNamed(indexname)) throw SQLError(string("Index named ") + indexname + " already exists!");
	if (ind_manager.IndexExistsOn(src.name, colname)) throw SQLError(string("Index on ") + src.name + "(" + colname + ")" + " already exists!");

	if (!cat_manager.CatalogExists(src.name))
		throw SQLError(string("The table named [") + src.name + "] doesn't exist");
	cat_manager.AppointTable(src.name);
	table std;
	cat_manager.StandardTableForm(std);
	int colno = std.columnOfName(colname);
	if (colno == -1)
		throw SQLError(string("The table named [") + src.name + "] doesn't have a column named [" + colname + "]");
	if (!cat_manager.Unique(colno))
		throw SQLError(string("The column named [") + colname + "] is not unique!");

	ind_manager.CreateNewIndex(indexname, std, colname);
	ind_manager.AppointIndex(src.name, colname);
	rec_manager.AppointTable(src.name);

	int filen = rec_manager.DataFileNumber();
	cat_manager.SetIndexStatus(colno, 1);

	for (int eachFile = 1; eachFile <= filen; ++eachFile)
	{
		rec_manager.GetSelectResult(Restriction(), eachFile);
		table one;
		std.copyTableHead(one);
		int recn = rec_manager.res.recordNumber();

		for (int eachRec = 0; eachRec < recn; ++eachRec)
		{
			rec_manager.res.copyIthRecord(eachRec, one);
			if (std.keytype[colno] == "int")
				ind_manager.InsertIntoIndex(one.keydata[colno].d[0], 0, string(), eachFile);
			if (std.keytype[colno] == "float")
				ind_manager.InsertIntoIndex(0, one.keydata[colno].f[0], string(), eachFile);
			if (std.keytype[colno] == "char")
				ind_manager.InsertIntoIndex(0, 0, one.keydata[colno].s[0], eachFile);
		
		}
	}
	cat_manager.Close();
}

void SQLAPI::dropIndex(const string & indexname)
{
	if (!ind_manager.IndexExistsNamed(indexname))
	{
		throw SQLError("Index named [" + indexname + "] doesn't exist!");
	}
	ind_manager.DropIndex(indexname);
	cat_manager.AppointTable(ind_manager.IndexDroppedTable);
	table std;
	cat_manager.StandardTableForm(std);
	cat_manager.SetIndexStatus(std.columnOfName(ind_manager.IndexDroppedColumn), 0);
	cat_manager.Close();
}