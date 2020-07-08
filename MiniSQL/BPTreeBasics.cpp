#include"pch.h"
#include"BPTreeBasics.h"
#include<cmath>
#include<algorithm>
#include<iostream>

BPTreeNode::BPTreeNode()
{
	nodeType = degree = keyLength = keyType = 0;
}

BPTreeNode::BPTreeNode(int node_type, int tree_degree, int key_type, int key_length)
{
	this->nodeType = node_type;
	this->degree = tree_degree;
	this->keyType = key_type;
	this->keyLength = key_length;
}

int BPTreeNode::keyNum() const
{
	return key.size();
}

bool BPTreeNode::overflow()
{
	return keyNum() >= degree;
}

bool BPTreeNode::insufficient()
{
	return leafRedir.size() < (degree) / 2;
}

bool BPTreeNode::enoughToGive()
{
	return !(leafRedir.size() - 1 < (degree + 1) / 2);
}

void BPTreeNode::printInfo() const
{
	cout << "Node Type: " << (nodeType == BPTree::leaf ? "leaf" : "not leaf") << endl;
	cout << "Tree Degree: " << degree << endl;
	cout << "Key Type: " << keyType << endl;
	cout << "Key Length: " << keyLength << endl;
	cout << "Key Values:\n";
	for (auto it = key.begin(); it != key.end(); ++it)
		cout << *it << ' ';
	cout << endl;
	cout << "Redirection:\n";
	for (auto it = leafRedir.begin(); it != leafRedir.end(); ++it)
		cout << *it << ' ';

	cout << endl;
}

void BPTreeNode::clearContent()
{
	this->leafRedir.clear();
	this->key.clear();
	this->nodeID.clear();
	this->LeafRightPtrID = 0;
}

void BPTreeNode::clearAll()
{
	clearContent();
	nodeType = degree = keyType = keyLength = 0;
}

void BPTreeNode::insertKey(const ValueInBytes &src, int redir)
{
	if (this->keyType != src.Tp)
		throw logic_error("Key type error!");

	key.push_back(src);
	leafRedir.push_back(redir);
	int pt = key.size() - 1;
	int rpt = leafRedir.size() - 1;
	while (pt > 0 && key[pt - 1] > src)
	{
		swap(key[pt - 1], key[pt]);
		swap(leafRedir[rpt - 1], leafRedir[rpt]);
		--rpt;
		--pt;
	}
}

void BPTreeNode::deleteKey(const ValueInBytes &target)
{
	if (this->keyType != target.Tp)
		throw logic_error("Key type error!");
	int i, j;
	for (i = 0; i < key.size(); ++i)
	{
		if (key[i] == target)
		{
			int corrRi = i + (leafRedir.size() - key.size());
			for (j = corrRi; j < leafRedir.size() - 1; ++j)
			{
				leafRedir[j] = leafRedir[j + 1];
			}
			leafRedir.pop_back();
			for (j = i; j < key.size() - 1; ++j)
			{
				key[j] = key[j + 1];
			}
			key.pop_back();
			return;
		}
	}
}

void BPTreeNode::replaceKey(const ValueInBytes & oldval, const ValueInBytes & newval, int redir)
{
	if (this->keyType != oldval.Tp || this->keyType != newval.Tp)
		throw logic_error("Key type error!");
	int i, j;
	for (i = key.size() - 1, j = leafRedir.size() - 1; i >= 0; --i, --j)
	{
		if (key[i] == oldval)
		{
			key[i] = newval;
			if (redir != -1) leafRedir[j] = redir;
		}
	}
}
