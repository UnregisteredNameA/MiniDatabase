#include "pch.h"
#include "interpreter.h"
#include <sstream>
#include "API.h"
#include <set>
#include "CatalogManager.h"
using namespace std;

char interpreter::string_sign;

static bool validName(const string &s)
{
	int flag = 0;
	for (int i = 0; i < s.size(); ++i) if (isalpha(s[i]) || s[i] == '_') flag = 1;
	if (!flag) return 0;
	for (int i = 0; i < s.size(); ++i) if (!isalnum(s[i]) && s[i] != '_') return 0;
	return 1;
}

static bool validNumber(const string &s)
{
	int dotn = 0;
	for (int i = 0; i < s.size(); ++i)
	{
		if (s[i] == '.') dotn++;
		if (!isdigit(s[i]) && s[i] != '.') return 0;
	}
	if (dotn > 1) return 0;
	return 1;
}

static bool validInt(const string &s)
{
	for (int i = 0; i < s.size(); ++i) if (!isdigit(s[i])) return 0;
	return 1;
}

static bool validString(const string &s) // 'abc\'def'
{
	if (s[0] != interpreter::current_string_sign() || s[s.size() - 1] != interpreter::current_string_sign()) return 0;
	for (int i = 1; i < s.size()-1; ++i)
	{
		if (s[i] == interpreter::current_string_sign() && s[i - 1] != '\\') return 0;
	}
	return 1;
}

char interpreter::current_string_sign()
{
	return string_sign;
}

void interpreter::set_string_sign(char sgn)
{
	interpreter::string_sign = sgn;
}

static bool strMatchAtTheEnd(const string &src, const string &sub)
{
	if (src.size() < sub.size()) return 0;
	for (int i = 1; i <= sub.size(); ++i) if (src.at(src.size() - i) != sub.at(sub.size() - i)) return 0;
	return 1;
}

static void eliminateSpaceOfTwoEnds(string &src)
{
	int l, r;
	for (l = 0; l < src.size(); ++l) if (src[l] != ' ') break;
	for (r = src.size()-1; r >= 0; --r) if (src[r] != ' ') break;
	if (l > r)
	{
		src.clear();
		return;
	}
	src = src.substr(l, r - l + 1);
}

interpreter::interpreter() : in(cin.rdbuf())
{
	info_display = 1; 
	instant_save = 1;
	input_from_screen = 1;
	in >> noskipws;
	fin >> noskipws;
	string_sign = '\'';//string_sign
}

interpreter::~interpreter()
{
	fin.close();
}

void interpreter::readsql()
{
	sql.clear();
	rawsql.clear();
	char c = 0;
	bool EscapeCharacter = 0;
	bool instring = 0;
	while (c != ';')
	{
		if (!in.good()) return;
		in >> c;
		if (!EscapeCharacter && c == '\\')
		{
			EscapeCharacter = 1;
			continue;
		}
		else
		{
			EscapeCharacter = 0;
		}
		if (c == '\n' || c == '\t') c = ' ';
		if (!EscapeCharacter && c == string_sign) instring = !instring;
		bool addspace = !EscapeCharacter && !instring && (c == '(' || c == ')' || c == ',' || c == ';');
		if (addspace) sql += ' ';
		sql += c;
		if (addspace) sql += ' ';
		if (addspace) rawsql += ' ';
		rawsql += c;
		if (addspace) rawsql += ' ';
	}
	transform(sql.begin(), sql.end(), sql.begin(), ::tolower);
}

void interpreter::readsql_fromfile()
{
	sql.clear();
	rawsql.clear();
	char c = 0;
	bool EscapeCharacter = 0;
	bool instring = 0;
	while (c != ';')
	{
		if (!fin.good()) return;
		fin >> c;
		if (!EscapeCharacter && c == '\\')
		{
			EscapeCharacter = 1;
			continue;
		}
		else
		{
			EscapeCharacter = 0;
		}
		if (c == '\n' || c == '\t') c = ' ';
		if (!EscapeCharacter && c == string_sign) instring = !instring;
		bool addspace = !EscapeCharacter && !instring && (c == '(' || c == ')' || c == ',' || c == ';');
		if (addspace) sql += ' ';
		sql += c;
		if (addspace) sql += ' ';
		if (addspace) rawsql += ' ';
		rawsql += c;
		if (addspace) rawsql += ' ';
	}
	transform(sql.begin(), sql.end(), sql.begin(), ::tolower);
}

