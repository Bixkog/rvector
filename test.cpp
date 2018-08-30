#include "rvector.h"
#include <gtest/gtest.h>
#include <string>
#include <boost/preprocessor/repetition/repeat.hpp>
#include "test_type.h"

template<typename T>
T init_value(int i = 0)
{
	return {};
}

template<>
std::string init_value<std::string>(int i)
{
	return "test" + std::to_string(i);
}

template<>
int init_value<int>(int i)
{
	return i;
}

template<>
TestType init_value<TestType>(int i)
{
	return TestType(i);
}


template<typename T>
class rvector_test : public ::testing::Test
{
	void SetUp()
	{
		TestType::aliveObjects = 0;
	}

	void TearDown()
	{
		EXPECT_EQ(TestType::aliveObjects, 0);
		TestType::aliveObjects = 0;
	}
};

const size_t big_size = 4096;


using TestedTypes = ::testing::Types<int, std::string, TestType>;
TYPED_TEST_CASE(rvector_test, TestedTypes);


TYPED_TEST(rvector_test, empty)
{
	rvector<TypeParam> v;
}

TYPED_TEST(rvector_test, length_val_small)
{
	auto val = init_value<TypeParam>();
	rvector<TypeParam> v(5, val);
	for(auto& e : v)
		EXPECT_EQ(e, val);
}

TYPED_TEST(rvector_test, length_val_big)
{
	auto val = init_value<TypeParam>();
	rvector<TypeParam> v(big_size, val);
	for(auto& e : v)
		EXPECT_EQ(e, val);
}

TYPED_TEST(rvector_test, length_small)
{
	rvector<TypeParam> v(5);
	for(auto& e : v)
		EXPECT_EQ(e, TypeParam());	
}

TYPED_TEST(rvector_test, length_big)
{
	rvector<TypeParam> v(big_size);
	for(auto& e : v)
		EXPECT_EQ(e, TypeParam());	
}

TYPED_TEST(rvector_test, iterator_small)
{
	rvector<TypeParam> v(5);
	rvector<TypeParam> v2(v.begin(), v.end());
	for(auto& e : v2)
		EXPECT_EQ(e, TypeParam());
	EXPECT_EQ(v2.size(), 5u);
}

TYPED_TEST(rvector_test, iterator_big)
{
	rvector<TypeParam> v(big_size);
	rvector<TypeParam> v2(v.begin(), v.end());
	for(auto& e : v2)
		EXPECT_EQ(e, TypeParam());
	EXPECT_EQ(v2.size(), big_size);
}

TYPED_TEST(rvector_test, copy_small)
{
	auto val = init_value<TypeParam>();
	rvector<TypeParam> v1(5, val);
	rvector<TypeParam> v2(v1);

	for(auto& e : v2)
		EXPECT_EQ(e, val);
}

TYPED_TEST(rvector_test, copy_big)
{
	auto val = init_value<TypeParam>();
	rvector<TypeParam> v1(big_size, val);
	rvector<TypeParam> v2(v1);

	for(auto& e : v2)
		EXPECT_EQ(e, val);
}

TYPED_TEST(rvector_test, move_small)
{
	auto val = init_value<TypeParam>();
	rvector<TypeParam> v1(5, val);
	rvector<TypeParam> v2(std::move(v1));

	for(auto& e : v2)
		EXPECT_EQ(e, val);
}


TYPED_TEST(rvector_test, move_big)
{
	auto val = init_value<TypeParam>();
	rvector<TypeParam> v1(big_size, val);
	rvector<TypeParam> v2(std::move(v1));

	for(auto& e : v2)
		EXPECT_EQ(e, val);
}

TYPED_TEST(rvector_test, initializer_list_small)
{
	auto val = init_value<TypeParam> ();
	rvector<TypeParam> v = {val, val, val, val, val};

	for(auto& e : v)
		EXPECT_EQ(e, val);
}

#define SINGLE(...) __VA_ARGS__
#define M(z, n, text) ,text
#define COMMA ,

TYPED_TEST(rvector_test, initializer_list_big)
{
	auto val = init_value<TypeParam> ();

	rvector<TypeParam> v = {val BOOST_PP_REPEAT(256, M, val) 
							BOOST_PP_REPEAT(256, M, val)
							BOOST_PP_REPEAT(256, M, val)
							BOOST_PP_REPEAT(256, M, val)};

	for(auto& e : v)
		EXPECT_EQ(e, val);
}

// TODO: test move assign

TYPED_TEST(rvector_test, assign_ss)
{
	auto val1 = init_value<TypeParam> (1);
	auto val2 = init_value<TypeParam> (2);
	rvector<TypeParam> v1(5, val1);
	rvector<TypeParam> v2(6, val2);
	v1 = v2;

	EXPECT_EQ(v1.size(), v2.size());
	EXPECT_EQ(v1.size(), 6u);

	for(auto& e : v1)
		EXPECT_EQ(e, val2);
}

