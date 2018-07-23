#include <vector>
#include "rvector.h"
#include "test_type.h"

int main()
{
	rvector<TestType> v;
	TestType e{};
	for(int i = 0; i < 100000; ++i)
		v.push_back(e);
}