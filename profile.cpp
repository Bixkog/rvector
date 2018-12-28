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
#include <boost/container/stable_vector.hpp>


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
	void clear() {
		data.emplace_back(std::move(durations));
		durations = std::map<std::string, double>();
	}

	static std::vector<std::map<std::string, double>> data;

private:
	std::string name;
	std::chrono::time_point<Clock> begin;
	static std::map<std::string, double> durations;
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

	double RunSimulation(int iter = 1000, std::string name = "Simulation") {
		BenchTimer bt(name);
		while(iter--) {
			(dispatch_action<Ts>(), ...);
		}
		return bt.check();
	}

private:

	template <typename T>
	void push_back_action() {
		BenchTimer bt("push_back");

		auto& typed_env = std::get<V<V<T>>>(env);
		if(typed_env.size() == 0) {
			construct_action<T>();
			return;
		}
		std::uniform_int_distribution<> q_dist(1, typed_env.size() / 3 + 1);
		std::uniform_int_distribution<> pick_dist(0, typed_env.size()-1);
		std::uniform_int_distribution<> size_dist(1, 100000);

		int q = q_dist(gen);
		while(q--) {
			int pick = pick_dist(gen);
			int size = size_dist(gen);
			while(size--)
				typed_env[pick].emplace_back();
		}
	}

	template <typename T>
	void pop_back_action() {
		BenchTimer bt("pop_back");
		auto& typed_env = std::get<V<V<T>>>(env);
		if(typed_env.size() == 0) {
			construct_action<T>();
			return;
		}
		std::uniform_int_distribution<> q_dist(1, typed_env.size() / 3 + 1);
		std::uniform_int_distribution<> pick_dist(0, typed_env.size()-1);
		

		int q = q_dist(gen);
		while(q--) {
			int pick = pick_dist(gen);
			std::uniform_int_distribution<> size_dist(0, typed_env[pick].size());
			int size = size_dist(gen);
			while(size--)
				typed_env[pick].pop_back();
		}
	}

	template <typename T>
	void construct_action() {
		BenchTimer bt("construct");

		auto& typed_env = std::get<V<V<T>>>(env);
		std::uniform_int_distribution<> q_dist(1, 3);
		std::uniform_int_distribution<> size_dist(1, 1000);

		int q = q_dist(gen);
		while(q--) {
			int size = size_dist(gen);
			typed_env.emplace_back();
			while(size--)
				typed_env.back().emplace_back();
		}
	}

	template <typename T>
	void copy_action() {
		BenchTimer bt("copy");

		auto& typed_env = std::get<V<V<T>>>(env);
		if(typed_env.size() == 0) {
			construct_action<T>();
			return;
		}
		std::uniform_int_distribution<> q_dist(1, 3);
		std::uniform_int_distribution<> pick_dist(0, typed_env.size()-1);

		int q = q_dist(gen);
		while(q--) {
			int pick = pick_dist(gen);
			typed_env.emplace_back(typed_env[pick].begin(), typed_env[pick].end());
		}
	}

	template <typename T>
	void insert_action() {
		BenchTimer bt("insert");

		auto& typed_env = std::get<V<V<T>>>(env);
		if(typed_env.size() == 0) {
			construct_action<T>();
			return;
		}
		std::uniform_int_distribution<> q_dist(1, typed_env.size() / 3 + 1);
		std::uniform_int_distribution<> pick_dist(0, typed_env.size() - 1);
		std::uniform_int_distribution<> size_dist(1, 10000);

		int q = q_dist(gen);
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
	void erase_action() {
		BenchTimer bt("erase");

		auto& typed_env = std::get<V<V<T>>>(env);
		if(typed_env.size() == 0) {
			construct_action<T>();
			return;
		}
		std::uniform_int_distribution<> q_dist(1, typed_env.size() / 3 + 1);
		std::uniform_int_distribution<> pick_dist(0, typed_env.size()-1);

		int q = q_dist(gen);
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
	void dispatch_action() {
		std::uniform_int_distribution<> action_dist(1, actions_num);
		switch(action_dist(gen)) {
			case 1: push_back_action<T>(); break;
			case 2: pop_back_action<T>(); break;
			case 3: construct_action<T>(); break;
			case 4: copy_action<T>(); break;
			case 5: insert_action<T>(); break;
			case 6: erase_action<T>(); break;
			default:
				std::cout << "ADD ACTION" << std::endl;
		}
	}

	std::random_device rd;
	std::mt19937 gen;
	std::tuple<V<V<Ts>>...> env;
	int seed;
};

template <template<typename> typename V, typename... Ts>
void test_type(std::string name, int max_it = 500) {
	for(int iter = 100; iter <= max_it; iter += 100) {
		double avg = 0.;
		for(int seed = 12345512; seed < 12345512 + 5; seed++) {
			VectorEnv<V, Ts...> v_env(seed);
			avg += v_env.RunSimulation(iter);
		}
		std::cout << name << ": " << iter << " iter "
					<< avg / 5. << " s" << std::endl;
	}
	BenchTimer::print_data();
	BenchTimer::clear();
}

template <template<typename> typename V, typename... Ts>
void experiment(std::string name, int max_it = 1000, int tests = 10) {
	std::vector<int> it_count;
	std::vector<double> means;
	std::vector<double> variances;
	for(int iter = 100; iter <= max_it; iter += 100) {
		double avg = 0.;
		std::vector<double> test_res;
		for(int seed = 12345512; seed < 12345512 + tests; seed++) {
			VectorEnv<V, Ts...> v_env(seed);
			test_res.push_back(v_env.RunSimulation(iter));
			avg += test_res.back();
		}
		auto extremum = std::max_element(test_res.begin(), test_res.end());
		test_res.erase(extremum);
		avg = (avg - *extremum) / (tests - 1);
		BenchTimer::clear();
		means.push_back(avg);
		it_count.push_back(iter);
		double var = 0.;
		for(double res : test_res) var += (avg - res) * (avg - res);
		variances.push_back(std::sqrt(var / (tests - 1)));
		std::cout << name << ": " 
					<< iter << " iter, "
					<< avg / tests << "s mean, " 
					<< variances.back() << "s stddev" << std::endl;
	}

	auto data = BenchTimer::data;
	if(data.empty()) return;
	BenchTimer::data.clear();

	std::ofstream out("data/" + name + ".csv");

	out << "iterations,mean,variance,";
	for(auto const& [k, v] : data[0]) {
		(void) v;
		out << k << ",";
	}
	out << std::endl;

	for(auto const& [it, mean, var, row] : zip(it_count, means, variances, data)) {
		out << it << "," << mean << "," << var;
		for([[maybe_unused]] auto const& [k, v] : row) {
			(void) k;
			out << "," << v;
		}
		out << std::endl;
	}
}



int main()
{
	experiment<rvector, int>("rvector<int>", 1000);
	experiment<std::vector, int>("std::vector<int>", 1000);
	experiment<folly::fbvector, int>("folly::fbvector<int>", 1000);
	experiment<boost::container::vector, int>("boost::container::vector<int>", 1000);
	experiment<boost::container::stable_vector, int>("boost::container::stable_vector<int>", 1000);
	
	experiment<rvector, TestType>("rvector<TestType>");
	experiment<std::vector, TestType>("std::vector<TestType>");
	experiment<folly::fbvector, TestType>("folly::fbvector<TestType>");
	experiment<boost::container::vector, TestType>("boost::container::vector<TestType>");
	experiment<boost::container::stable_vector, TestType>("boost::container::stable_vector<TestType>");
	
	experiment<rvector, std::array<int, 10>>("rvector<std::array<int,10>>");
	experiment<std::vector, std::array<int, 10>>("std::vector<std::array<int,10>>");
	experiment<folly::fbvector,  std::array<int, 10>>("folly::fbvector<std::array<int,10>>");
	experiment<boost::container::vector,  std::array<int, 10>>("boost::container::vector<std::array<int,10>>");
	experiment<boost::container::stable_vector,  std::array<int, 10>>("boost::container::stable_vector<std::array<int,10>>");
	
	experiment<rvector, std::string, int, std::array<int, 10>>("rvector<std::string, int, std::array<int,10>>", 800);
	experiment<std::vector, std::string, int, std::array<int, 10>>("std::vector<std::string, int, std::array<int,10>>", 800);
	experiment<folly::fbvector, std::string, int, std::array<int, 10>>("folly::fbvector<std::string, int, std::array<int,10>>", 800);
	experiment<boost::container::vector, std::string, int, std::array<int, 10>>("boost::container::vector<std::string, int, std::array<int,10>>", 800);
	experiment<boost::container::stable_vector, std::string, int, std::array<int, 10>>("boost::container::stable_vector<std::string, int, std::array<int,10>>", 800);
}