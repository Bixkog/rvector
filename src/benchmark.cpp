#include <vector>
#include <chrono>
#include <random>
#include <iostream>
#include <functional>
#include <map>
#include <math.h>
#include <fstream>
#include "rvector.h"
#include "test_type.h"
#include <folly/FBVector.h>
#include <boost/container/vector.hpp>
#include <EASTL/vector.h>
#include <new>

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line) {
	return malloc(size);
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line) 
{
    return malloc(size);
}  

template<typename T, typename F>
auto map(F f, std::vector<T> const& v) -> decltype(auto) {
	std::vector<decltype(f(std::declval<T>()))> res;
	res.reserve(v.size());
	for(auto const& e : v) res.emplace_back(f(e));
	return res; 
}

template<typename... Ts>
auto zip(std::vector<Ts> const&... Vs) -> decltype(auto) {
	auto min_size = std::min({Vs.size()...});
	std::vector<std::tuple<Ts...>> res;
	res.reserve(min_size);
	for(size_t i = 0; i < min_size; ++i) {
		res.emplace_back(Vs[i]...);
	}
	return res;
}

class BenchTimer
{
	using Clock = std::chrono::high_resolution_clock;
public:
	BenchTimer(std::string name)
	: name(name),
	begin(Clock::now())
	{}

	double check() {
		return std::chrono::duration<double>(Clock::now() - begin).count();
	}

	~BenchTimer() {
		durations[name] += std::chrono::duration<double>(Clock::now() - begin).count();
	}

	static
	void print_data() {
		for(auto p : BenchTimer::durations) {
			std::cout << p.first << ": " << p.second << "s" << std::endl;
		}
	}

	static
	void save_epoch(int i) {
		for(auto const& [k, v] : durations)
			data[i][k] += v;
	}

	static
	void clear() {
		durations.clear();
	}

	static
	void  clear_data() {
		data.clear();
	}


private:
	std::string name;
	std::chrono::time_point<Clock> begin;
public:
	static std::map<std::string, double> durations;
	static std::vector<std::map<std::string, double>> data;
};

std::map<std::string, double> BenchTimer::durations = {};
std::vector<std::map<std::string, double>> BenchTimer::data = {};

void check_mremap()
{
	std::cout << mm::mremap_skips << " " << mm::grows << std::endl;
	mm::mremap_skips = 0;
	mm::grows = 0;
}

template <template<typename> typename V, typename... Ts>
class VectorEnv {
public:
	VectorEnv(int seed) 
	: rd{},
		gen{rd()},
		env{} {
		gen.seed(seed);
	}

	void RunSimulation(int iter = 1000) {
		for(int i = 0; i < iter; i++) {
			BenchTimer bt("Simulation");
			(dispatch_action<Ts>(i), ...);
			if((i + 1) % 100 == 0) {
				BenchTimer::save_epoch(i / 100);
			}
		}
	}

	size_t GetNeededCapacity() {
		return (typedNeededCapacity<Ts>() + ...);
	}

	size_t GetAverageLength() {
		return (typedAverageLength<Ts>() + ...) / sizeof...(Ts);
	}

	size_t GetActualCapacity() {
		return (typedActualCapacity<Ts>() + ...);
	}

private:
	template <typename T>
	void initVectors() {
		auto& typed_env = std::get<V<V<T>>>(env);
		typed_env.resize(1);
	}

	template <typename T>
	size_t typedNeededCapacity() {
		auto const& typed_env = std::get<V<V<T>>>(env);
		size_t len_sum = 0;
		for(auto const& v : typed_env) len_sum += v.size();
		return len_sum * sizeof(T);
	}

	template <typename T>
	size_t typedAverageLength() {
		auto const& typed_env = std::get<V<V<T>>>(env);
		size_t len_sum = 0;
		for(auto const& v : typed_env) len_sum = std::max(len_sum, v.size());
		return len_sum ;
	}

	template <typename T>
	size_t typedActualCapacity() {
		auto const& typed_env = std::get<V<V<T>>>(env);
		size_t len_sum = 0;
		for(auto const& v : typed_env) len_sum += v.capacity();
		return len_sum * sizeof(T);
	}

	template <typename T>
	void push_back_action(int i) {
		auto& typed_env = std::get<V<V<T>>>(env);
		if(typed_env.size() == 0) {
			construct_action<T>(i);
			return;
		}
		std::uniform_int_distribution<> q_dist(1, typed_env.size() / 3 + 1);
		std::uniform_int_distribution<> pick_dist(0, typed_env.size()-1);
		std::uniform_int_distribution<> size_dist(1, i * 100);
		int q = q_dist(gen);

		BenchTimer bt("push_back");
		while(q--) {
			int pick = pick_dist(gen);
			int size = size_dist(gen);
			while(size--)
				typed_env[pick].emplace_back();
		}
	}

