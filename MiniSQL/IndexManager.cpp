#include "pch.h"
#include "IndexManager.h"
#include <stack>
#include <utility>
#include <cassert>
using namespace std;

IndexManager::IndexManager() : BUFFER("Index_buf", 0, 1000), MAP_BUFFER("Index_mapping_buf", 0, 10), ID_ALLOC_BUFFER("Index_alloc_buf", 0, 5)
{
	sibling_id = cur_id = father_id = 0;
}


IndexManager::~IndexManager()
{
}

void IndexManager::save_newnode_to_file(const BPTreeNode &src, int id)
{
#ifdef BPTREE_MONITOR
	cout << "Saving new node with ID " << id << ":" << endl;
	src.printInfo();
	cout << "--------------------------------------" << endl;
#endif
	SQLIndexBlock srcblk;
	srcblk.SetFromBPTreeNode(src);
	int pos;
	BUFFER.AllocFromBlock(&srcblk, IndexBlock, string(), pos);
	BUFFER.WriteBlock(IndexFileName(curTableName, curColName, to_string(id)), pos);
}

void IndexManager::import_into_current(int id)
{
	cur_id = id;
	string ifname = IndexFileName(curTableName, curColName, to_string(id));
	BUFFER.AllocFromReadFile(ifname, IndexBlock, string(), curPos);
	curPt = (SQLIndexBlock*)BUFFER.BlockPtr(curPos);
	curPt->ExportToBPTreeNode(curNode);
#ifdef BPTREE_MONITOR
	cout << "Import Into Current Node with ID" << id << ":" << endl;
	curNode.printInfo();
	cout << "--------------------------------------" << endl;
#endif
}

void IndexManager::import_into_father(int id)
{
	father_id = id;
	string ifname = IndexFileName(curTableName, curColName, to_string(id));
	BUFFER.AllocFromReadFile(ifname, IndexBlock, string(), fatherPos);
	fatherPt = (SQLIndexBlock*)BUFFER.BlockPtr(fatherPos);
	fatherPt->ExportToBPTreeNode(fatherNode);
#ifdef BPTREE_MONITOR
	cout << "Import Into Father Node with ID" << id << ":" << endl;
	fatherNode.printInfo();
	cout << "--------------------------------------" << endl;
#endif
}

void IndexManager::import_into_sibling(int id)
{
	sibling_id = id;
	string ifname = IndexFileName(curTableName, curColName, to_string(id));
	BUFFER.AllocFromReadFile(ifname, IndexBlock, string(), siblingPos);
	siblingPt = (SQLIndexBlock*)BUFFER.BlockPtr(siblingPos);
	siblingPt->ExportToBPTreeNode(siblingNode);
#ifdef BPTREE_MONITOR
	cout << "Import Into Sibling Node with ID" << id << ":" << endl;
	siblingNode.printInfo();
	cout << "--------------------------------------" << endl;
#endif
}

void IndexManager::save_cur_father_sibling(int curen, int fatheren, int siblingen)
{
	string ifname;
	if (curen)
	{
#ifdef BPTREE_MONITOR
		cout << "Saving Current Node with ID" << cur_id << ":" << endl;
		curNode.printInfo();
		cout << "--------------------------------------" << endl;
#endif
		curPt->SetFromBPTreeNode(curNode);
		ifname = IndexFileName(curTableName, curColName, to_string(cur_id));
		BUFFER.WriteBlock(ifname, curPos);
	}
	if (fatheren)
	{
#ifdef BPTREE_MONITOR
		cout << "Saving Father Node with ID" << father_id << ":" << endl;
		fatherNode.printInfo();
		cout << "--------------------------------------" << endl;
#endif
		fatherPt->SetFromBPTreeNode(fatherNode);
		ifname = IndexFileName(curTableName, curColName, to_string(father_id));
		BUFFER.WriteBlock(ifname, fatherPos);
	}
	if (siblingen)
	{
#ifdef BPTREE_MONITOR
		cout << "Saving Sibling Node with ID" << sibling_id << ":" << endl;
		siblingNode.printInfo();
		cout << "--------------------------------------" << endl;
#endif
		siblingPt->SetFromBPTreeNode(siblingNode);
		ifname = IndexFileName(curTableName, curColName, to_string(sibling_id));
		BUFFER.WriteBlock(ifname, siblingPos);
	}
}


