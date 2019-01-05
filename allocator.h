#pragma once
#include <string.h>
#include <algorithm>
#include <memory>
#include <linux/mman.h>
#include <iostream>

#define LIKELY(x)       __builtin_expect((x),1)
#define UNLIKELY(x)     __builtin_expect((x),0)

namespace mm
{
	int mremap_skips = 0;
	int grows = 0;
	// Policies
	template<typename T>
	using Trivial = std::enable_if_t<std::is_trivial<T>::value>;

	template<typename T>
	using T_Destr = std::enable_if_t<std::is_trivially_destructible<T>::value>;
	template<typename T>
	using NT_Destr = std::enable_if_t<!std::is_trivially_destructible<T>::value>;

	template<typename T, typename R = void>
	using T_Copy = std::enable_if_t<std::is_trivially_copy_constructible<T>::value, R>;
	template<typename T, typename R = void>
	using NT_Copy = std::enable_if_t<!std::is_trivially_copy_constructible<T>::value, R>;

	template<typename T, typename R = void>
	using T_Move = std::enable_if_t<std::is_trivially_move_constructible<T>::value, R>;
	template<typename T, typename R = void>
	using NT_Move = std::enable_if_t<!std::is_trivially_move_constructible<T>::value, R>;

	template<typename T>
	using T_Move_a = std::enable_if_t<std::is_trivially_move_assignable<T>::value>;
	template<typename T>
	using NT_Move_a = std::enable_if_t<!std::is_trivially_move_assignable<T>::value>;

	using size_type = size_t;
	
// destruct
	template<typename T>
	NT_Destr<T>
	destruct(T* begin, T* end)
	{
		std::destroy(begin, end);
	}

	template<typename T>
	T_Destr<T>
	destruct(T*, T*)
	{
	}

// fill
	template<typename T>
	T_Copy<T> fill(T* data, size_type n, const T& value = T())
	{
		std::uninitialized_fill_n(data, n, value);
	}

	template<typename T>
	NT_Copy<T> fill(T* data, size_type n, const T& value = T())
	{
		std::uninitialized_fill_n(data, n, value);
	}

	template<typename T, typename InputIterator>
	T_Copy<T> fill(T* data, InputIterator begin, InputIterator end)
	{
		memcpy(data, &*begin, (end - begin) * sizeof(T)); 
	}

	template<typename T, typename InputIterator>
	NT_Copy<T> fill(T* data, InputIterator begin, InputIterator end)
	{
		std::uninitialized_copy(begin, end, data);
	}

// TODO: check if policies are sufficient
// shiftr data
	template<typename T>
	T_Move_a<T> 
	shiftr_data(T* begin, size_type end)
	{
		memmove(begin + 1, begin, end * sizeof(T));
	}

	template<typename T>
	NT_Move_a<T> 
	shiftr_data(T* begin, size_type end)
	{
		auto end_p = begin + end;
		new (end_p) T(std::move(*(end_p - 1)));
		std::move_backward(begin, end_p - 1, end_p);
		begin->~T();
	}
} // namespace mm