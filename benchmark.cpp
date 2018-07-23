#include <benchmark/benchmark.h>
#include "rvector.h"
#include <vector>
#include <string>
#include "test_type.h"

template<typename T>
T init_e()
{
	return {};
}

template<>
std::string init_e<std::string>()
{
	return "testtesttesttest";
}

template <typename V>
void BM_push(benchmark::State& state)
{
	V v;
	auto e = init_e<typename V::value_type>();
	for(auto _ : state)
	{
		for(int i = state.range(0); i--;)
			v.push_back(e);
	}
	state.SetBytesProcessed(
      static_cast<int64_t>(state.iterations())*state.range(0)*sizeof(e));
}

BENCHMARK_TEMPLATE(BM_push, std::vector<int>)->Range(1<<10, 1<<27);
BENCHMARK_TEMPLATE(BM_push, rvector<int>)->Range(1<<10, 1<<27);
BENCHMARK_TEMPLATE(BM_push, std::vector<char>)->Range(1<<10, 1<<27);
BENCHMARK_TEMPLATE(BM_push, rvector<char>)->Range(1<<10, 1<<27);
BENCHMARK_TEMPLATE(BM_push, std::vector<std::array<int, 100>>)->Range(1<<10, 1<<24);
BENCHMARK_TEMPLATE(BM_push, rvector<std::array<int, 100>>)->Range(1<<10, 1<<24);
BENCHMARK_TEMPLATE(BM_push, std::vector<TestType>)->Range(1<<10, 1<<24);
BENCHMARK_TEMPLATE(BM_push, rvector<TestType>)->Range(1<<10, 1<<24);
BENCHMARK_TEMPLATE(BM_push, std::vector<std::string>)->Range(1<<10, 1<<24);
BENCHMARK_TEMPLATE(BM_push, rvector<std::string>)->Range(1<<10, 1<<24);

template <typename V>
void BM_pop(benchmark::State& state)
{
	typename V::value_type e{};
	for(auto _ : state)
	{
		state.PauseTiming();
		V v(state.range(0), e);
		state.ResumeTiming();
		while(!v.empty())
			v.pop_back();
	}
	state.SetBytesProcessed(
      static_cast<int64_t>(state.iterations())*state.range(0)*sizeof(e));
}

BENCHMARK_TEMPLATE(BM_pop, std::vector<int>)->Range(1<<0, 1<<10);
BENCHMARK_TEMPLATE(BM_pop, rvector<int>)->Range(1<<0, 1<<10);


template<typename V>
void BM_fill_c(benchmark::State& state)
{
	typename V::value_type e{};
	for(auto _ : state)
	{
		V v(state.range(0), e);
	}
	state.SetBytesProcessed(
      static_cast<int64_t>(state.iterations())*state.range(0)*sizeof(e));
}

BENCHMARK_TEMPLATE(BM_fill_c, std::vector<int>)->Range(1<<0, 1<<27);
BENCHMARK_TEMPLATE(BM_fill_c, rvector<int>)->Range(1<<0, 1<<27);

template<typename V>
void BM_fill_l(benchmark::State& state)
{
	typename V::value_type e{};
	V v(state.range(0), e);

	for(auto _ : state)
	{
		V v_(v);
	}
	state.SetBytesProcessed(
      static_cast<int64_t>(state.iterations())*state.range(0)*sizeof(e));
}

BENCHMARK_TEMPLATE(BM_fill_l, std::vector<int>)->Range(1<<0, 1<<27);
BENCHMARK_TEMPLATE(BM_fill_l, rvector<int>)->Range(1<<0, 1<<27);



BENCHMARK_MAIN();