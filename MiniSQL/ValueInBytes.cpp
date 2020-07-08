#include "pch.h"
#include "ValueInBytes.h"
#include <algorithm>
#include <stdexcept>

ValueInBytes::ValueInBytes()
{
	Tp = 0;
	len = 0;
	value.clear();
}

ValueInBytes::ValueInBytes(int _Tp, int _len)
{
	this->Tp = _Tp;
	this->len = _len;
}

ValueInBytes::ValueInBytes(int IntValue)
{
	this->Tp = 0;
	this->len = 4;
	this->value.push_back(IntValue >> 24);
	this->value.push_back((IntValue >> 16) & 0xFF);
	this->value.push_back((IntValue >> 8) & 0xFF);
	this->value.push_back((IntValue >> 0) & 0xFF);
}

ValueInBytes::ValueInBytes(float FloatValue)
{
	this->Tp = 1;
	this->len = 4;
	int tmp;
	memcpy_s(&tmp, 4, &FloatValue, 4);
	this->value.push_back(tmp >> 24);
	this->value.push_back((tmp >> 16) & 0xFF);
	this->value.push_back((tmp >> 8) & 0xFF);
	this->value.push_back((tmp >> 0) & 0xFF);
}

ValueInBytes::ValueInBytes(const string & CharValue, int stdlen)
{
	this->Tp = 2;
	this->len = stdlen;
	for (int i = 0; i < CharValue.size(); ++i)
	{
		value.push_back(CharValue[i]);
	}
}

ValueInBytes::~ValueInBytes()
{
}

void ValueInBytes::clear()
{
	value.clear();
}

void ValueInBytes::SetFromBytes(const Byte *src, int num)
{
	value.clear();
	for (int i = 0; i < num; ++i)
	{
		if (this->Tp == 2 && isblank(src[i])) return;
		value.push_back(src[i]);
	}
}

bool operator<(const ValueInBytes &a, const ValueInBytes &b) 
{
	if (a.Tp != b.Tp)
	{
		throw logic_error("Error from ValueInBytes: Two arguements are not comparable!");
	}
	switch (a.Tp)
	{
	case 0:
		return a.interpretAsInt() < b.interpretAsInt();
	case 1:
		return a.interpretAsFloat() < b.interpretAsFloat();
	case 2:
		return strcmp(a.interpretAsChar().c_str(), b.interpretAsChar().c_str()) < 0;
	}
}

int ValueInBytes::interpretAsInt() const
{
	if (value.size() != 4 || Tp != 0) throw logic_error("Interpretation Failed!");
	return (value[0] << 24) + (value[1] << 16) + (value[2] << 8) + (value[3] << 0);
}

float ValueInBytes::interpretAsFloat() const
{
	if (value.size() != 4 || Tp != 1) throw logic_error("Interpretation Failed!");
	unsigned int tmp = (value[0] << 24) + (value[1] << 16) + (value[2] << 8) + (value[3] << 0);
	float ret;
	memcpy_s(&ret, 4, &tmp, 4);
	return ret;
}

string ValueInBytes::interpretAsChar() const
{
	if(Tp != 2) throw logic_error("Interpretation Failed!");
	string ret;
	for (auto it = value.begin(); it != value.end(); ++it)
	{
		if (*it == 0) return ret;
		ret.append(1, *it);
	}
	return ret;
}

ValueInBytes & ValueInBytes::operator=(const ValueInBytes & b)
{
	this->len = b.len;
	this->Tp = b.Tp;
	this->value = b.value;
	return *this;
}

bool operator>(const ValueInBytes &a, const ValueInBytes &b) 
{
	return (b < a);
}
bool operator<=(const ValueInBytes &a, const ValueInBytes &b)
{
	return !(b < a);
}
bool operator>=(const ValueInBytes &a, const ValueInBytes &b)
{
	return !(a < b);
}
bool operator==(const ValueInBytes &a, const ValueInBytes &b)
{
	return !(a < b) && !(b < a);
}
bool operator!=(const ValueInBytes &a, const ValueInBytes &b)
{
	return (a < b) || (b < a);
}

ostream &operator<<(ostream &out, const ValueInBytes v)
{
	switch (v.Tp)
	{
	case 0:
		out << v.interpretAsInt();
		return out;
	case 1:
		out << v.interpretAsFloat();
		return out;
	case 2:
		out << v.interpretAsChar();
		return out;
	}
}