string IndexManager::IndexFileName(const string & tablename, const string & colname, const string & id)
{
	return string("DBFile\\") + tablename + "_" + colname + "_i" + id + ".index";
}

string IndexManager::IndexMapFileName(int ith)
{
	return string("DBFile\\") + string("index") + to_string(ith) + ".map";
}

string IndexManager::IndexAllocFileName(const string &tablename, const string &colname)
{
	return string("DBFile\\") + string("index") + "_" + tablename + "_" + colname + ".alloc";
}

void IndexManager::CreateNewIndex(const string &indexname, const table &src, const string &colname)
{
	string fname = this->IndexFileName(src.name, colname, "0");

	int colno = src.columnOfName(colname);
	int deg;
	if (src.keytype[colno] == "int" || src.keytype[colno] == "float")
		deg = BPTREE_MAX_DEGREE;
	else
		deg = 3000 / src.keydata[colno].len;

	if (deg > BPTREE_MAX_DEGREE) deg = BPTREE_MAX_DEGREE;
	if (deg < BPTREE_MIN_DEGREE) deg = BPTREE_MIN_DEGREE;
	int ktp;
	if (src.keytype[colno] == "int") ktp = 0;
	if (src.keytype[colno] == "float") ktp = 1;
	if (src.keytype[colno] == "char") ktp = 2;

	BPTreeNode nd(BPTree::leaf, deg, ktp, (ktp != 2 ? 4 : src.keydata[colno].len));

	SQLIndexBlock indblk;
	indblk.SetFromBPTreeNode(nd);
	indblk.SetBlockID("0");

	BUFFER.AllocFromBlock(&indblk, IndexBlock, "0", curPos);
	BUFFER.WriteBlock(fname, curPos);
	

	string mfname;
	int mfno = 1;
	int allfull = 0;
	int tmp;

	SQLDirectEditBlock *bpt = nullptr;
	while (1)
	{
		mfname = IndexMapFileName(mfno);
		if (!MAP_BUFFER.FileExists(mfname))
		{
			allfull = 1;
			break;
		}
		MAP_BUFFER.AllocFromReadFile(mfname, DirectEditBlock, string(), tmp);
		bpt = (SQLDirectEditBlock*)(MAP_BUFFER.BlockPtr(tmp));
		if (bpt->beg[3720])
		{
			mfno++;
			continue;
		}
		else break;
	} 

	if (!MAP_BUFFER.FileExists(mfname))
	{
		SQLDirectEditBlock mblk;
		MAP_BUFFER.AllocFromBlock(&mblk, DirectEditBlock, string(), tmp);
		MAP_BUFFER.WriteBlock(mfname, tmp);
		bpt = (SQLDirectEditBlock*)MAP_BUFFER.BlockPtr(tmp);
	}
	
	int writepos = 0;
	while (bpt->beg[writepos])
	{
		writepos += 120;
	}

	memcpy_s(bpt->beg + writepos + 0 , 40, indexname.c_str(), indexname.size());
	memcpy_s(bpt->beg + writepos + 40, 40, src.name.c_str(), src.name.size());
	memcpy_s(bpt->beg + writepos + 80, 40, colname.c_str(), colname.size());

	MAP_BUFFER.WriteBlock(mfname, tmp);

	//[indexname, tablename, columnname] each 40 bytes
	//0,40,80 ---- 120,160,200 ---- 240,280,320 ---- 360, ... (n-1)*120
	//... 3720 3760 3800; 32/block

	SQLDirectEditBlock alloc;
	int allocpos;
	ID_ALLOC_BUFFER.AllocFromBlock(&alloc, DirectEditBlock, string(), allocpos);
	ID_ALLOC_BUFFER.WriteBlock(IndexAllocFileName(src.name, colname), allocpos);
}