int interpreter::checksqltype()
{
	stringstream ss(sql);
	string s1, s2;
	ss >> s1 >> s2;
	if (s1 == "quit") return QUIT;
	if (s1 == "execfile") return EXECFILE;
	if (s1 == "create" && s2 == "table") return CREATE_TABLE;
	if (s1 == "create" && s2 == "index") return CREATE_INDEX;
	if (s1 == "drop" && s2 == "table") return DROP_TABLE;
	if (s1 == "drop" && s2 == "index") return DROP_INDEX;
	if (s1 == "select") return SELECT;
	if (s1 == "insert" && s2 == "into") return INSERT;
	if (s1 == "delete" && s2 == "from") return DELETE;
	if (s1 == "stringsign") return SET_STRING_SIGN;
	error_info = "Invalid command!";
	return INVALID;
}

int interpreter::whereCheck(const string &whereclause) //a>=7 and b<=3.34 and c='xxx'; 
{
	string s, cond;
	lim.List.clear();
	bool instr = 0, escapech = 0;
	char ch;
	int opc = 0;
	int L, R; int chi = -1;
	for(int ci = 0; ci < whereclause.size(); ++ci)
	{
		++chi;
		ch = whereclause[ci];
		if (escapech == 0)
		{
			if (ch == '\\')
			{
				escapech = 1; 
				continue;
			}
		}
		else
		{
			escapech = 0;
		}
		cond += ch;
		if (!escapech && ch == string_sign) instr = !instr;

		if (strMatchAtTheEnd(cond, "<") && !escapech && !instr && whereclause[ci + 1] != '>' && whereclause[ci + 1] != '=')
		{
			if (opc != 0) return FAILED;
			L = chi - 1; 
			R = chi + 1;
			opc = 1;
		}
		if (strMatchAtTheEnd(cond, "<>") && !escapech && !instr)
		{
			if (opc != 0) return FAILED;
			L = chi - 2;
			R = chi + 1;
			opc = 2;
		}
		if (strMatchAtTheEnd(cond, ">") && !escapech && !instr && whereclause[ci + 1] != '=')
		{
			if (!(opc == 0 || opc == 2)) return FAILED;
			if (opc == 0)
			{
				opc = 4;
				L = chi - 1;
				R = chi + 1;
			}
		}
		if (strMatchAtTheEnd(cond, ">=") && !escapech && !instr)
		{
			if (!(opc == 0)) return FAILED; 
			L = chi - 2;
			R = chi + 1;
			opc = 8;
		}
		if (strMatchAtTheEnd(cond, "<=") && !escapech && !instr)
		{
			if (!(opc == 0)) return FAILED;
			L = chi - 2;
			R = chi + 1;
			opc = 16;
		}
		if (strMatchAtTheEnd(cond, "=") && !escapech && !instr)
		{
			if (!(opc == 0 || opc == 8 || opc == 16)) return FAILED;
			if (opc == 0)
			{
				opc = 32;
				L = chi - 1;
				R = chi + 1;
			}
		}

		int end_of_rest = 0;
		if (strMatchAtTheEnd(cond, " and ") && !escapech && !instr)
		{
			end_of_rest = 1;
		}
		if (strMatchAtTheEnd(cond, ";") && !escapech && !instr)
		{
			end_of_rest = 2;
		}

		if (end_of_rest)
		{
			chi = -1;
			string reststr = (end_of_rest == 1 ? cond.substr(0, cond.size() - 5) : cond.substr(0, cond.size() - 1));
			string oprL = reststr.substr(0, L - 0 + 1);
			string oprR = reststr.substr(R, reststr.size()-1 - R + 1);
			eliminateSpaceOfTwoEnds(oprL);
			eliminateSpaceOfTwoEnds(oprR);
			
			RestricionItem rest;
			switch (opc)
			{
			case 1: rest.op = RestrictionOP::LessThan; break;
			case 2: rest.op = RestrictionOP::NotEqualTo; break;
			case 4: rest.op = RestrictionOP::GreaterThan; break;
			case 8: rest.op = RestrictionOP::GreaterOrEqualToThan; break;
			case 16: rest.op = RestrictionOP::LessOrEqualToThan; break;
			case 32: rest.op = RestrictionOP::EqualTo; break;
			}
			if (validName(oprL))
			{
				rest.inColName = oprL;
				rest.inLR = RestrictionOP::Left;
				if (validName(oprR)) return FAILED;
				if (validNumber(oprR)) { rest.oprf = stof(oprR); rest.oprd = stoi(oprR); rest.constType = (validInt(oprR) ? 0 : 1); }
				else if (validString(oprR)) { rest.oprs = oprR.substr(1, oprR.size() - 2); rest.constType = 2; }
				else return FAILED;
			}
			else if (validName(oprR))
			{
				rest.inColName = oprR;
				rest.inLR = RestrictionOP::Right;
				if (validNumber(oprL)) { rest.oprf = stof(oprL); rest.oprd = stoi(oprL); rest.constType = (validInt(oprL) ? 0 : 1); }
				else if (validString(oprL)) { rest.oprs = oprL.substr(1, oprL.size() - 2); rest.constType = 2; }
				else return FAILED;
			}
			else return FAILED;
			lim.List.push_back(rest);

			if (end_of_rest == 1)
			{
				cond.clear();
				opc = 0;
				continue;
			}
			if (end_of_rest == 2)
			{
				return SUCCESSFUL;
			}
		}
	}
	return FAILED;
}

