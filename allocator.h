#pragma once
#include <string.h>
#include <algorithm>
#include <memory>
#include <sys/mman.h>
#include <iostream>



namespace mm
{
	int mremap_skips;
	int grows;
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

	template<typename T>
	using T_Move = std::enable_if_t<std::is_trivially_move_constructible<T>::value>;
	template<typename T>
	using NT_Move = std::enable_if_t<!std::is_trivially_move_constructible<T>::value>;

	template<typename T>
	using T_Move_a = std::enable_if_t<std::is_trivially_move_assignable<T>::value>;
	template<typename T>
	using NT_Move_a = std::enable_if_t<!std::is_trivially_move_assignable<T>::value>;

	using size_type = size_t;
	constexpr size_t map_threshold = 1024;

	template<typename T>
	T* allocate(size_type n)
	{
		if(n > map_threshold)
	    return (T*) mmap(NULL, n*sizeof(T), 
	                PROT_READ | PROT_WRITE,
	                MAP_PRIVATE | MAP_ANONYMOUS,
	                -1, 0);
	    else
	        return (T*) malloc(n*sizeof(T));
	}

	template<typename T>
	void deallocate(T* p, size_type n)
	{
		if(n > map_threshold)
	    	munmap(p, n*sizeof(T));
	    else
	        free(p);
	}


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
		std::fill_n(data, n, value);
	}

	template<typename T>
	NT_Copy<T> fill(T* data, size_type n, const T& value = T())
	{
		std::uninitialized_fill_n(data, n, value);
	}

	template<typename T, typename InputIterator>
	T_Copy<T> fill(T* data, InputIterator begin, InputIterator end)
	{
		std::uninitialized_copy_n(begin, end - begin, data);
	}

	template<typename T, typename InputIterator>
	NT_Copy<T> fill(T* data, InputIterator begin, InputIterator end)
	{
		std::uninitialized_copy_n(begin, end - begin, data);
	}

// fix_capacity

	inline
	size_type fix_capacity(size_type n)
	{
		if(n < map_threshold)
	        return n;
	    return map_threshold * (n/map_threshold + 1);
	}

// realloc
	template<typename T>
	T_Copy<T, T*> realloc_(T* data, size_type length, size_type capacity, size_type n)
	{
		if((n > map_threshold) != (capacity > map_threshold))
	    {
	        T* new_data = allocate<T>(n);
	        memcpy(new_data, data, length * sizeof(T));
	        deallocate(data, capacity);
	        return new_data;
	    }
	    else
	    {
	        if(capacity > map_threshold)
	            return (T*) mremap(data, capacity*sizeof(T), 
	                        n*sizeof(T), MREMAP_MAYMOVE);
	        else
	            return (T*) realloc(data, n*sizeof(T));
	    }
	}

	template<typename T>
	NT_Copy<T, T*> realloc_(T* data, size_type length, size_type capacity, size_type n)
	{
		grows++;
        if(capacity > map_threshold)
        {
            void* new_data = mremap(data, capacity*sizeof(T), 
                        		n*sizeof(T), 0);
            if(new_data != (void*)-1)
            {
            	mremap_skips++;
            	return (T*) new_data;
            }
        }
	    T* new_data = allocate<T>(n);
	    std::uninitialized_move_n(data, length, new_data);
	    destruct(data, data + length);
	    deallocate(data, capacity);
	    return new_data;
	}

// change_capacity
	template<typename T>
	void change_capacity(T*& data, 
						size_type length, 
						size_type& capacity, 
						size_type n)
	{
		size_type new_capacity = fix_capacity(n);
	    if(data)
	        data = realloc_(data, length, capacity, new_capacity);
	    else
	        data = allocate<T>(new_capacity);
	    capacity = new_capacity;
	}

// grow
	template<typename T>
	void grow(T*& data, size_type length, size_type& capacity)
	{
		if(length < capacity) return;
		change_capacity(data, length, capacity, capacity*2 + 1);
	}

// shiftr data
// TODO: check if policies are sufficient
	template<typename T>
	T_Move_a<T> 
	shiftr_data(T* begin, size_type end)
	{
		memmove(begin+1, begin, end * sizeof(T));
	}

	template<typename T>
	NT_Move_a<T> 
	shiftr_data(T* begin, size_type end)
	{
		auto end_p = begin + end;
		while(end_p != begin)
		{
			new (end_p) T(std::move(*(end_p - 1)));
			(--end_p)->~T();
		}
	}
} // namespace mm