	template <typename T>
	void pop_back_action(int i) {
		auto& typed_env = std::get<V<V<T>>>(env);
		if(typed_env.size() == 0) {
			construct_action<T>(i);
			return;
		}
		std::uniform_int_distribution<> q_dist(1, typed_env.size() / 3 + 1);
		std::uniform_int_distribution<> pick_dist(0, typed_env.size()-1);
		int q = q_dist(gen);
		
		BenchTimer bt("pop_back");
		while(q--) {
			int pick = pick_dist(gen);
			std::uniform_int_distribution<> size_dist(0, typed_env[pick].size());
			int size = size_dist(gen);
			while(size--)
				typed_env[pick].pop_back();
		}
	}

	template <typename T>
	void construct_action(int i) {
		auto& typed_env = std::get<V<V<T>>>(env);
		std::uniform_int_distribution<> q_dist(1, 3);
		std::uniform_int_distribution<> size_dist(1, i + 10);

		int q = q_dist(gen);
		
		while(q--) {
			int size = size_dist(gen);
			typed_env.emplace_back();
			BenchTimer bt("construct");
			while(size--)
				typed_env.back().emplace_back();
		}
	}

	template <typename T>
	void copy_action(int i) {
		auto& typed_env = std::get<V<V<T>>>(env);
		if(typed_env.size() == 0) {
			construct_action<T>(i);
			return;
		}
		std::uniform_int_distribution<> q_dist(1, 3);
		std::uniform_int_distribution<> pick_dist(0, typed_env.size()-1);
		int q = q_dist(gen);
		
		while(q--) {
			int pick = pick_dist(gen);
			typed_env.emplace_back();
			BenchTimer bt("copy");
			typed_env.back().assign(typed_env[pick].begin(), typed_env[pick].end());
		}
	}

	template <typename T>
	void insert_action(int i) {
		auto& typed_env = std::get<V<V<T>>>(env);
		if(typed_env.size() == 0) {
			construct_action<T>(i);
			return;
		}
		std::uniform_int_distribution<> q_dist(1, typed_env.size() / 3 + 1);
		std::uniform_int_distribution<> pick_dist(0, typed_env.size() - 1);
		std::uniform_int_distribution<> size_dist(1, i);

		int q = q_dist(gen);
		
		BenchTimer bt("insert");
		while(q--) {
			int pick = pick_dist(gen);
			int size = size_dist(gen);

			if(typed_env[pick].size() == 0) {
				typed_env[pick].insert(typed_env[pick].begin(), size, T());
				continue;
			}

			std::uniform_int_distribution<> pos_dist(0, typed_env[pick].size()-1);
			auto pos = typed_env[pick].begin() + pos_dist(gen);
			typed_env[pick].insert(pos, size, T());
		}
	}

	template <typename T>
	void erase_action(int i) {
		auto& typed_env = std::get<V<V<T>>>(env);
		if(typed_env.size() == 0) {
			construct_action<T>(i);
			return;
		}
		std::uniform_int_distribution<> q_dist(1, typed_env.size() / 3 + 1);
		std::uniform_int_distribution<> pick_dist(0, typed_env.size()-1);
		
		int q = q_dist(gen);
		
		BenchTimer bt("erase");
		while(q--) {
			int pick = pick_dist(gen);
			if(typed_env[pick].size() == 0) continue;
			std::uniform_int_distribution<> pos_dist(0, typed_env[pick].size()-1);
			auto pos = typed_env[pick].begin() + pos_dist(gen);
			std::uniform_int_distribution<> size_dist(0, std::distance(pos, typed_env[pick].end()) - 1);
			int size = size_dist(gen);
			if(size == 0) continue;
			typed_env[pick].erase(pos, pos + size);
		}
	}

	constexpr static int actions_num = 6;
	template<typename T>
	void dispatch_action(int i) {
		std::uniform_int_distribution<> action_dist(1, actions_num);
		switch(action_dist(gen)) {
			case 1: push_back_action<T>(i); break;
			case 2: pop_back_action<T>(i); break;
			case 3: construct_action<T>(i); break;
			case 4: copy_action<T>(i); break;
			case 5: insert_action<T>(i); break;
			case 6: erase_action<T>(i); break;
		}
	}

	std::random_device rd;
	std::mt19937 gen;
	std::tuple<V<V<Ts>>...> env;
	int seed;
};

template <template<typename> typename V, typename... Ts>
void experiment(std::string name, int max_it = 1000, int tests = 10) {
	malloc_trim(0);
	BenchTimer::data.resize(max_it / 100);
	for(int seed = 12345512; seed < 12345512 + tests; seed++) {
		VectorEnv<V, Ts...> v_env(seed);
		v_env.RunSimulation(max_it);
		BenchTimer::clear();
	}

	auto data = BenchTimer::data;
	BenchTimer::clear_data();
	if(data.empty()) return ;
	for(size_t i = 0; i < data.size(); ++i) {
		std::cout << name << ": " 
				<< (i+1) * 100 << " iter, "
				<< data[i]["Simulation"] << "s" << std::endl;
	}

	std::ofstream out("data/additional/add2/" + name + ".csv");

	out << "iterations";
	for(auto const& [k, v] : data[0]) {
		(void) v;
		out << "," << k;
	}
	out << std::endl;
	
	for(size_t i = 0; i < data.size(); i++) {
		auto& row = data[i];
		size_t it = (i+1) * 100;
		out << it;
		for(auto const& [k, v] : row) {
			(void) k;
			out << "," << v;
		}
		out << std::endl;
	}
}