void IndexManager::real_drop_index(const string &tablename, const string &colname)
{
	//AppointIndex(tablename, colname);
	this->dropped_tablename = tablename;
	this->dropped_colname = colname;
	int x;
	ID_ALLOC_BUFFER.AllocFromReadFile(IndexAllocFileName(tablename, colname), DirectEditBlock, string(), x);
	SQLDirectEditBlock *pt = (SQLDirectEditBlock*)ID_ALLOC_BUFFER.BlockPtr(x);
	int maxid;
	memcpy_s(&maxid, 4, pt->beg, 4);
	for (int i = 0; i <= maxid + 2; ++i)
	{
		remove(IndexFileName(tablename, colname, to_string(i)).c_str());
	}
	remove(IndexAllocFileName(tablename, colname).c_str());
}

void IndexManager::DropIndex(const string & indexname)
{
	if (!IndexExistsNamed(indexname)) return;
	string mfname;
	int fno = 1; 
	SQLDirectEditBlock *mpt;
	string in, tn, cn;
	while (1)
	{
		mfname = IndexMapFileName(fno);
		if (!MAP_BUFFER.FileExists(mfname)) return;
		int mpos;
		MAP_BUFFER.AllocFromReadFile(mfname, DirectEditBlock, string(), mpos);
		mpt = (SQLDirectEditBlock *)MAP_BUFFER.BlockPtr(mpos);
		for (int i = 0; i < 32; ++i)
		{
			in = (char*)(mpt->beg + i * 120);
			tn = (char*)(mpt->beg + 40 + i * 120);
			cn = (char*)(mpt->beg + 80 + i * 120);
			if (in == indexname)
			{
				memset(mpt->beg + 120 * i, 0, 120);
				MAP_BUFFER.WriteBlock(mfname, mpos);
				real_drop_index(tn, cn);
				return;
			}
		}
		++fno;
	}
}

void IndexManager::DropIndex(const string &tablename, const string &colname)
{
	if (!IndexExistsOn(tablename, colname)) return;

	string mfname;
	int fno = 1;
	SQLDirectEditBlock *mpt;
	string in, tn, cn;
	while (1)
	{
		mfname = IndexMapFileName(fno);
		if (!MAP_BUFFER.FileExists(mfname)) return;
		int mpos;
		MAP_BUFFER.AllocFromReadFile(mfname, DirectEditBlock, string(), mpos);
		mpt = (SQLDirectEditBlock *)MAP_BUFFER.BlockPtr(mpos);
		for (int i = 0; i < 32; ++i)
		{
			in = (char*)(mpt->beg + i * 120);
			tn = (char*)(mpt->beg + 40 + i * 120);
			cn = (char*)(mpt->beg + 80 + i * 120);
			if (tn == tablename && cn == colname)
			{
				memset(mpt->beg + 120 * i, 0, 120);
				MAP_BUFFER.WriteBlock(mfname, mpos);
				real_drop_index(tablename, colname);
				return;
			}
		}
		++fno;
	}
}

bool IndexManager::IndexExistsNamed(const string & indexname)
{
	int mpos;
	SQLDirectEditBlock *mpt;
	int fno = 1;
	string mfname;

	while (1)
	{
		mfname = IndexMapFileName(fno);
		if(!MAP_BUFFER.FileExists(mfname)) return 0;

		MAP_BUFFER.AllocFromReadFile(mfname, DirectEditBlock, string(), mpos);
		mpt = (SQLDirectEditBlock *)MAP_BUFFER.BlockPtr(mpos);
		for (int i = 0; i < 32; ++i)
		{
			if (indexname == string((char*)(mpt->beg + (i * 120)))) return 1;
		}
		++fno;
	}

	return false;
}

bool IndexManager::IndexExistsOn(const string & tablename, const string & colname)
{
	int mpos;
	SQLDirectEditBlock *mpt;
	int fno = 1;
	string mfname;

	while (1)
	{
		mfname = IndexMapFileName(fno);
		if (!MAP_BUFFER.FileExists(mfname)) return 0;

		MAP_BUFFER.AllocFromReadFile(mfname, DirectEditBlock, string(), mpos);
		mpt = (SQLDirectEditBlock *)MAP_BUFFER.BlockPtr(mpos);
		for (int i = 0; i < 32; ++i)
		{
			if (tablename == string((char*)(mpt->beg + (i * 120) + 40)) && colname == string((char*)(mpt->beg + (i * 120) + 80))) return 1;
		}
		++fno;
	}

	return false;
}

