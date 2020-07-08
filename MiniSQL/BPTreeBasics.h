#pragma once
#include<vector>
#include<string>
#include<stdexcept>
#include"ValueInBytes.h"
using namespace std;

namespace BPTree {
	constexpr int leaf = 1;
	constexpr int root = 2;
	constexpr int medium = 0;
}

class BPTreeNode {

	friend class IndexManager;
	friend class SQLIndexBlock;

private:

public:
	BPTreeNode();
	BPTreeNode(int node_type, int tree_degree, int key_type, int key_length);

	string nodeID;
	int LeafRightPtrID;

	int nodeType;
	int degree;
	int keyType;
	int keyLength;
	int keyNum() const;

	vector<int> leafRedir;

	vector<ValueInBytes> key;
	void insertKey(const ValueInBytes &src, int redir);
	void deleteKey(const ValueInBytes &target);
	void replaceKey(const ValueInBytes &oldval, const ValueInBytes &newval, int redir = -1);

	void clearAll();
	void clearContent();

	bool overflow();
	bool insufficient();
	bool enoughToGive();

	void printInfo() const;

};

