#include <vector>
#include <chrono>
#include <random>
#include <iostream>
#include <functional>
#include <map>
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
		durations.clear();
	}

private:
	std::string name;
	std::chrono::time_point<Clock> begin;
	static std::map<std::string, double> durations;
};

std::map<std::string, double> BenchTimer::durations = {};

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
		std::uniform_int_distribution<> size_dist(1, 10000);

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


int main()
{
	test_type<rvector, int>("rvector<int>");
	check_mremap();
	test_type<std::vector, int>("std::vector<int>");
	
	test_type<rvector, TestType>("rvector<TestType>");
	check_mremap();
	test_type<std::vector, TestType>("std::vector<TestType>");
	
	test_type<rvector, std::array<int, 10>>("rvector<std::array<int, 10>>", 500);
	check_mremap();
	test_type<std::vector, std::array<int, 10>>("std::vector<std::array<int, 10>>", 500);
	
	test_type<rvector, std::string, int, std::array<int, 10>>("rvector<std::string, int, std::array<int, 10>>", 500);
	check_mremap();
	test_type<std::vector, std::string, int, std::array<int, 10>>("std::vector<std::string, int, std::array<int, 10>>", 500);
}