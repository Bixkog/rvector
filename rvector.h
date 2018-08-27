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

template <class T>
    bool operator==(const rvector<T>& x,const rvector<T>& y);
template <class T>
    bool operator< (const rvector<T>& x,const rvector<T>& y);
template <class T>
    bool operator!=(const rvector<T>& x,const rvector<T>& y);
template <class T>
    bool operator> (const rvector<T>& x,const rvector<T>& y);
template <class T>
    bool operator>=(const rvector<T>& x,const rvector<T>& y);
template <class T>
    bool operator<=(const rvector<T>& x,const rvector<T>& y);

template <class T>
    void swap(rvector<T>& x, rvector<T>& y);

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
    explicit rvector(size_type count);
	explicit rvector(size_type count, const T& value);
    template <class InputIterator, 
        typename = typename std::iterator_traits<InputIterator>::value_type>
    rvector (InputIterator first, InputIterator last);
	rvector(const rvector& other);
	rvector(rvector&& other);
	rvector(std::initializer_list<T> ilist);

	~rvector();

	rvector& operator =(const rvector& other);
	rvector& operator =(rvector&& other);
	rvector& operator =(std::initializer_list<T> ilist);

	void assign(size_type count, const T& value);
	template<typename InputIt, 
        typename = typename std::iterator_traits<InputIt>::value_type>
	void assign(InputIt first, InputIt last);
	void assign(std::initializer_list<T> ilist);

	iterator begin() noexcept;
	const_iterator begin() const noexcept;
	iterator end() noexcept;
    const_iterator end() const noexcept;

	reverse_iterator rbegin() noexcept;
    const_reverse_iterator rbegin() const noexcept;
    reverse_iterator rend() noexcept;
    const_reverse_iterator rend() const noexcept;
 
    const_iterator cbegin() noexcept;
    const_iterator cend() noexcept;
    const_reverse_iterator crbegin() const noexcept;
    const_reverse_iterator crend() const noexcept;
 
 //    // capacity:
    size_type size() const noexcept;
    size_type max_size() const noexcept;
    void resize(size_type sz);
    void resize(size_type sz, const T& c);
    size_type capacity() const noexcept;
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
    iterator insert(const_iterator position, const T& x);
    iterator insert(const_iterator position, T&& x);
    iterator insert(const_iterator position, size_type n, const T& x);
    template <class InputIterator, 
        typename = typename std::iterator_traits<InputIterator>::value_type>
    iterator insert (const_iterator position, InputIterator first, 
                         InputIterator last);
    iterator insert(const_iterator position, std::initializer_list<T>);
 
    iterator erase(const_iterator position);
    iterator erase(const_iterator first, const_iterator last);
    void     swap(rvector<T>& other);
    void     clear() noexcept;
private:
	T* data_;
	size_type length;
    size_type capacity_;
public:
    constexpr static size_t map_threshold = 4096 / sizeof(T);
};

template<typename T>
rvector<T>::rvector() noexcept
 : data_(nullptr),
 length(0),
 capacity_(0)
{
}

template<typename T>
rvector<T>::rvector(rvector<T>::size_type length)
 : data_(nullptr),
 length(length),
 capacity_(mm::fix_capacity<T>(length))
{
    data_ = mm::allocate<T>(capacity_);
    mm::fill(data_, length);
}

template<typename T>
rvector<T>::rvector(rvector<T>::size_type length, const T& value)
 : data_(nullptr),
 length(length),
 capacity_(mm::fix_capacity<T>(length))
{
    data_ = mm::allocate<T>(capacity_);
    mm::fill(data_, length, value);
}

template <typename T>
template <class InputIterator, typename>
rvector<T>::rvector (InputIterator first, InputIterator last)
 : data_(nullptr),
 length(std::distance(first, last)),
 capacity_(mm::fix_capacity<T>(length))
{
    data_ = mm::allocate<T>(capacity_);
    mm::fill(data_, first, last);
}

template<typename T>
rvector<T>::rvector(const rvector<T>& other)
 : data_(nullptr),
 length(other.length),
 capacity_(other.capacity_)
{
    data_ = mm::allocate<T>(capacity_);
    mm::fill(data_, other.begin(), other.end());
}

template<typename T>
rvector<T>::rvector(rvector<T>&& other)
 : data_(other.data_),
 length(other.length),
 capacity_(other.capacity_)
{
    other.data_ =  nullptr;
    other.capacity_ = 0;
    other.length = 0;
}

template<typename T>
rvector<T>::rvector(std::initializer_list<T> ilist)
 : data_(nullptr),
 length(ilist.size()),
 capacity_(mm::fix_capacity<T>(ilist.size()))
{
    data_ = mm::allocate<T>(capacity_);
    mm::fill(data_, ilist.begin(), ilist.end());
}