int interpreter::setColNo()
{
	table tmp;
	CM.AppointTable(res.name);
	CM.StandardTableForm(tmp);

	for (auto it = lim.List.begin(); it != lim.List.end(); ++it)
	{
		it->inColno = tmp.columnOfName(it->inColName);
		if (it->inColno == -1)
		{
			error_info = string("Column named [") + it->inColName + "] not found!";
			return FAILED;
		}
		if (it->constType == 0)
		{
			if (tmp.keytype[it->inColno] == "char")
			{
				error_info = "Key type doesn't match!";
				return FAILED;
			}
			continue;
		}
		if (it->constType == 1)
		{
			if (tmp.keytype[it->inColno] == "char")
			{
				error_info = "Key type doesn't match!";
				return FAILED;
			}
			if (tmp.keytype[it->inColno] == "int")
			{
				// == 5.5 --> no result
				// != 5.5 --> all result
				// > 5.5 --> > 5
				// < 5.5 --> < 6
				// >= 5.5 --> >= 6
				// <= 5.5 --> <= 5

				//?
				error_info = "Key type doesn't match!";
				return FAILED;
			}
			continue;
		}
		if (it->constType == 2)
		{
			if (tmp.keytype[it->inColno] != "char")
			{
				error_info = "Key type doesn't match!";
				return FAILED;
			}
			continue;
		}
	}

	CM.Close();
	return SUCCESSFUL;
}

int interpreter::createTable()
{
	int i, j, len;
	stringstream ss(sql), rss(rawsql);
	string s, s1, s2, rs, rs1, rs2;
	ss >> s >> s >> s;
	rss >> rs >> rs >> rs;

	if (!validName(s))
	{
		error_info = "Invalid table name!";
		return FAILED;
	}
	res.clear();
	res.name = rs;
	ss >> s; rss >> rs;
	error_info = "Invalid syntax!";
	if (s != "(") return FAILED;
	while (ss >> s1 && ss >> s2 && rss >> rs1 && rss >> rs2)
	{
		if (!validName(s1)) return FAILED;
		if (s1 == "primary" && s2 == "key")
		{
			ss >> s; rss >> rs; if (s != "(") return FAILED;
			ss >> s; rss >> rs; if (!validName(s)) return FAILED;
			res.prikeyname.push_back(rs);
			ss >> s; rss >> rs;  if (s != ")") return FAILED;
			ss >> s; rss >> rs;
			if (s == ",") continue;
			if (s == ")")
			{
				ss >> s; rss >> rs;
				if (s == ";") break; else return FAILED;
			}
		}
		res.keyname.push_back(rs1);
		if (s2 != "int" && s2 != "float" && s2 != "char")
		{
			error_info = "Invalid type \'";
			error_info += s2; error_info += "\'!";
			return FAILED;
		}
		if (s2 == "int") res.keytype.push_back("int");
		if (s2 == "float") res.keytype.push_back("float");
		if (s2 == "char") res.keytype.push_back("char");
		if (s2 == "char")
		{
			ss >> s; rss >> rs; if (s != "(") return FAILED;
			ss >> s; rss >> rs;
			for (i = 0; i < s.size(); ++i) if (!isdigit(s[i])) return FAILED;
			len = stoi(s);
			if (len < 1 || len > 255)
			{
				error_info = "Char type length should be ranged from 1 to 255!";
				return FAILED;
			}
			ss >> s; rss >> rs; if (s != ")") return FAILED;
		}
		datablock dt;
		if (s2 == "char") dt.len = len;
		res.keydata.push_back(dt);
		ss >> s; rss >> rs;
		if (s == "unique")
		{
			res.unique.push_back(1);
			ss >> s; rss >> rs;
		}
		else if (s == "primary")
		{
			ss >> s; rss >> rs;
			if (s == "key") res.prikeyname.push_back(rs1); else return FAILED;
			res.unique.push_back(1);
			ss >> s; rss >> rs;
		}
		else
		{
			res.unique.push_back(0);
		}
		if (s == ",") continue;
		if (s == ")")
		{
			ss >> s; rss >> rs;
			if (s == ";") break; else return FAILED;
		}
		return FAILED;
	}

	if (res.prikeyname.size() > 1)
	{
		error_info = "There can only be one primary key!";
		return FAILED;
	}
	if (res.prikeyname.size())
	{
		int flag = 0;
		for (auto it = res.keyname.begin(); it != res.keyname.end(); ++it)
		{
			if (*it == res.prikeyname[0]) flag = 1;
		}
		if (!flag)
		{
			error_info = "Primary key not found!";
			return FAILED;
		}
	}
	set<string> st;
	for (auto it = res.keyname.begin(); it != res.keyname.end(); ++it)
	{
		if (st.find(*it) != st.end())
		{
			error_info = "Duplicated key name!";
			return FAILED;
		}
		st.insert(*it);
	}

	return SUCCESSFUL;
}

