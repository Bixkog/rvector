#include <vector>
#include <chrono>
#include <iostream>
#include "rvector.h"
#include "test_type.h"

class BenchTimer
{
	using Clock = std::chrono::high_resolution_clock;
public:
	BenchTimer(std::string name)
	: name(name),
	begin(Clock::now())
	{}

	~BenchTimer()
	{
		auto end = Clock::now();
		std::cout << name << ": " 
				  << std::chrono::duration<double>(end - begin).count()
				  << " s"
				  << std::endl;
	}
private:
	std::string name;
	std::chrono::time_point<Clock> begin;
};

void check_mremap()
{
	std::cout << mm::mremap_skips << " " << mm::grows << std::endl;
	mm::mremap_skips = 0;
	mm::grows = 0;
}

template<typename F, typename... Args>
void bench_function(F f, std::string name, int max_size, Args... args)
{
	for(int size = 10; size < max_size; size *= 10)
	{
		int t = max_size / size / 10;
		auto timer = BenchTimer(name + "/" + std::to_string(size));
		while(t-->0) f(size, args...);
	}
}

template<typename V>
void push_back_bench(int size, typename V::value_type e)
{
	V v;
	V v2;
	for(int i = 0; i < size; ++i)
	{
		v.push_back(e);
		v2.push_back(e);
	}
	std::cout << mm::grows << " " << mm::mremap_skips << std::endl;
	mm::grows = 0;
	mm::mremap_skips = 0;
}


int main()
{
	// bench_function(push_back_bench<std::vector<int>>, 
	// 				"std::vector_TestType_push_back", 10e7, 
	// 				0);
	// bench_function(push_back_bench<rvector<int>>, 
	// 				"rvector_TestType_push_back", 10e7, 
	// 				0);
	push_back_bench<rvector<TestType>>(10e7, {});
	// check_mremap();
	// bench_function(push_back_bench<std::vector<std::string>>, 
	// 				"std::vector_string_push_back", 10e7, 
	// 				"testtesttesttesttesttesttest");
	// bench_function(push_back_bench<rvector<std::string>>, 
	// 				"rvector_string_push_back", 10e7, 
	// 				"testtesttesttesttesttesttest");
	// check_mremap();

}