TYPED_TEST(rvector_test, assign_sb)
{
	auto val1 = init_value<TypeParam> (1);
	auto val2 = init_value<TypeParam> (2);	
	rvector<TypeParam> v1(5, val1);
	rvector<TypeParam> v2(big_size, val2);

	v1 = v2;

	EXPECT_EQ(v1.size(), v2.size());
	EXPECT_EQ(v1.size(), big_size);

	for(auto& e : v1)
		EXPECT_EQ(e, val2);
}

TYPED_TEST(rvector_test, assign_bs)
{
	auto val1 = init_value<TypeParam> (1);
	auto val2 = init_value<TypeParam> (2);	
	rvector<TypeParam> v1(big_size, val1);
	rvector<TypeParam> v2(6, val2);

	v1 = v2;

	EXPECT_EQ(v1.size(), v2.size());
	EXPECT_EQ(v1.size(), 6u);

	for(auto& e : v1)
		EXPECT_EQ(e, val2);
}

TYPED_TEST(rvector_test, assign_bb)
{
	auto val1 = init_value<TypeParam> (1);
	auto val2 = init_value<TypeParam> (2);	
	rvector<TypeParam> v1(big_size, val1);
	rvector<TypeParam> v2(big_size + 1, val2);

	v1 = v2;

	EXPECT_EQ(v1.size(), v2.size());
	EXPECT_EQ(v1.size(), big_size + 1u);

	for(auto& e : v1)
		EXPECT_EQ(e, val2);
}

TYPED_TEST(rvector_test, begin_end)
{
	auto val1 = init_value<TypeParam> (1);	
	rvector<TypeParam> v1(big_size, val1);

	size_t c = 0;
	for(auto e: v1)
	{
		(void)e;
		c++;
	}
	EXPECT_EQ(c, big_size);
}

TYPED_TEST(rvector_test, iterators)
{
	rvector<TypeParam> v;
	for(int i = 0; i < 10; i++)
		v.push_back(init_value<TypeParam>(i));

	auto it = v.begin();
	int i = 0;
	while(it != v.end())
	{
		EXPECT_EQ(*it, init_value<TypeParam>(i));
		i++;
		it++;
	}
	EXPECT_EQ(i, 10);

	auto cit = v.cbegin();
	i = 0;
	while(cit != v.cend())
	{
		EXPECT_EQ(*cit, init_value<TypeParam>(i));
		i++;
		cit++;
	}
	EXPECT_EQ(i, 10);

	auto rit = v.rbegin();
	i = 10;
	while(rit != v.rend())
	{
		i--;
		EXPECT_EQ(*rit, init_value<TypeParam>(i));
		rit++;
	}
	EXPECT_EQ(i, 0);

	auto crit = v.crbegin();
	i = 10;
	while(crit != v.crend())
	{
		i--;
		EXPECT_EQ(*crit, init_value<TypeParam>(i));
		crit++;
	}
	EXPECT_EQ(i, 0);
}

TYPED_TEST(rvector_test, inspect)
{
	rvector<TypeParam> v;
	for(int i = 0; i < 100; i++)
		v.push_back(init_value<TypeParam>(i));

	for(size_t i = 0; i < v.size(); ++i)
	{
		EXPECT_EQ(v[i], init_value<TypeParam>(i));
		EXPECT_EQ(v.at(i), init_value<TypeParam>(i));
	}
}

TYPED_TEST(rvector_test, push_pop_emplace)
{

	rvector<TypeParam> v;

	for(size_t i = 0u; i < 127u*2u; ++i)
		v.push_back(init_value<TypeParam>(i));

	EXPECT_EQ(v.size(), 127u*2u);
	EXPECT_GE(v.max_size(), 127u*2u);
	EXPECT_LE(v.max_size(), 127u*4u);
	for(size_t i = 0; i < 127u*2u; ++i)
		EXPECT_EQ(v[i], init_value<TypeParam>(i));

	for(size_t i = 0; i < 127u*2u; ++i)
		v.pop_back();

	EXPECT_EQ(v.size(), 0u);

	for(size_t i = 0; i < 127u*2u; ++i)
		v.emplace_back(init_value<TypeParam>(i));

	for(size_t i = 0; i < 127u*2u; ++i)
		EXPECT_EQ(v[i], init_value<TypeParam>(i));
	EXPECT_EQ(v.size(), 127u*2u);
	EXPECT_GE(v.max_size(), 127u*2u);
	EXPECT_LE(v.max_size(), 127u*4u);
}