using boost_gf = boost::container::growth_factor_100;
using boost_options = boost::container::vector_options_t<boost::container::growth_factor<boost_gf>>;

template<typename T>
using boost_vector = boost::container::vector<T, boost::container::new_allocator<T>, boost_options>; 

template <template<typename> typename V, typename T>
void push_back_bench(std::string name, int it_count = 1000000000) {
	BenchTimer::data.resize(6);
	int e = 0;
	for(int i = 1000; i < it_count; i *= 10) {
		{
			BenchTimer bt("push_back");
			for(int t = 0; t < 10; t++) {
				V<T> v;
				for(int it = 0; it < i; it++)
					v.emplace_back();
			}
		}
		BenchTimer::save_epoch(e++);
		BenchTimer::clear();
	}
	std::ofstream out("data/simple/" + name + ".csv");

	out << "push_back_count,time" << std::endl;
	auto data = BenchTimer::data;
	for(size_t i = 0; i < data.size(); i++) {
		auto& row = data[i];
		size_t it = std::pow(10, (i+3));
		out << it;
		for(auto const& [k, v] : row) {
			(void) k;
			out << "," << v;
		}
		out << std::endl;
	}
	BenchTimer::clear_data();
}

int main()
{
	push_back_bench<rvector, int>("rvector<int>");
	push_back_bench<std::vector, int>("std::vector<int>");
	push_back_bench<folly::fbvector, int>("folly::fbvector<int>");
	push_back_bench<eastl::vector, int>("eastl::vector<int>");
	push_back_bench<boost_vector, int>("boost_vector<int>");

	push_back_bench<rvector, std::string>("rvector<std::string>");
	push_back_bench<std::vector, std::string>("std::vector");
	push_back_bench<folly::fbvector, std::string>("folly::fbvector");
	push_back_bench<eastl::vector, std::string>("eastl::vector");
	push_back_bench<boost_vector, std::string>("boost_vector");

	experiment<std::vector, int>("std::vector<int>", 3000);
	experiment<rvector, int>("rvector<int>", 3000);
	experiment<folly::fbvector, int>("folly::fbvector<int>", 3000);
	experiment<boost_vector, int>("boost_vector<int>", 3000);
	experiment<eastl::vector, int>("eastl::vector<int>", 3000);
	
	experiment<rvector, TestType>("rvector<TestType>", 1500);
	experiment<std::vector, TestType>("std::vector<TestType>", 1500);
	experiment<folly::fbvector, TestType>("folly::fbvector<TestType>", 1500);
	experiment<boost_vector, TestType>("boost_vector<TestType>", 1500);
	experiment<eastl::vector, TestType>("eastl::vector<TestType>", 1500);
	
	experiment<rvector, std::string>("rvector<std::string>", 1200);
	experiment<std::vector, std::string>("std::vector<std::string>", 1200);
	experiment<folly::fbvector, std::string>("folly::fbvector<std::string>", 1200);
	experiment<boost_vector, std::string>("boost_vector<std::string>", 1200);
	experiment<eastl::vector, std::string>("eastl::vector<std::string>", 1200);

	experiment<rvector, std::array<int, 10>>("rvector<std::array<int,10>>", 1200);
	experiment<std::vector, std::array<int, 10>>("std::vector<std::array<int,10>>", 1200);
	experiment<folly::fbvector,  std::array<int, 10>>("folly::fbvector<std::array<int,10>>", 1200);
	experiment<boost_vector,  std::array<int, 10>>("boost_vector<std::array<int,10>>", 1200);
	experiment<eastl::vector,  std::array<int, 10>>("eastl::vector<std::array<int,10>>", 1200);
	
	experiment<std::vector, std::string, int, std::array<int, 10>>("std::vector<std::string, int, std::array<int,10>>", 1000);
	experiment<rvector, std::string, int, std::array<int, 10>>("rvector<std::string, int, std::array<int,10>>", 1000);
	experiment<folly::fbvector, std::string, int, std::array<int, 10>>("folly::fbvector<std::string, int, std::array<int,10>>", 1000);
	experiment<boost_vector, std::string, int, std::array<int, 10>>("boost_vector<std::string, int, std::array<int,10>>", 1000);
	experiment<eastl::vector, std::string, int, std::array<int, 10>>("eastl::vector<std::string, int, std::array<int,10>>", 1000);
}