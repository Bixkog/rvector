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
	EXPECT_EQ(v2.size(), 5);
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
	for(auto _: v1)
		c++;
	EXPECT_EQ(c, big_size);
}

TYPED_TEST(rvector_test, inspect)
{
	auto val = init_value<TypeParam> (1);

	rvector<TypeParam> v(100, val);

	for(size_t i = 0; i < v.size(); ++i)
	{
		EXPECT_EQ(v[i], val);
		EXPECT_EQ(v.at(i), val);
	}
}

TYPED_TEST(rvector_test, push_pop_emplace)
{

	rvector<TypeParam> v;

	for(size_t i = 0; i < 127*2; ++i)
		v.push_back(init_value<TypeParam>(i));

	EXPECT_EQ(v.size(), 127*2);
	EXPECT_GE(v.max_size(), 127*2);
	EXPECT_LE(v.max_size(), 127*4);
	for(size_t i = 0; i < 127*2; ++i)
		EXPECT_EQ(v[i], init_value<TypeParam>(i));

	for(size_t i = 0; i < 127*2; ++i)
		v.pop_back();

	EXPECT_EQ(v.size(), 0u);

	for(size_t i = 0; i < 127*2; ++i)
		v.emplace_back(init_value<TypeParam>(i));

	for(size_t i = 0; i < 127*2; ++i)
		EXPECT_EQ(v[i], init_value<TypeParam>(i));
	EXPECT_EQ(v.size(), 127*2);
	EXPECT_GE(v.max_size(), 127*2);
	EXPECT_LE(v.max_size(), 127*4);
}

TYPED_TEST(rvector_test, emplace_position)
{
	auto val1 = init_value<TypeParam> (1);	
	auto val2 = init_value<TypeParam> (2);	
	rvector<TypeParam> v(5, val1);

	for(auto it = v.end() - 1; it != v.begin(); it--)
		it = v.emplace(it, val2);
	for(size_t i = 0; i < v.size(); ++i)
		EXPECT_EQ(v[i], i%2==0?val1:val2);
}

TYPED_TEST(rvector_test, clear)
{
	rvector<TypeParam> v(big_size);
	v.clear();
	EXPECT_EQ(v.size(), 0);
}