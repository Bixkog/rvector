#pragma once
// #define _GNU_SOURCE
#include <sys/mman.h> // mremap
#include <iterator>
#include <new>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <string.h>

#include "allocator.h"

template <class T> 
class rvector;

// template <class T>
//     bool operator==(const vector<T>& x,const vector<T>& y);
// template <class T>
//     bool operator< (const vector<T>& x,const vector<T>& y);
// template <class T>
//     bool operator!=(const vector<T>& x,const vector<T>& y);
// template <class T>
//     bool operator> (const vector<T>& x,const vector<T>& y);
// template <class T>
//     bool operator>=(const vector<T>& x,const vector<T>& y);
// template <class T>
//     bool operator<=(const vector<T>& x,const vector<T>& y);

// template <class T>
//     void swap(vector<T>& x, vector<T>& y);


template<typename T>
class rvector
{
public:
	using value_type = T;
	using size_type = size_t;
	
	using reference = value_type&;
	using const_reference = const reference;

	using iterator = T*;
	using const_iterator = const T*;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = const reverse_iterator;

	rvector() noexcept;
	explicit rvector(size_type count, const T& value = T());
	rvector(const rvector& other);
	rvector(rvector&& other);
	rvector(std::initializer_list<T> ilist);

	~rvector();

	rvector& operator =(const rvector& other);
	rvector& operator =(rvector&& other);
	rvector& operator =(std::initializer_list<T> ilist);

	void assign(size_type count, const T& value);
	// template<typename InputIt>
	// void assign(InputIt first, InputIt last);
	// void assign(std::initializer_list<T> ilist);

	// T* data() noexcept;
	// const T* data() const noexcept;

	iterator begin() noexcept;
	const_iterator begin() const noexcept;
	iterator end() noexcept;
    const_iterator end() const noexcept;

	// reverse_iterator rbegin() noexcept;
 //    const_reverse_iterator rbegin() const noexcept;
 //    reverse_iterator rend() noexcept;
 //    const_reverse_iterator rend() const noexcept;
 
 //    const_iterator cbegin() noexcept;
 //    const_iterator cend() noexcept;
 //    const_reverse_iterator crbegin() const noexcept;
 //    const_reverse_iterator crend() const noexcept;
 
 //    // capacity:
    size_type size() const noexcept;
    size_type max_size() const noexcept;
 //    void resize(size_type sz);
 //    void resize(size_type sz, const T& c);
 //    size_type capacity() const noexcept;
    bool empty() const noexcept;
    void reserve(size_type n);
    void shrink_to_fit();
 
 //    // element access:
    reference operator[](size_type n);
    const_reference operator[](size_type n) const;
    reference at(size_type n);
    const_reference at(size_type n) const;
    reference front();
    const_reference front() const;
    reference back();
    const_reference back() const;
 
 //    //data access
    T* data() noexcept;
    const T* data() const noexcept;
 
 //    // modifiers:
    template <class... Args> 
    void emplace_back(Args&&... args);
    void push_back(const T& x);
    void push_back(T&& x);
    void pop_back();
 
    template <class... Args> 
    iterator emplace(const_iterator position, Args&&... args);
 //    iterator insert(const_iterator position, const T& x);
 //    iterator insert(const_iterator position, T&& x);
 //    iterator insert(const_iterator position, size_type n, const T& x);
 //    template <class InputIterator>
 //    iterator insert (const_iterator position, InputIterator first, 
 //                         InputIterator last);
 //    iterator insert(const_iterator position, initializer_list<T>);
 
 //    iterator erase(const_iterator position);
 //    iterator erase(const_iterator first, const_iterator last);
 //    void     swap(rvector<T>& other);
 //    void     clear() noexcept;
private:
	T* data_;
	size_type length;
    size_type capacity;
public:
    constexpr static size_t map_threshold = 4096 / sizeof(T);
};

template<typename T>
rvector<T>::rvector() noexcept
 : data_(nullptr),
 length(0),
 capacity(0)
{
}

template<typename T>
rvector<T>::rvector(rvector<T>::size_type length, const T& value)
 : data_(nullptr),
 length(length),
 capacity(mm::fix_capacity(length))
{
    data_ = mm::allocate<T>(capacity);
    mm::fill(data_, length, value);
}

template<typename T>
rvector<T>::rvector(const rvector<T>& other)
 : data_(nullptr),
 length(other.length),
 capacity(other.capacity)
{
    data_ = mm::allocate<T>(capacity);
    mm::fill(data_, other.begin(), other.end());
}

template<typename T>
rvector<T>::rvector(rvector<T>&& other)
 : data_(other.data_),
 length(other.length),
 capacity(other.capacity)
{
    other.data_ =  nullptr;
    other.capacity = 0;
    other.length = 0;
}

template<typename T>
rvector<T>::rvector(std::initializer_list<T> ilist)
 : data_(nullptr),
 length(ilist.size()),
 capacity(mm::fix_capacity(ilist.size()))
{
    data_ = mm::allocate<T>(capacity);
    mm::fill(data_, ilist.begin(), ilist.end());
}

