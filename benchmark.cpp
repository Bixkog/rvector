#include <benchmark/benchmark.h>
#include "rvector.h"
#include <vector>
#include <string>

template <typename V>
void BM_push(benchmark::State& state)
{
	V v;
	typename V::value_type e;
	for(auto _ : state)
	{
		for(int i = state.range(0); i--;)
			v.push_back(e);
	}
	state.SetBytesProcessed(
      static_cast<int64_t>(state.iterations())*state.range(0));
}

BENCHMARK_TEMPLATE(BM_push, std::vector<int>)->Range(1<<10, 1<<27);
BENCHMARK_TEMPLATE(BM_push, rvector<int>)->Range(1<<10, 1<<27);
BENCHMARK_TEMPLATE(BM_push, std::vector<char>)->Range(1<<10, 1<<27);
BENCHMARK_TEMPLATE(BM_push, rvector<char>)->Range(1<<10, 1<<27);
BENCHMARK_TEMPLATE(BM_push, std::vector<std::array<int, 100>>)->Range(1<<10, 1<<24);
BENCHMARK_TEMPLATE(BM_push, rvector<std::array<int, 100>>)->Range(1<<10, 1<<24);
BENCHMARK_MAIN();