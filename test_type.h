#pragma once
#include <iostream>

struct TestType
{
	int n;
	int* p;
	static int aliveObjects;

	TestType(int a = 5, int b = 1)
	: n(a),
	p(new int(b))
	{
		aliveObjects++;
	}

	TestType(const TestType& other):
	n(other.n),
	p(new int(*other.p))
	{
		aliveObjects++;
	}

	~TestType()
	{
		delete p;
		aliveObjects--;
	}

	TestType& operator = (const TestType& other)
	{
		n = other.n;
		*p = *other.p;
		return *this;
	}

	bool operator == (const TestType& other) const
	{
		return n == other.n and *p == *other.p;
	}


	bool operator < (const TestType& other) const
	{
		return n < other.n or (n == other.n and *p < *other.p);
	}

	friend 
	std::ostream& operator << (std::ostream& out, TestType t)
	{
		out << t.n;
		return out;
	}

};