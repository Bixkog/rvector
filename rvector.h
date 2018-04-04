#pragma once
// #define _GNU_SOURCE
#include <sys/mman.h> // mremap
#include <iterator>
#include <new>
#include <iostream>
#include <stdexcept>
#include <string.h>

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

    T* allocate(size_type n);
    void deallocate(T* p, size_type n);
    void destruct();

    void fill(size_type n, const T& value = T());
    template <class InputIterator>
    void fill(InputIterator begin, InputIterator end);

    size_type fix_capacity(size_type n);
    void realloc_(size_type n);
    void change_capacity(size_type n);
    void grow();
private:
	T* data_;
	size_type length;
    size_type capacity;
public:
    constexpr static size_t map_threshold = 4096 / sizeof(T);
};
// // private methods
// template <typename T>
// template <class InputIterator>
// void rvector<T>::initialize_array(InputIterator begin, InputIterator end)
// {
//     auto it = data_;
//     while(begin != end)
//         *(it++) = *(begin++);   
// }

// private methods

template <typename T>
T* rvector<T>::allocate(rvector<T>::size_type n)
{
    if(n > map_threshold)
        return (T*) mmap(NULL, n*sizeof(T), 
                    PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS,
                    -1, 0);
    else
        return (T*) malloc(n*sizeof(T));
}

template <typename T>
void rvector<T>::deallocate(T* p, rvector<T>::size_type n)
{
    if(n > map_threshold)
        munmap(p, n*sizeof(T));
    else
        free(p);
}

template <typename T>
void rvector<T>::realloc_(rvector<T>::size_type n)
{

    if((n > map_threshold) != (capacity > map_threshold))
    {
        T* new_data_ = allocate(n);
        memcpy(new_data_, data_, length * sizeof(T));
        deallocate(data_, capacity);
        data_ = new_data_;
    }
    else
    {
        if(capacity > map_threshold)
            data_ = (T*) mremap(data_, capacity*sizeof(T), 
                       n*sizeof(T), MREMAP_MAYMOVE);
        else
            data_ = (T*) realloc(data_, n*sizeof(T));
    }
}

template <typename T>
void rvector<T>::change_capacity(rvector<T>::size_type n)
{
    size_type new_capacity = fix_capacity(n);
    if(data_ == nullptr)
        data_ = allocate(new_capacity);
    else
        realloc_(new_capacity);
    capacity = new_capacity;
}


template<typename T>
void rvector<T>::destruct()
{
    for(auto& e : *this)
            e.~T();
}

template<typename T>
void rvector<T>::fill(rvector<T>::size_type n, const T& value)
{
    for(size_type i = 0; i < n; ++i)
        new (data_ + i) T(value);
}

template <typename T>
template <typename InputIterator>
void rvector<T>::fill(InputIterator begin, InputIterator end)
{
    size_type i = 0;
    while (begin != end)
    {
        new (data_ + i) T(*begin);
        begin++;
        i++;
    }
}

template<typename T>
typename rvector<T>::size_type 
rvector<T>::fix_capacity(rvector<T>::size_type n)
{
    if(n < map_threshold)
        return n;
    return map_threshold * (n/map_threshold + 1);
}

template <typename T>
void rvector<T>::grow()
{
    if(length < capacity) return;
    change_capacity(capacity*2 + 1);
}

// public methods

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
 capacity(fix_capacity(length))
{
    data_ = allocate(capacity);
    fill(length, value);
}

template<typename T>
rvector<T>::rvector(const rvector<T>& other)
 : data_(nullptr),
 length(other.length),
 capacity(other.capacity)
{
    data_ = allocate(capacity);
    fill(other.begin(), other.end());
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
 capacity(fix_capacity(ilist.size()))
{
    data_ = allocate(capacity);
    fill(ilist.begin(), ilist.end());
}

template<typename T>
rvector<T>::~rvector()
{
    destruct();
    deallocate(data_, capacity);
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
        change_capacity(other.capacity);

    // destruct();
    fill(other.begin(), other.end());

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
        change_capacity(ilist.size());

    // destruct();
    fill(ilist.begin(), ilist.end());

    length = ilist.size();


    return *this;
} 

template <typename T>
void rvector<T>::assign(rvector<T>::size_type count, const T& value)
{
    if(count > capacity)
        change_capacity(count);

    // destruct();
    fill(count, value);
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
    change_capacity(n);
}

template <typename T>
void rvector<T>::shrink_to_fit()
{
    if(capacity < map_threshold)
        change_capacity(length);
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
        grow();
    new (data_ + length) T(std::forward<Args>(args)...);
    length++;
}

template <typename T>
void rvector<T>::push_back(const T& x)
{
    grow();
    new (data_ + length) T(x);
    length++;
}

template <typename T>
void rvector<T>::push_back(T&& x)
{
    grow();
    new (data_ + length) T(std::forward<T>(x));
    length++;
}

template <typename T>
void rvector<T>::pop_back()
{
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
    grow();
    iterator position_ = begin() + n;
    memmove(position_+1, position_, (end() - position_) * sizeof(T));
    new (position_) T(std::forward<Args>(args)...);
    length++;
    return position_;
}