int interpreter::dropTable()
{
	res.clear();
	stringstream ss(sql), rss(rawsql);
	string s, rs;
	ss >> s >> s >> s;
	rss >> rs >> rs >> rs;
	error_info = "Syntax Error";
	res.name = rs;
	ss >> s;
	if (s != ";") return FAILED;
	return SUCCESSFUL;
}

int interpreter::insertSQL()
{
	res.clear();
	int i, j, len;
	stringstream ss(sql), rss(rawsql);
	string s, s1, s2, rs, rs1, rs2, tmp;
	ss >> s >> s >> s;
	rss >> rs >> rs >> rs;

	res.name = rs;
	error_info = "Invalid Syntax!";
	
	ss >> s; rss >> rs;
	if (s != "values") return FAILED; //insert into xxx values 

	ss >> s; rss >> rs;
	if (s != "(") return FAILED; //insert into xxx values(

	datablock tmpdbk;

	while (1) //insert into xxx values( 'abc' , 1 , 2.57 , 4 ) ;
	{
		if (!ss.good()) return FAILED;
		ss >> s; rss >> rs;
		tmpdbk.clear();
		
		if (s[0] == string_sign)
		{
			if (s[s.size() - 1] != string_sign)
			{
				ss >> noskipws; rss >> noskipws;
				char c;
				tmp.clear();
				tmp = rs;
				while (ss.good() && (ss >> c) && (rss >> c) && c != string_sign)
				{
					tmp.append(1, c);
				}
				ss >> skipws; rss >> skipws;
				tmp.append(1, string_sign);
			}
			else
			{
				tmp = rs;
			}
			tmp = tmp.substr(1, tmp.size() - 2);
			if (!ss.good()) return FAILED;
			res.keytype.push_back("char");
			tmpdbk.len = tmp.size();
			tmpdbk.s.push_back(tmp);
			res.keydata.push_back(tmpdbk);
			res.keyname.push_back("NULL");
			res.unique.push_back(0);
			ss >> s; rss >> rs;
			if (s == ",") continue;
			if (s == ")")
			{
				ss >> s;
				if (s == ";") return SUCCESSFUL; else return FAILED;
			}
			return FAILED;
		}
		else if(isdigit(s[0]))
		{
			if (!validNumber(s)) return FAILED;
			float tmpf = stof(s);
			if (tmpf == (int)tmpf && s.find('.') == string::npos)
			{
				res.keytype.push_back("int");
			}
			else
			{
				res.keytype.push_back("float");
			}
			tmpdbk.d.push_back((int)tmpf);
			tmpdbk.f.push_back(tmpf);
			res.keydata.push_back(tmpdbk);
			res.keyname.push_back("NULL");
			res.unique.push_back(0);
			ss >> s; rss >> rs;
			if (s == ",") continue;
			if (s == ")")
			{
				ss >> s;
				if (s == ";") return SUCCESSFUL; else return FAILED;
			}
			return FAILED;
		}
		else return FAILED;
	}
}