TYPED_TEST(rvector_test, emplace_position)
{
	auto val1 = init_value<TypeParam> (1);	
	auto val2 = init_value<TypeParam> (2);	
	rvector<TypeParam> v(5, val1);

	for(auto it = v.end() - 1; it != v.begin(); it--)
		it = v.emplace(it, val2);
	EXPECT_EQ(v.size(), 9u);
	for(size_t i = 0; i < v.size(); ++i)
		EXPECT_EQ(v[i], i%2==0?val1:val2);
}

TYPED_TEST(rvector_test, clear)
{
	rvector<TypeParam> v(big_size);
	v.clear();
	EXPECT_EQ(v.size(), 0u);
}

TYPED_TEST(rvector_test, assign_m)
{
	rvector<TypeParam> v1;
	rvector<TypeParam> v2;

	for(size_t i = 0; i < big_size; i++)
		v2.push_back(init_value<TypeParam>(i));
	v1.assign(v2.begin(), v2.end());

	EXPECT_EQ(v1.size(), big_size);
	for(size_t i = 0; i < big_size; i++)
		EXPECT_EQ(v1[i], init_value<TypeParam>(i));

	auto val = init_value<TypeParam> ();

	v1.assign({val BOOST_PP_REPEAT(256, M, val) 
					BOOST_PP_REPEAT(256, M, val)
					BOOST_PP_REPEAT(256, M, val)
					BOOST_PP_REPEAT(256, M, val)});

	EXPECT_EQ(v1.size(), 1025u);
	for(auto& e : v1)
		EXPECT_EQ(e, val);
}

TYPED_TEST(rvector_test, resize)
{
	rvector<TypeParam> v1;
	v1.resize(100);
	EXPECT_EQ(v1.size(), 100u);
	for(auto& e : v1)
		EXPECT_EQ(e, TypeParam{});

	v1.resize(50);
	EXPECT_EQ(v1.size(), 50u);

	v1.resize(big_size, init_value<TypeParam>(17));
	EXPECT_EQ(v1.size(), big_size);
	for(size_t i = 0; i < big_size; i++)
	{
		if(i < 50) EXPECT_EQ(v1[i], TypeParam{});
		else EXPECT_EQ(v1[i], init_value<TypeParam>(17));
	}

	v1.resize(big_size/2, init_value<TypeParam>(42));
	EXPECT_EQ(v1.size(), big_size/2);
	for(size_t i = 0; i < big_size/2; i++)
	{
		if(i < 50u) EXPECT_EQ(v1[i], TypeParam{});
		else EXPECT_EQ(v1[i], init_value<TypeParam>(17));
	}
}

TYPED_TEST(rvector_test, insert)
{
	auto val1 = init_value<TypeParam> (1);	
	auto val2 = init_value<TypeParam> (2);	
	
	rvector<TypeParam> v(5, val1);
	for(auto it = v.end() - 1; it != v.begin(); it--)
		it = v.insert(it, val2);
	EXPECT_EQ(v.size(), 9u);
	for(size_t i = 0; i < v.size(); ++i)
		EXPECT_EQ(v[i], i%2==0?val1:val2);

	rvector<TypeParam> v2(5, val1);
	for(auto it = v2.end() - 1; it != v2.begin(); it--)
		it = v2.insert(it, init_value<TypeParam> (2));
	EXPECT_EQ(v2.size(), 9u);
	for(size_t i = 0; i < v2.size(); ++i)
		EXPECT_EQ(v2[i], i%2==0?val1:val2);

	rvector<TypeParam> v3(1000, val1);
	v3.insert(v3.begin() + 500, 1000, val2);
	EXPECT_EQ(v3.size(), 2000u);
	for(size_t i = 0; i < v3.size(); i++)
	{
		if(i < 500 or i >= 1500) EXPECT_EQ(v3[i], val1);
		else EXPECT_EQ(v3[i], val2);
	}

	rvector<TypeParam> v4(1000, val1);
	rvector<TypeParam> t;
	for(size_t i = 0; i < 1000; i++) 
		t.push_back(init_value<TypeParam>(i));
	v4.insert(v4.begin() + 500, t.begin(), t.end());
	EXPECT_EQ(v4.size(), 2000u);
	for(size_t i = 0; i < v4.size(); i++)
	{
		if(i < 500 or i >= 1500) EXPECT_EQ(v4[i], val1);
		else EXPECT_EQ(v4[i], init_value<TypeParam>(i-500));
	}

	rvector<TypeParam> v5(1000, val1);
	v5.insert(v5.begin() + 500, {val2 BOOST_PP_REPEAT(256, M, val2) 
					BOOST_PP_REPEAT(256, M, val2)
					BOOST_PP_REPEAT(256, M, val2)
					BOOST_PP_REPEAT(256, M, val2)});
	EXPECT_EQ(v5.size(), 2025u);
	for(size_t i = 0u; i < v5.size(); i++)
	{
		if(i < 500u or i >= 1525u) EXPECT_EQ(v5[i], val1);
		else EXPECT_EQ(v5[i], val2);
	}

	rvector<TypeParam> v6(1000, val1); // BEGIN
	v6.insert(v6.begin(), 1000u, val2);
	EXPECT_EQ(v6.size(), 2000u);
	for(size_t i = 0; i < v6.size(); i++)
	{
		if(i >= 1000u) EXPECT_EQ(v6[i], val1);
		else EXPECT_EQ(v6[i], val2);
	}

	rvector<TypeParam> v7(1000, val1);
	v7.insert(v7.end(), 1000u, val2);
	EXPECT_EQ(v7.size(), 2000u);
	for(size_t i = 0; i < v7.size(); i++)
	{
		if(i < 1000u) EXPECT_EQ(v7[i], val1);
		else EXPECT_EQ(v7[i], val2);
	}
}