void IndexManager::AppointIndex(const string & tablename, const string & colname)
{
	curTableName = tablename;
	curColName = colname;
	ID_ALLOC_BUFFER.AllocFromReadFile(IndexAllocFileName(tablename, colname), DirectEditBlock, string(), allocPos);
	allocPtr = (SQLDirectEditBlock*)ID_ALLOC_BUFFER.BlockPtr(allocPos);
}

void IndexManager::InsertIntoIndex(int isrc, float fsrc, const string & csrc, int redir)
{
	string ifname, afname;
	afname = IndexAllocFileName(curTableName, curColName);
	
	import_into_current(0);

	ValueInBytes src(curNode.keyType, curNode.keyLength);
	switch (curNode.keyType)
	{
	case 0: 
		src = ValueInBytes(isrc);
		break;
	case 1:
		src = ValueInBytes(fsrc);
		break;
	case 2:
		src = ValueInBytes(csrc, curNode.keyLength);
		break;
	}

	int i, j, kn;
	stack<int> route;
	route.push(0);
	while (curNode.nodeType != BPTree::leaf) //reach the leaf
	{
		kn = curNode.keyNum();
		for (i = 0; i < kn; ++i)
		{
			int rightpos = 0;
			if (src < curNode.key[i]) break;
		}

		int next = curNode.leafRedir[i];
		route.push(next);
		
		import_into_current(next);
	}

	curNode.insertKey(src, redir);

	if (!curNode.overflow())
	{
		ID_ALLOC_BUFFER.WriteBlock(IndexAllocFileName(curTableName, curColName), allocPos);
		//curPt->SetFromBPTreeNode(curNode);
		//BUFFER.WriteBlock(IndexFileName(curTableName, curColName, to_string(route.top())), curPos);
		save_cur_father_sibling(1, 0, 0);
		return;
	}

	/***************************************************************************************************************/
	//Part the leaf:
	//p1[1]p2[2]p3[3]p4[4]p5[5]NULL---->p1[1]p2[2]p3[3]NULL, p4[4]p5[5]NULL; while (4,p_to_4) upward;
	//This part must includes increment on MAX_ID. Any return must have a save on allocated_new_id before them.

	int allocated_new_id;
	memcpy_s(&allocated_new_id, 4, allocPtr->beg, 4);
	pair<ValueInBytes, int> upload;
	if (route.size() > 1)
	{

		BPTreeNode newleaf(BPTree::leaf, curNode.degree, curNode.keyType, curNode.keyLength);

		allocated_new_id++;

		int oldkn = curNode.keyNum();
		for (int i = (oldkn + 1) / 2; i < oldkn; ++i) //5-->3,2,i=(3,4)  4-->2,2,i=(2,3)
		{
			newleaf.insertKey(curNode.key[i], curNode.leafRedir[i]);
		}

		for (int i = (oldkn + 1) / 2; i < oldkn; ++i)
		{
			curNode.key.pop_back();
			curNode.leafRedir.pop_back();
		}

		newleaf.LeafRightPtrID = curNode.LeafRightPtrID;
		curNode.LeafRightPtrID = allocated_new_id;

		//curPt->SetFromBPTreeNode(curNode);
		//BUFFER.WriteBlock(IndexFileName(curTableName, curColName, to_string(route.top())), curPos);
		save_cur_father_sibling(1, 0, 0);
		save_newnode_to_file(newleaf, allocated_new_id);

		//Update upwards: give a (Value, pointer) pair to the parents
		upload.first = newleaf.key[0];
		upload.second = allocated_new_id;

	}

	int solved = 0;
	while (1) 
	{
		route.pop();
		if (route.empty()) break;
		int indexpagepos = route.top();

		import_into_current(indexpagepos);
		string thisindexfn = IndexFileName(curTableName, curColName, to_string(indexpagepos));

		curNode.insertKey(upload.first, upload.second);

		if (!curNode.overflow())
		{
			//curPt->SetFromBPTreeNode(curNode);
			//BUFFER.WriteBlock(thisindexfn, curPos);
			save_cur_father_sibling(1, 0, 0);
			solved = 1; 
			break;
		}

		if (cur_id == 0) break;

		//part the index
		//pt0[1]pt1[2]pt2[3]pt3[4]pt4[5]pt5 -->   pt0[1]pt1[2]pt2, <3,ptr_to_new> ,pt3[4]pt4[5]pt5
		//5->2,2,[3,4]; 4->2,1[3,3];
		allocated_new_id++;

		BPTreeNode newindex(BPTree::medium, curNode.degree, curNode.keyType, curNode.keyLength);
		int ikn = curNode.keyNum();
		newindex.leafRedir.push_back(curNode.leafRedir[ikn - (ikn - 1) / 2]);
		for (i = (ikn - 1) / 2; i >= 1; --i)
		{
			newindex.key.push_back(curNode.key[ikn - i]);
			newindex.leafRedir.push_back(curNode.leafRedir[ikn + 1 - i]);
		}

		curNode.leafRedir.pop_back();
		for (i = (ikn - 1) / 2; i >= 1; --i)
		{
			curNode.key.pop_back();
			curNode.leafRedir.pop_back();
		}

		upload.first = curNode.key[curNode.key.size() - 1];
		upload.second = allocated_new_id;
		curNode.key.pop_back();

		//curPt->SetFromBPTreeNode(curNode);
		//BUFFER.WriteBlock(thisindexfn, curPos);
		save_cur_father_sibling(1, 0, 0);

		save_newnode_to_file(newindex, allocated_new_id);
		/*SQLIndexBlock nib;
		nib.SetFromBPTreeNode(newindex);
		int nipos;
		BUFFER.AllocFromBlock(&nib, IndexBlock, string(), nipos);
		BUFFER.WriteBlock(IndexFileName(curTableName, curColName, to_string(allocated_new_id)), nipos);*/
	}


	if (!solved) 
	{
		// part the root
		/*
			case #1: root is leaf
			Before : ptr0<0>ptr1<1>ptr2<2>ptr3<3>ptr4<4>ptr5<5>
			After  : [L: ptr0<0>ptr1<1>ptr2<2>]<--[Root: newP0<3>newP1]-->[R: ptr3<3>ptr4<4>ptr5<5>]
			odd: n7,p7->n4,p4|n3,p3;
			even: n6,p6->n3,p3|n3,p3;

			case #2: root is not leaf
			Before : ptr0<1>ptr1<2>ptr2<3>ptr3<4>ptr4<5>ptr5<6>ptr6
			After  : [L: ptr0<0>ptr1<1>ptr2<2>ptr3]<--[Root: newP0<4>newP1]-->[R: ptr4<5>ptr5<6>Ptr6]
			odd: n7,p8->n3,p4|n3,p4
			even: n6,p7->n3,p4|n2,p3
		*/

		int subNodeType = (curNode.nodeType == BPTree::leaf ? BPTree::leaf : BPTree::medium);
		BPTreeNode newLnode(subNodeType, curNode.degree, curNode.keyType, curNode.keyLength);
		BPTreeNode newRnode(subNodeType, curNode.degree, curNode.keyType, curNode.keyLength);
		
		++allocated_new_id;
		int Lid = allocated_new_id;
		++allocated_new_id;
		int Rid = allocated_new_id;

		if (subNodeType == BPTree::leaf)
		{
			//case #1
			for (i = 0; i < (curNode.keyNum() + 1) / 2; ++i)
			{
				newLnode.insertKey(curNode.key[i], curNode.leafRedir[i]);
			}
			for (; i < curNode.keyNum(); ++i)
			{
				newRnode.insertKey(curNode.key[i], curNode.leafRedir[i]);
			}
			newLnode.LeafRightPtrID = Rid;

			curNode.nodeType = BPTree::root;
			curNode.clearContent();
			curNode.key.push_back(newRnode.key[0]);
			curNode.leafRedir.push_back(Lid);
			curNode.leafRedir.push_back(Rid);

			//curPt->SetFromBPTreeNode(curNode);
			//BUFFER.WriteBlock(IndexFileName(curTableName, curColName, "0"), curPos);
			save_cur_father_sibling(1, 0, 0);

			save_newnode_to_file(newLnode, Lid);
			save_newnode_to_file(newRnode, Rid);
		}
		else
		{
			//case #2
/*			odd: n7,p8->n3,p4|n3,p4
			even: n6,p7->n3,p4|n2,p3*/
			//curNode.insertKey(upload.first, upload.second);
			newLnode.leafRedir.push_back(curNode.leafRedir[0]);
			for (i = 0; i < curNode.keyNum() / 2; ++i)
			{
				newLnode.key.push_back(curNode.key[i]);
				newLnode.leafRedir.push_back(curNode.leafRedir[i + 1]);
			}
			ValueInBytes mid = curNode.key[i];

			for (++i; i < curNode.keyNum(); ++i)
			{
				newRnode.key.push_back(curNode.key[i]);
				newRnode.leafRedir.push_back(curNode.leafRedir[i]);
			}
			newRnode.leafRedir.push_back(curNode.leafRedir[i]);

			curNode.clearContent();
			curNode.key.push_back(mid);
			curNode.leafRedir.push_back(Lid);
			curNode.leafRedir.push_back(Rid);

			//curPt->SetFromBPTreeNode(curNode);
			//BUFFER.WriteBlock(IndexFileName(curTableName, curColName, "0"), curPos);
			save_cur_father_sibling(1, 0, 0);

			save_newnode_to_file(newLnode, Lid);
			save_newnode_to_file(newRnode, Rid);
		}
		
	}
	
	memcpy_s(allocPtr->beg, 4, &allocated_new_id, 4);
	ID_ALLOC_BUFFER.WriteBlock(IndexAllocFileName(curTableName, curColName), allocPos);
}

