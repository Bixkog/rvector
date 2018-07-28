#include <vector>
#include <chrono>
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
				  << " ms"
				  << std::endl;
	}
private:
	std::string name;
	std::chrono::time_point<Clock> begin;
};

int main()
{
	{
		rvector<TestType> v;
		TestType e{};
		{
			auto timer = BenchTimer("rvector TestType 10e7");
			for(int i = 0; i < 10000000; ++i)
				v.push_back(e);
		}
		std::cout << mm::mremap_skips << " " << mm::grows << std::endl;
		
		std::vector<TestType> v_;
		{
			auto timer = BenchTimer("std::vector TestType 10e7");
			for(int i = 0; i < 10000000; ++i)
				v_.push_back(e);
		}
		mm::mremap_skips = 0;
		mm::grows = 0;
	}

	{
		std::string e = "askdbjhbavsbdjkhvcas";
		{
			auto timer = BenchTimer("rvector TestType 10e7");
			for(int t = 0; t < 10; ++t)
			{
				rvector<std::string> v;
				for(int i = 0; i < 10000000; ++i)
					v.push_back(e);
			}
		}
		std::cout << mm::mremap_skips << " " << mm::grows << std::endl;
		
		
		{
			auto timer = BenchTimer("std::vector TestType 10e7");
			for(int t = 0; t < 10; ++t)
			{
				std::vector<std::string> v_;
				for(int i = 0; i < 10000000; ++i)
					v_.push_back(e);
			}
		}
		mm::mremap_skips = 0;
		mm::grows = 0;
	}
}