TYPED_TEST(rvector_test, erease)
{
	rvector<TypeParam> v;
	for(size_t i = 0; i < big_size; i++)
		v.push_back(init_value<TypeParam>(i));


	v.erase(v.begin() + 50);
	EXPECT_EQ(v.size(), big_size - 1);
	EXPECT_EQ(v[50], init_value<TypeParam>(51));

	v.erase(v.end()-1);
	EXPECT_EQ(v.size(), big_size - 2);
	EXPECT_EQ(v.back(), init_value<TypeParam>(big_size-2));

	v.erase(v.begin());
	EXPECT_EQ(v.size(), big_size - 3);
	EXPECT_EQ(v.front(), init_value<TypeParam>(1));

	v.clear();
	for(size_t i = 0; i < big_size; i++)
		v.push_back(init_value<TypeParam>(i));

	v.erase(v.begin() + 100, v.begin() + 200);
	EXPECT_EQ(v.size(), big_size - 100);
	for(size_t i = 0; i < v.size(); i++)
	{
		if(i < 100) EXPECT_EQ(v[i], init_value<TypeParam>(i));
		else EXPECT_EQ(v[i], init_value<TypeParam>(i+100));
	}

	v.erase(v.begin(), v.begin() + 100);
	EXPECT_EQ(v.size(), big_size - 200);
	for(size_t i = 0; i < v.size(); i++)
		EXPECT_EQ(v[i], init_value<TypeParam>(i + 200));
}

TYPED_TEST(rvector_test, swap)
{
	rvector<TypeParam> v1(100, init_value<TypeParam>(1));
	rvector<TypeParam> v2(200, init_value<TypeParam>(2));
	v1.swap(v2);

	EXPECT_EQ(v1.size(), 200u);
	EXPECT_EQ(v2.size(), 100u);

	for(auto& e : v1) EXPECT_EQ(e, init_value<TypeParam>(2));
	for(auto& e : v2) EXPECT_EQ(e, init_value<TypeParam>(1));
}

TYPED_TEST(rvector_test, equality_relation)
{
	rvector<TypeParam> v(100, init_value<TypeParam>(1));
	
	rvector<TypeParam> v1(v);
	rvector<TypeParam> v2(100, init_value<TypeParam>(1));
	rvector<TypeParam> v3;
	for(int i = 0; i < 100; i++) 
		v3.push_back(init_value<TypeParam>(1));

	EXPECT_TRUE(v == v1);
	EXPECT_TRUE(v == v2);
	EXPECT_TRUE(v == v3);
	EXPECT_TRUE(v1 == v2);
	EXPECT_TRUE(v2 == v3);
	EXPECT_TRUE(v1 == v3);

	rvector<TypeParam> u1(99, init_value<TypeParam>(1));
	rvector<TypeParam> u2(100, init_value<TypeParam>(1));
	u2[33] = init_value<TypeParam>(2);

	EXPECT_TRUE(v != u1);
	EXPECT_TRUE(v != u2);
	EXPECT_TRUE(u1 != u2);
}

TYPED_TEST(rvector_test, ordering_relation)
{
	rvector<TypeParam> v(100, init_value<TypeParam>(1));
	auto v1(v); v1[99] = init_value<TypeParam>(2);
	auto v2(v); v2[98] = init_value<TypeParam>(2);
	auto v3(v); v3[98] = init_value<TypeParam>(3);
	
	EXPECT_TRUE(v <= v and v >= v);
	EXPECT_TRUE(v < v1);
	EXPECT_TRUE(v < v2);
	EXPECT_TRUE(v < v3);
	EXPECT_TRUE(v3 > v);
	EXPECT_TRUE(v2 > v);
	EXPECT_TRUE(v1 > v);
	EXPECT_TRUE(v1 < v2 and v2 < v3 and v1 < v3);
	EXPECT_TRUE(v1 <= v2 and v2 <= v3 and v1 <= v3);
	EXPECT_TRUE(v3 >= v1 and v3 >= v2 and v2 >= v1);
}