int IndexManager::SearchKey(int itarget, float ftarget, const string & ctarget)
{
	ValueInBytes target;
	string ifname;
	import_into_current(0);
	
	switch (curNode.keyType)
	{
	case 0:
		target = ValueInBytes(itarget);
		break;
	case 1:
		target = ValueInBytes(ftarget);
		break;
	case 2:
		target = ValueInBytes(ctarget, curNode.keyLength);
		break;
	}

	while (curNode.nodeType != BPTree::leaf)
	{
		int i;
		int kn = curNode.keyNum();
		for (i = 0; i < kn; ++i)
		{
			int rightpos = 0;
			if (target < curNode.key[i]) break;
		}

		int next = curNode.leafRedir[i];
		import_into_current(next);
	}

	for (int i = 0; i < curNode.keyNum(); ++i)
	{
		if (target == curNode.key[i]) return curNode.leafRedir[i];
	}

	return -1;
}

void IndexManager::DeleteFromIndex(int ival, float fval, const string & cval) //Assume val is in index.
{
	ValueInBytes target;
	import_into_current(0);
	switch (curNode.keyType)
	{
	case 0:
		target = ValueInBytes(ival);
		break;
	case 1:
		target = ValueInBytes(fval);
		break;
	case 2:
		target = ValueInBytes(cval, curNode.keyLength);
		break;
	}

	stack<pair<int, int> > route_and_posi; //posi is the position in leafRedir;
	pair<int, int> once;
	once.first = 0;

	while (curNode.nodeType != BPTree::leaf)
	{
		int i;
		int kn = curNode.keyNum();
		for (i = 0; i < kn; ++i)
		{
			int rightpos = 0;
			if (target < curNode.key[i]) break;
		}
		int next = curNode.leafRedir[i];
		once.second = i;
		route_and_posi.push(once);

		once.first = next;
		import_into_current(next);
	}
	once.second = -1;
	route_and_posi.push(once);

	ValueInBytes MostLeft, MostRight;
	bool MostLeftAltered = 0, MostRightAltered = 0;

	if (curNode.key[0] == target)
	{
		MostLeft = curNode.key[0];
		MostLeftAltered = 1;
	}
	if (curNode.key[curNode.key.size() - 1] == target)
	{
		MostRight = curNode.key[curNode.key.size() - 1];
		MostRightAltered = 1;
	}

	curNode.deleteKey(target);

	once = route_and_posi.top();
	if (!curNode.insufficient() || once.first == 0)
	{
		//curPt->SetFromBPTreeNode(curNode);
		//BUFFER.WriteBlock(IndexFileName(curTableName, curColName, to_string(route_and_posi.top().first)), curPos);
		save_cur_father_sibling(1, 0, 0);
		return;
	}
	
	// Leaf is insufficient:
	pair<int, int> father;
	int noMoreMerging = 0;
	
	while (1)
	{
		route_and_posi.pop();
		if (route_and_posi.empty()) break;
		if (!curNode.insufficient()) return;
		father = route_and_posi.top();

		/* Key Redistribution. */
		/* 
			non-index/leaf(From left): 
						p0[0]p1[1]p2[2]p3[3],  p4[4]p5[5] -->
						p0[0]p1[1]p2[2], p3[3]p4[4]p5[5]; Update Index key for 4 to become 3
			non-index/leaf(From right):
						p0[0]p1[1], p2[2]p3[3]p4[4]p5[5]-->
						p0[0]p1[1]p2[2], p3[3]p4[4]p5[5] 


			index(From left): 
						p0[1]p1[2]p2[3]p3[4]p4,([5] at higher level), p5[6]p6 -->
						p0[1]p1[2]p2[3]p3 ([4] at higher level) p4[5]p5[6]p6;
			index(From right):
						<curNode_redir>
						{curNode : p0[1]p1},([2] at higher level), {sibling : p2[3]p3[4]p4[5]p5[6]p6} -->
						p0[1]p1[2]p2 ([3] at higher level) p3[4]p4[5]p5[6]p6;

		*/

		int given = 0;
		import_into_father(father.first);
		int son_i = father.second;
		if (son_i + 1 < fatherNode.leafRedir.size()) //The rightward sibling give curNode a key
		{
			import_into_sibling(fatherNode.leafRedir[son_i + 1]);
			if (siblingNode.enoughToGive())
			{
				given = 1;
				if (curNode.nodeType == BPTree::leaf) //leaf from right
				{
					fatherNode.replaceKey(fatherNode.key[son_i], siblingNode.key[1]);
					curNode.insertKey(siblingNode.key[0], siblingNode.leafRedir[0]);
					siblingNode.deleteKey(siblingNode.key[0]);
				} 
				else //index from right
				{
					curNode.key.push_back(fatherNode.key[son_i]);
					curNode.leafRedir.push_back(siblingNode.leafRedir[0]);

					fatherNode.replaceKey(fatherNode.key[son_i], siblingNode.key[0]);

					siblingNode.key.erase(siblingNode.key.begin());
					siblingNode.leafRedir.erase(siblingNode.leafRedir.begin());
				}
			}
		}
		if(son_i - 1 >= 0 && !given) //Leftward sibling gives the key
		{
			//cur_id = fatherNode.leafRedir[son_i];
			import_into_sibling(fatherNode.leafRedir[son_i - 1]);
			if (siblingNode.enoughToGive())
			{
				given = 1;
				if (curNode.nodeType == BPTree::leaf) //leaf from left
				{
					fatherNode.replaceKey(fatherNode.key[son_i - 1], siblingNode.key[siblingNode.keyNum() - 1]);/*update*/
					curNode.insertKey(siblingNode.key[siblingNode.keyNum()-1], siblingNode.leafRedir[siblingNode.leafRedir.size() - 1]);
					siblingNode.deleteKey(siblingNode.key[siblingNode.keyNum() - 1]);
				}
				else //index from left
				{
					curNode.key.insert(curNode.key.begin(), fatherNode.key[son_i - 1]);
					curNode.leafRedir.insert(curNode.leafRedir.begin(), siblingNode.leafRedir[siblingNode.leafRedir.size() - 1]);
					//curNode.key.push_back(fatherNode.key[son_i - 1]);
					//curNode.leafRedir.push_back(siblingNode.leafRedir[siblingNode.leafRedir.size() - 1]);

					fatherNode.replaceKey(fatherNode.key[son_i - 1], siblingNode.key[siblingNode.keyNum() - 1]);

					siblingNode.deleteKey(siblingNode.key[siblingNode.keyNum() - 1]);
				}
			}
		}

		if (given)  //redistribution solved the problem
		{
			save_cur_father_sibling();
			return;
		}

		//merge siblings:
		/*
			Analysis
			leaf     : p0[0]p1[1] , p2[2]p3[3] --->  p0[0]p1[1]p2[2]p3[3];
			parent   : <ptrAlpha>[0]<ptrBravo>[2]<ptrCharlie>[9]<ptrDelta>  ---> <ptrAlpha>[0]<ptrBravo>[9]<ptrDelta>
				     : parent deleted (key = 2, ptr = right)

			index:   : p0[1]p1[2]p2 , p3[4]p4[5]p5 --->  p0[1]p1[2]p2[3]p3[4]p4[5]p5;
			parent   : <alpha>[?]<bravo>([3]<charlie>)[?]
					 : 

		*/

		int merged = 0;
		if (son_i + 1 < fatherNode.leafRedir.size()) //curNode is the leftward node, merge with the rightward node
		{
			import_into_sibling(fatherNode.leafRedir[son_i + 1]);
			if (curNode.nodeType == BPTree::leaf) //leaf
			{
				merged = 1;
				fatherNode.deleteKey(fatherNode.key[son_i]);
				for (int i = 0; i < siblingNode.keyNum(); ++i)
					curNode.insertKey(siblingNode.key[i], siblingNode.leafRedir[i]);
				curNode.LeafRightPtrID = siblingNode.LeafRightPtrID;
				BUFFER.DeleteFile(IndexFileName(curTableName, curColName, to_string(sibling_id)));
				save_cur_father_sibling(1, 1, 0);
			}
			else //index
			{
				merged = 1;
				curNode.key.push_back(fatherNode.key[son_i]);
				for (int i = 0; i < siblingNode.keyNum(); ++i)
					curNode.key.push_back(siblingNode.key[i]);
				for (int i = 0; i < siblingNode.leafRedir.size(); ++i)
					curNode.leafRedir.push_back(siblingNode.leafRedir[i]);
				fatherNode.deleteKey(fatherNode.key[son_i]);
				BUFFER.DeleteFile(IndexFileName(curTableName, curColName, to_string(sibling_id)));
				save_cur_father_sibling(1, 1, 0);
			}
		}

		if (son_i - 1 >= 0 && !merged) //curNode is the rightward node, merge with the one on its left
		{
			import_into_sibling(fatherNode.leafRedir[son_i - 1]);
			if (curNode.nodeType == BPTree::leaf) //leaf from left
			{
				fatherNode.deleteKey(fatherNode.key[son_i - 1]);
				for (int i = 0; i < curNode.keyNum(); ++i)
					siblingNode.insertKey(curNode.key[i], curNode.leafRedir[i]);
				BUFFER.DeleteFile(IndexFileName(curTableName, curColName, to_string(cur_id)));
				save_cur_father_sibling(0, 1, 1);
			}
			else //index from left
			{
				siblingNode.key.push_back(fatherNode.key[son_i - 1]);
				for (int i = 0; i < curNode.keyNum(); ++i)
					siblingNode.key.push_back(curNode.key[i]);
				for (int i = 0; i < curNode.leafRedir.size(); ++i)
					siblingNode.leafRedir.push_back(curNode.leafRedir[i]);
				fatherNode.deleteKey(fatherNode.key[son_i - 1]);
				BUFFER.DeleteFile(IndexFileName(curTableName, curColName, to_string(cur_id)));
				save_cur_father_sibling(0, 1, 1);
			}
		}

		/*Get new curNode here.*/
		import_into_current(father_id);
	}

	// reset the root;
	//assert(curNode.leafRedir.size() == 1);
	if (curNode.leafRedir.size() != 1)
	{
		save_cur_father_sibling(1, 0, 0);
		return;
	}

	import_into_sibling(curNode.leafRedir[0]); //actually it is children. use this function to make the code simpler.

	curNode.key = siblingNode.key;
	curNode.leafRedir = siblingNode.leafRedir;
	curNode.keyType = siblingNode.keyType;
	save_cur_father_sibling(1, 0, 0);

	BUFFER.DeleteFile(IndexFileName(curTableName, curColName, to_string(sibling_id)));

}

/*
Deletion Analysis:
case #1: leaf is sufficient after deletion.
case #2: leaf is insufficient after deletion, leaf's sibling can provide 
case #3: leaf's sibling cannot provide, merge and update upwards to index, index sufficient after deletion
case #4: index insufficient after deletion, index's sibling can provide
case #5: index's sibling cannot provide, update upwards
case #6: root only have one child, create new root.


*/