template<typename T>
rvector<T>::~rvector()
{
    mm::destruct(data_, data_ + length);
    mm::deallocate(data_, capacity);
}

template <typename T>
typename rvector<T>::iterator 
rvector<T>::begin() noexcept
{
    return data_;
}

template <typename T>
typename rvector<T>::const_iterator 
rvector<T>::begin() const noexcept
{
    return data_;
}

template <typename T>
typename rvector<T>::iterator 
rvector<T>::end() noexcept
{
    return data_ + length;
}

template <typename T>
typename rvector<T>::const_iterator 
rvector<T>::end() const noexcept
{
    return data_ + length;
}

template <typename T>
rvector<T>& rvector<T>::operator=(const rvector<T>& other)
{
    if(other.length > length)
        mm::change_capacity(data_, length, capacity, other.capacity);

    mm::destruct(data_, data_ + length);
    mm::fill(data_, other.begin(), other.end());
    length = other.length;

    return *this;
}

template <typename T>
rvector<T>& rvector<T>::operator=(rvector<T>&& other)
{
    std::swap(data_, other.data_);
    std::swap(length, other.length);
    std::swap(capacity, other.capacity);

    return *this;
}
   
template <typename T>
rvector<T>& rvector<T>::operator=(std::initializer_list<T> ilist)
{
    if(ilist.size() > capacity)
        mm::change_capacity(data_, length, capacity, ilist.size());

    mm::destruct(data_, data_ + length);
    mm::fill(data_, ilist.begin(), ilist.end());
    length = ilist.size();
    return *this;
} 

template <typename T>
void rvector<T>::assign(rvector<T>::size_type count, const T& value)
{
    if(count > capacity)
        mm::change_capacity(data_, length, capacity, count);

    mm::destruct(data_, data_ + length);
    mm::fill(data_, count, value);
    length = count;
}

template <typename T>
typename rvector<T>::size_type 
rvector<T>::size() const noexcept
{
    return length;
}

template <typename T>
typename rvector<T>::size_type 
rvector<T>::max_size() const noexcept
{
    return capacity;
}

template <typename T>
bool rvector<T>::empty() const noexcept
{
    return length == 0;
}

template <typename T>
void rvector<T>::reserve(rvector<T>::size_type n)
{
    if(n <= capacity) return;
    n = n < 2*capacity? 2*capacity: n;
    mm::change_capacity(data_, length, capacity, n);
}

template <typename T>
void rvector<T>::shrink_to_fit()
{
    if(capacity < map_threshold)
        mm::change_capacity(data_, length, capacity, length);
}

template <typename T>
typename rvector<T>::reference 
rvector<T>::operator[](rvector<T>::size_type n)
{
    return data_[n];
}

template <typename T>
typename rvector<T>::const_reference 
rvector<T>::operator[](rvector<T>::size_type n) const
{
    return data_[n];
}

template <typename T>
typename rvector<T>::reference 
rvector<T>::at(rvector<T>::size_type n)
{
    if(n >= length)
        throw std::out_of_range("Index out of range: " + std::to_string(n));
    return data_[n];
}

template <typename T>
typename rvector<T>::const_reference 
rvector<T>::at(rvector<T>::size_type n) const
{
    if(n >= length)
        throw std::out_of_range("Index out of range: " + std::to_string(n));
    return data_[n];
}

template <typename T>
typename rvector<T>::reference 
rvector<T>::front()
{
    return data_[0];
}

template <typename T>
typename rvector<T>::const_reference 
rvector<T>::front() const
{
    return data_[0];
}

template <typename T>
typename rvector<T>::reference 
rvector<T>::back()
{
    return data_[length - 1];
}

template <typename T>
typename rvector<T>::const_reference 
rvector<T>::back() const
{
    return data_[length - 1];
}

template <typename T>
T* rvector<T>::data() noexcept
{
    return data_;
}

template <typename T>
const T* rvector<T>::data() const noexcept
{
    return data_;
}

template <typename T>
template <class... Args> 
void rvector<T>::emplace_back(Args&&... args)
{
    if(length == capacity)
        mm::grow(data_, length, capacity);
    new (data_ + length) T(std::forward<Args>(args)...);
    length++;
}

template <typename T>
void rvector<T>::push_back(const T& x)
{
    mm::grow(data_, length, capacity);
    new (data_ + length) T(x);
    length++;
}

template <typename T>
void rvector<T>::push_back(T&& x)
{
    mm::grow(data_, length, capacity);
    new (data_ + length) T(std::forward<T>(x));
    length++;
}

template <typename T>
void rvector<T>::pop_back()
{
    if(length == 0) return;
    back().~T();
    length--;
}

template <typename T>
template <class... Args> 
typename rvector<T>::iterator 
rvector<T>::emplace(rvector<T>::const_iterator position, 
                    Args&&... args)
{
    size_type n = position - begin();
    mm::grow(data_, length, capacity);
    iterator position_ = begin() + n;
    mm::shiftr_data(position_, (end() - position_));
    new (position_) T(std::forward<Args>(args)...);
    length++;
    return position_;
}