int interpreter::deleteSQL()
{
	res.clear();
	stringstream ss(sql), rss(rawsql);
	string s, rs;
	ss >> s >> s >> s;
	rss >> rs >> rs >> rs;
	res.name = rs;
	ss >> s; rss >> rs;
	error_info = "Syntax Error!";
	if (s != "where")
	{
		if (s == ";")
		{
			lim.List.clear();
			return SUCCESSFUL;
		}
		return FAILED;
	}
	rss >> noskipws;
	string tmp;
	char c;
	while (rss.good())
	{
		rss >> c;
		tmp.append(1, c);
	}
	int a = whereCheck(tmp);
	if (a == FAILED) return FAILED;
	a = setColNo();
	if (a == FAILED) return FAILED;
	return SUCCESSFUL;
}

int interpreter::selectSQL()
{
	res.clear();
	stringstream ss(sql), rss(rawsql);
	string s, rs;
	ss >> s >> s;
	rss >> rs >> rs;
	error_info = "Syntax Error";
	if (s != "*") return FAILED;
	ss >> s; rss >> rs;
	if (s != "from") return FAILED;
	ss >> s; rss >> rs;
	res.name = rs;
	ss >> s; rss >> rs;
	if (s != "where")
	{
		if (s == ";")
		{
			lim.List.clear();
			return SUCCESSFUL;
		}
		return FAILED;
	}
	getline(rss, rs);
	int a = whereCheck(rs);
	if (a == FAILED) return FAILED;
	a = setColNo();
	if (a == FAILED)
	{
		return FAILED;
	}
	return SUCCESSFUL;
	return 0;
}

int interpreter::createIndex() //create index index0 on table1(colx);
{
	res.clear();
	stringstream ss(sql), rss(rawsql);
	string s, rs;
	ss >> s >> s >> s;
	rss >> rs >> rs >> rs;
	error_info = "Syntax Error";

	string index_name, table_name, column_name;

	index_name = rs;
	 
	ss >> s;
	rss >> rs;
	if (s != "on") return FAILED;
	
	ss >> s;
	rss >> rs;
	table_name = rs;

	ss >> s; 
	rss >> rs;
	if (s != "(") return FAILED;

	ss >> s;
	rss >> rs;
	column_name = rs;

	ss >> s;
	rss >> rs;
	if (s != ")") return FAILED;

	ss >> s;
	if (s != ";") return FAILED;

	/* Lack: See if index is already created */

	if (!CM.CatalogExists(table_name))
	{
		error_info = string("Table [") + table_name + "] not found!";
		return FAILED;
	}
	CM.AppointTable(table_name);
	CM.StandardTableForm(res);
	if (res.columnOfName(column_name) == -1)
	{
		error_info = string("This table doesn't have column [") + column_name + "]!";
		return FAILED;
	}

	indexName = index_name;
	colName = column_name;
	return SUCCESSFUL;
}

int interpreter::dropIndex()
{
	res.clear();
	stringstream ss(sql), rss(rawsql);
	string s, rs;
	ss >> s >> s >> s;
	rss >> rs >> rs >> rs;
	error_info = "Syntax Error";
	this->indexName = rs;
	ss >> s;
	if (s != ";") return FAILED;
	return SUCCESSFUL;
}

int interpreter::execfile()
{
	res.clear();
	stringstream ss(sql), rss(rawsql);
	string s, rs;
	ss >> s >> s;
	rss >> rs >> rs;
	error_info = "Syntax Error";
	string fn = rs;
	ss >> s;
	if (s != ";") return FAILED;
	fin.close();
	fin.open(rs);
	if (!fin.good())
	{
		cout << "File not found!" << endl;
		return FAILED;
	}
	info_display = 0;
	instant_save = 0;
	input_from_screen = 0;
	while (fin.good()) 
		run();
	API.insertionEnd();

	cout << "SQL File Executed." << endl;

	info_display = 1;
	instant_save = 1;
	input_from_screen = 1;
	fin.close();
	return SUCCESSFUL;
}

