#pragma once
#include<vector>
#include<string>
using namespace std;

typedef unsigned char Byte;

class ValueInBytes {

	friend bool operator<(const ValueInBytes&, const ValueInBytes&);
	friend bool operator>(const ValueInBytes&, const ValueInBytes&);
	friend bool operator<=(const ValueInBytes&, const ValueInBytes&);
	friend bool operator>=(const ValueInBytes&, const ValueInBytes&);
	friend bool operator==(const ValueInBytes&, const ValueInBytes&);
	friend bool operator!=(const ValueInBytes&, const ValueInBytes&);

public:
	int Tp;
	int len;
	vector<Byte> value;

	ValueInBytes();
	ValueInBytes(int IntValue);
	ValueInBytes(float FloatValue);
	ValueInBytes(const string &CharValue, int stdlen);

	ValueInBytes(int valueType, int length);
	void SetFromBytes(const Byte *src, int num);

	void clear();

	int interpretAsInt() const;
	float interpretAsFloat() const;
	string interpretAsChar() const;

	~ValueInBytes();

	ValueInBytes &operator=(const ValueInBytes &b);
};

bool operator<(const ValueInBytes &a, const ValueInBytes &b);
bool operator>(const ValueInBytes &a, const ValueInBytes &b);
bool operator<=(const ValueInBytes &a, const ValueInBytes &b);
bool operator>=(const ValueInBytes &a, const ValueInBytes &b);
bool operator==(const ValueInBytes &a, const ValueInBytes &b);
bool operator!=(const ValueInBytes &a, const ValueInBytes &b);

ostream &operator<<(ostream &out, const ValueInBytes v);