template<typename T>
rvector<T>::~rvector()
{
    mm::destruct(data_, data_ + length);
    mm::deallocate(data_, capacity_);
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
typename rvector<T>::reverse_iterator 
rvector<T>::rbegin() noexcept // TEST
{
    return data_ + length - 1;
}

template <typename T>
typename rvector<T>::const_reverse_iterator 
rvector<T>::rbegin() const noexcept // TEST
{
    return data_ + length - 1;
}

template <typename T>
typename rvector<T>::reverse_iterator 
rvector<T>::rend() noexcept // TEST
{
    return data_ - 1;
}

template <typename T>
typename rvector<T>::const_reverse_iterator 
rvector<T>::rend() const noexcept // TEST
{
    return data_ - 1;
}

template <typename T>
typename rvector<T>::const_iterator 
rvector<T>::cbegin() noexcept // TEST
{
    return data_;
}

template <typename T>
typename rvector<T>::const_iterator 
rvector<T>::cend() noexcept // TEST
{
    return data_ + length;
}

template <typename T>
typename rvector<T>::const_reverse_iterator 
rvector<T>::crbegin() const noexcept // TEST
{
    return data_ + length - 1;
}

template <typename T>
typename rvector<T>::const_reverse_iterator 
rvector<T>::crend() const noexcept // TEST
{
    return data_ - 1;
}

template <typename T>
rvector<T>& rvector<T>::operator=(const rvector<T>& other)
{
    if(other.length > length)
        mm::change_capacity(data_, length, capacity_, other.capacity_);

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
    std::swap(capacity_, other.capacity_);

    return *this;
}
   
template <typename T>
rvector<T>& rvector<T>::operator=(std::initializer_list<T> ilist)
{
    if(ilist.size() > capacity_)
        mm::change_capacity(data_, length, capacity_, ilist.size());

    mm::destruct(data_, data_ + length);
    mm::fill(data_, ilist.begin(), ilist.end());
    length = ilist.size();
    return *this;
} 

template <typename T>
void rvector<T>::assign(rvector<T>::size_type count, const T& value)
{
    if(count > capacity_)
        mm::change_capacity(data_, length, capacity_, count);

    mm::destruct(data_, data_ + length);
    mm::fill(data_, count, value);
    length = count;
}
template <typename T>
template <typename InputIt, typename>
void rvector<T>::assign(InputIt first, InputIt last) // TEST
{
    auto count = std::distance(first, last);
    if(count > capacity_)
        mm::change_capacity(data_, length, capacity_, count);

    mm::destruct(data_, data_ + length);
    mm::fill(data_, first, last);
    length = count;
}

template <typename T>
void rvector<T>::assign(std::initializer_list<T> ilist) // TEST
{
    assign(ilist.begin(), ilist.end());
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
    return capacity_;
}

template <typename T>
void rvector<T>::resize(rvector<T>::size_type size) // TEST
{
    if(size > capacity_)
        mm::change_capacity(data_, length, capacity_, size);        
    if(size < length)
    { 
        mm::destruct(data_ + size, data_ + length);
        length = size;
    }
    else if(size > length)
        mm::fill(data_ + length, size - length);
}
template <typename T>
void rvector<T>::resize(size_type sz, const T& c) // TEST
{
    if(size > capacity_)
        mm::change_capacity(data_, length, capacity_, size);        
    if(size < length)
    { 
        mm::destruct(data_ + size, data_ + length);
        length = size;
    }
    else if(size > length)
        mm::fill(data_ + length, size - length, c);
}

template <typename T>
typename rvector<T>::size_type 
rvector<T>::capacity() const noexcept
{
    return capacity_;
}


template <typename T>
bool rvector<T>::empty() const noexcept
{
    return length == 0;
}

template <typename T>
void rvector<T>::reserve(rvector<T>::size_type n)
{
    if(n <= capacity_) return;
    n = n < 2*capacity_? 2*capacity_: n;
    mm::change_capacity(data_, length, capacity_, n);
}

template <typename T>
void rvector<T>::shrink_to_fit()
{
    if(capacity_ < map_threshold)
        mm::change_capacity(data_, length, capacity_, length);
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
    mm::grow(data_, length, capacity_);
    new (data_ + length) T(std::forward<Args>(args)...);
    length++;
}

template <typename T>
void rvector<T>::push_back(const T& x)
{
    mm::grow(data_, length, capacity_);
    new (data_ + length) T(x);
    length++;
}

template <typename T>
void rvector<T>::push_back(T&& x)
{
    mm::grow(data_, length, capacity_);
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
    mm::grow(data_, length, capacity_);
    iterator position_ = begin() + n;
    mm::shiftr_data(position_, (end() - position_));
    new (position_) T(std::forward<Args>(args)...);
    length++;
    return position_;
}

template <typename T>
typename rvector<T>::iterator 
rvector<T>::insert(rvector<T>::const_iterator position, const T& x) // TEST
{
    size_type n = position - begin();
    mm::grow(data_, length, capacity_);
    iterator position_ = begin() + n;
    mm::shiftr_data(position_, (end() - position_));
    new (position_) T(x);
    length++;
    return position_;
}

template <typename T>
typename rvector<T>::iterator 
rvector<T>::insert(rvector<T>::const_iterator position, T&& x) // TEST
{
    size_type n = position - begin();
    mm::grow(data_, length, capacity_);
    iterator position_ = begin() + n;
    mm::shiftr_data(position_, (end() - position_));
    new (position_) T(std::forward(x));
    length++;
    return position_;
}

template <typename T>
typename rvector<T>::iterator 
rvector<T>::insert(rvector<T>::const_iterator position, size_type n, const T& x) // TEST
{
    size_type m = position - begin();
    if(length + n > capacity_)
    {
        size_type new_cap = std::max(length+n, capacity_*2 + 1);
        mm::change_capacity(data_, length, capacity_, new_cap);    
    }
    iterator position_ = begin() + m;
    mm::shiftr_data(position_, (end() - position_), n);
    mm::fill(position_, n, x);
    length += n;
    return position_;
}

template <typename T>
template <class InputIterator, typename>
typename rvector<T>::iterator 
rvector<T>::insert (rvector<T>::const_iterator position, InputIterator first, 
                     InputIterator last) // TEST
{
    size_type m = position - begin();
    size_type n = std::distance(first, last);
    if(length + n > capacity_)
    {
        size_type new_cap = std::max(length+n, capacity_*2 + 1);
        mm::change_capacity(data_, length, capacity_, new_cap);    
    }
    iterator position_ = begin() + m;
    mm::shiftr_data(position_, (end() - position_), n);
    mm::fill(position_, first, last);
    length += n;
    return position_;    
}

template <typename T>
typename rvector<T>::iterator 
rvector<T>::insert(rvector<T>::const_iterator position, std::initializer_list<T> ilist)
{
    size_type m = position - begin();
    auto first = ilist.begin();
    auto last = ilist.end();
    size_type n = std::distance(first, last);
    if(length + n > capacity_)
    {
        size_type new_cap = std::max(length+n, capacity_*2 + 1);
        mm::change_capacity(data_, length, capacity_, new_cap);    
    }
    iterator position_ = begin() + m;
    mm::shiftr_data(position_, (end() - position_), n);
    mm::fill(position_, first, last);
    length += n;
    return position_; 
}

template <typename T>
typename rvector<T>::iterator 
rvector<T>::erase(rvector<T>::const_iterator position) // TEST
{
    position->~T();
    mm::shiftl_data(position+1, end());
    length--;
}

template <typename T>
typename rvector<T>::iterator 
rvector<T>::erase(rvector<T>::const_iterator first, rvector<T>::const_iterator last) // TEST
{
    mm::destruct(first, last);
    size_type n = std::distance(first, last);
    mm::shiftl_data(last, end(), n);
    length -= n;
}

template <typename T>
void rvector<T>::swap(rvector<T>& other) // TEST
{
    std::swap(data_, other.data_);
    std::swap(length, other.length);
    std::swap(capacity_, other.capacity_);
}

template <typename T>
void rvector<T>::clear() noexcept
{
    mm::destruct(data_, data_ + length);
    length = 0;
}


template <class T>
bool operator==(const rvector<T>& x, const rvector<T>& y) // TEST
{
    if(x.size() != y.size()) return false;
    for(size_t i = 0; i < x.size(); i++)
        if(x[i] != y[i]) return false;
    return true;
}

template <class T>
bool operator< (const rvector<T>& x,const rvector<T>& y) // TEST
{
    return std::lexicographical_compare(x.begin(), x.end(), 
                                        y.begin(), y.end());
}

template <class T>
bool operator!=(const rvector<T>& x,const rvector<T>& y) // TEST
{
    return !(x == y);
}

template <class T>
bool operator> (const rvector<T>& x,const rvector<T>& y) // TEST
{
    return y < x;
}

template <class T>
bool operator>=(const rvector<T>& x,const rvector<T>& y) // TEST
{
    return !(x < y);
}

template <class T>
bool operator<=(const rvector<T>& x,const rvector<T>& y) // TEST
{
    return !(y < x);
}

template <class T>
void swap(rvector<T>& x, rvector<T>& y) // TEST
{
    x.swap(y);
}