void interpreter::run()
{
	if (info_display)
	{
		cout << endl;
		cout << ">>";
	}
	if (input_from_screen == 1)
		readsql();
	else
		readsql_fromfile();
	int ctp = checksqltype();
	//cout << "Operation: " << ctp << endl;
	cmdType = ctp;
	if (ctp == CREATE_TABLE)
	{
		int sta = createTable();
		if (sta == FAILED)
		{
			if(info_display) cout << error_info << endl;
			return;
		}
		try 
		{
			API.createTable(res);
		}
		catch (SQLError err)
		{
			if (info_display) cout << err.err_info() << endl;
			return;
		}
		if (info_display) cout << "Query OK.\n";
		return;
	}
	if (ctp == DROP_TABLE)
	{
		int sta = dropTable();
		if (sta == FAILED)
		{
			if (info_display) cout << error_info << endl;
			return;
		}
		try
		{
			API.dropTable(res.name);
		}
		catch (SQLError err)
		{
			if (info_display) cout << err.err_info() << endl;
			return;
		}
		if (info_display) cout << "Query OK.\n";
		return;
	}
	if (ctp == INSERT)
	{
		int sta = insertSQL();
		if (sta != SUCCESSFUL)
		{
			if (info_display) cout << error_info << endl;
			return;
		}
		CM.AppointTable(res.name);
		CM.StandardTableForm(tmp);
		if (!tmp.equalType(res))
		{
			if (info_display) cout << "Values doesn't match the table!" << endl;
			return;
		}
		try
		{
			API.insertSQL(res, instant_save);
		}
		catch (SQLError err)
		{
			if (info_display) cout << err.err_info() << endl;
			return;
		}
		if (info_display) cout << "Query OK.\n";
		return;
	}
	if (ctp == SELECT)
	{
		int sta = selectSQL();
		if (sta != SUCCESSFUL)
		{
			if (info_display) cout << error_info << endl;
			return;
		}
		try
		{
			API.selectSQL(res.name, lim);
		}
		catch (SQLError err)
		{
			if (info_display) cout << err.err_info() << endl;
			return;
		}
		if (info_display) cout << "Query OK.\n";
		API.results.printTableContent();
		return;
	}
	if (ctp == DELETE)
	{
		int sta = deleteSQL();
		if (sta != SUCCESSFUL)
		{
			if (info_display) cout << error_info << endl;
			return;
		}
		try
		{
			API.deleteSQL(res.name, lim);
		}
		catch (SQLError err)
		{
			if (info_display) cout << err.err_info() << endl;
			return;
		}
		if (info_display) cout << "Query OK." << endl;
		cout << API.delete_num << " tuples deleted in total." << endl;
		return;
	}
	if (ctp == CREATE_INDEX)
	{
		int sta = createIndex();
		if (sta != SUCCESSFUL)
		{
			if (info_display) cout << error_info << endl;
			return;
		}
		try
		{
			API.createIndex(indexName, res, colName);
		}
		catch (SQLError err)
		{
			if (info_display) cout << err.err_info() << endl;
			return;
		}
		if (info_display) cout << "Query OK." << endl;
		return;
	}
	if (ctp == DROP_INDEX)
	{
		int sta = dropIndex();
		if (sta != SUCCESSFUL)
		{
			if (info_display) cout << error_info << endl;
			return;
		}
		try
		{
			API.dropIndex(indexName);
		}
		catch (SQLError err)
		{
			if (info_display) cout << err.err_info() << endl;
			return;
		}
		if (info_display) cout << "Query OK." << endl;
		return;
	}
	if (ctp == SET_STRING_SIGN)
	{
		stringstream rss(rawsql);
		string x, f;
		rss >> x >> x;
		if (x.size() != 1)
		{
			cout << "Invalid sign" << endl;
			return;
		}
		rss >> f;
		if (f != ";")
		{
			cout << "Invalid Syntax" << endl;
			return;
		}
		set_string_sign(x[0]);
		cout << "Set successfully." << endl;
		return;
	}
	if (ctp == EXECFILE)
	{
		execfile();
	}
	if (ctp == QUIT)
	{
		exit(0);
	}
	if (ctp == EXECFILE) return;
	for (int i = 0; i < sql.size(); ++i)
	{
		if (!isblank(sql[i]))
		{
			cout << "Invalid Syntax!" << endl;
			return;
		}
	}
}


void interpreter::minisql_run()
{
	while (1) run();
}

