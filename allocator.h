#pragma once
#include <string.h>
#include <algorithm>
#include <memory>
#include <linux/mman.h>
#include <iostream>



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

	template<typename T>
	using T_Move = std::enable_if_t<std::is_trivially_move_constructible<T>::value>;
	template<typename T>
	using NT_Move = std::enable_if_t<!std::is_trivially_move_constructible<T>::value>;

	template<typename T>
	using T_Move_a = std::enable_if_t<std::is_trivially_move_assignable<T>::value>;
	template<typename T>
	using NT_Move_a = std::enable_if_t<!std::is_trivially_move_assignable<T>::value>;

	using size_type = size_t;
	template <typename T>
	constexpr size_t map_threshold = 4096 / sizeof(T);

	template<typename T>
	T* allocate(size_type n)
	{
		if(n > map_threshold<T>)
	    {
	    	auto p = (T*) mmap(NULL, n*sizeof(T), 
	                PROT_READ | PROT_WRITE,
	                MAP_PRIVATE | MAP_ANONYMOUS,
	                -1, 0);
	    	// std::cout << "mmap: " << p << std::endl;
	    	return p;
	    }
	    else
        {
        	auto p = (T*) malloc(n*sizeof(T));
        	// std::cout << "malloc: " << p << std::endl;
        	return p;
        }
	}

	template<typename T>
	void deallocate(T* p, size_type n)
	{
		if(n > map_threshold<T>)
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
		std::uninitialized_copy(begin, end, data);
	}

	template<typename T, typename InputIterator>
	NT_Copy<T> fill(T* data, InputIterator begin, InputIterator end)
	{
		std::uninitialized_copy(begin, end, data);
	}

// fix_capacity

	template <typename T>
	size_type fix_capacity(size_type n)
	{
		if(n < map_threshold<T>)
	        return n;
	    return map_threshold<T> * (n/map_threshold<T> + 1);
	}

// realloc
	template<typename T>
	T_Copy<T, T*> realloc_(T* data, 
							size_type length, 
							size_type capacity, 
							size_type n)
	{
		if((n > map_threshold<T>) != (capacity > map_threshold<T>))
	    {
	        T* new_data = allocate<T>(n);
	        memcpy(new_data, data, length * sizeof(T));
	        deallocate(data, capacity);
	        return new_data;
	    }
	    else
	    {
			grows++;
	        if(capacity > map_threshold<T>)
            {
            	// std::cout << "mremap\n";
            	auto p = (T*) mremap(data, capacity*sizeof(T), 
                        n*sizeof(T), MREMAP_MAYMOVE);
            	if(p == data)
	        	// {
        		//  // 	std::cout << "TRIVIAL NOT MOVED\n";
	        		mremap_skips++;
	        	// 	// std::cout << p << " = " << data 
	        	// 	// 		<< " size: " << n*sizeof(T) << std::endl;
	        	// }
	        	// else 
	        	// {
	        	// 	// std::cout << p << " != " << data << std::endl;
	        	// }
	        	return p;
            }
	        else
	        {
    //         	std::cout << "realloc: ";
	        	auto p = (T*) realloc(data, n*sizeof(T));
	        	if(p == data)
	        	// {
            		mremap_skips++;
          // //   		std::cout << p << " = " << data 
          // //   				<< " size: " << n*sizeof(T) <<std::endl;
	        	// }
	        	// else {
          // //   		std::cout << p << " != " << data << std::endl;
	        	// }
	        	return p;
	        }
	    }
	}

	template<typename T>
	NT_Copy<T, T*> realloc_(T* data, 
							size_type length, 
							size_type capacity, 
							size_type n)
	{
		grows++;
        if(capacity > map_threshold<T>)
        {
        	// std::cout << "mremap nontrivial ";
        	// std::cout << data;
            void* new_data = mremap(data, capacity*sizeof(T), 
                        		n*sizeof(T), 0);
            // std::cout << " " << new_data;
            if(new_data != (void*)-1)
            {
            	mremap_skips++;
            	// std::cout << "NON TRIVIAL NOT MOVED\n";
            	return (T*) new_data;
            }
            // std::cout << std::endl;
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
		size_type new_capacity = fix_capacity<T>(n);
		if(new_capacity < map_threshold<T> and capacity > map_threshold<T>)
			return;
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

// TODO: check if policies are sufficient
// shiftr data
	template<typename T>
	T_Move_a<T> 
	shiftr_data(T* begin, size_type end, size_t n = 1)
	{
		memmove(begin + n, begin, end * sizeof(T));
	}

	template<typename T>
	NT_Move_a<T> 
	shiftr_data(T* begin, size_type end, size_t n = 1)
	{
		auto end_p = begin + end + n - 1;
		auto begin_p = begin + n - 1;
		while(end_p != begin_p)
		{
			new (end_p) T(std::move(*(end_p - n)));
			(end_p - n)->~T();
			end_p--;
		}
	}

// shiftl
	template<typename T>
	T_Move_a<T> 
	shiftl_data(T* begin, size_type end, size_t n = 1)
	{
		memmove(begin - n, begin, end * sizeof(T));
	}

	template<typename T>
	NT_Move_a<T> 
	shiftl_data(T* begin, size_type end, size_t n = 1)
	{
		auto end_p = begin + end - n;
		auto begin_p = begin - n;
		while(end_p != begin_p)
		{
			new (begin_p) T(std::move(*(begin_p + n)));
			(begin_p + n)->~T();
			begin_p++;
		}
	}
} // namespace mm