#pragma once
// #define _GNU_SOURCE
#include <sys/mman.h> // mremap
#include <iterator>
#include <new>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <string.h>
#include <limits>
#include "allocator.h"

#define LIKELY(x)       __builtin_expect((x),1)
#define UNLIKELY(x)     __builtin_expect((x),0)

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
	
	using reference = T&;
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
	rvector(rvector&& other) noexcept;
	rvector(std::initializer_list<T> ilist);

	~rvector();

	rvector& operator =(const rvector& other);
	rvector& operator =(rvector&& other) noexcept;
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
    reference front() noexcept;
    const_reference front() const noexcept;
    reference back() noexcept;
    const_reference back() const noexcept;
 
 //    //data access
    T* data() noexcept;
    const T* data() const noexcept;
 
 //    // modifiers:
    template <class... Args> 
    void emplace_back(Args&&... args);
    template <class... Args> 
    void fast_emplace_back(Args&&... args);

    void push_back(const T& x);
    void fast_push_back(const T& x);
    void push_back(T&& x);
    void fast_push_back(T&& x);

    void pop_back() noexcept;
    void safe_pop_back() noexcept;
 
    template <class... Args> 
    iterator emplace(const_iterator position, Args&&... args);
    iterator insert(iterator position, const T& x);
    iterator insert(iterator position, T&& x);
    iterator insert(iterator position, size_type n, const T& x);
    template <class InputIterator, 
        typename = typename std::iterator_traits<InputIterator>::value_type>
    iterator insert (iterator position, InputIterator first, 
                         InputIterator last);
    iterator insert(iterator position, std::initializer_list<T>);
 
    iterator erase(iterator position);
    iterator erase(iterator first, iterator last);
    void     swap(rvector<T>& other);
    void     clear() noexcept;
private:
	T* data_;
	size_type length_;
    size_type capacity_;
public:
    constexpr static size_t map_threshold = 4096 / sizeof(T);
};

template<typename T>
rvector<T>::rvector() noexcept
 : data_(nullptr),
 length_(0),
 capacity_(0)
{
}

template<typename T>
rvector<T>::rvector(rvector<T>::size_type length)
 : data_(nullptr),
 length_(length),
 capacity_(mm::fix_capacity<T>(length_))
{
    data_ = mm::allocate<T>(capacity_);
    mm::fill(data_, length_);
}

template<typename T>
rvector<T>::rvector(typename rvector<T>::size_type length, const T& value)
 : data_(nullptr),
 length_(length),
 capacity_(mm::fix_capacity<T>(length_))
{
    data_ = mm::allocate<T>(capacity_);
    mm::fill(data_, length_, value);
}

template <typename T>
rvector(typename rvector<T>::size_type length, const T& v) -> rvector<T>;

template <typename T>
template <class InputIterator, typename>
rvector<T>::rvector(InputIterator first, InputIterator last)
 : data_(nullptr),
 length_(std::distance(first, last)),
 capacity_(mm::fix_capacity<T>(length_))
{
    data_ = mm::allocate<T>(capacity_);
    mm::fill(data_, first, last);
}

template <class InputIterator, typename = typename std::iterator_traits<InputIterator>::value_type>
rvector(InputIterator first, InputIterator last) -> 
    rvector<typename std::iterator_traits<InputIterator>::value_type>;

template<typename T>
rvector<T>::rvector(const rvector<T>& other)
 : data_(nullptr),
 length_(other.length_),
 capacity_(other.capacity_)
{
    data_ = mm::allocate<T>(capacity_);
    mm::fill(data_, other.begin(), other.end());
}

template <typename T>
rvector(const rvector<T>& other) -> rvector<T>;

template<typename T>
rvector<T>::rvector(rvector<T>&& other) noexcept
 : data_(other.data_),
 length_(other.length_),
 capacity_(other.capacity_)
{
    other.data_ =  nullptr;
    other.capacity_ = 0;
    other.length_ = 0;
}

template <typename T>
rvector(rvector<T>&& other) -> rvector<T>;

template<typename T>
rvector<T>::rvector(std::initializer_list<T> ilist)
 : data_(nullptr),
 length_(ilist.size()),
 capacity_(mm::fix_capacity<T>(ilist.size()))
{
    data_ = mm::allocate<T>(capacity_);
    mm::fill(data_, ilist.begin(), ilist.end());
}

template<typename T>
rvector(std::initializer_list<T> ilist) -> rvector<T>;

template<typename T>
rvector<T>::~rvector()
{
    mm::destruct(data_, data_ + length_);
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
    return data_ + length_;
}

template <typename T>
typename rvector<T>::const_iterator 
rvector<T>::end() const noexcept
{
    return data_ + length_;
}

template <typename T>
typename rvector<T>::reverse_iterator 
rvector<T>::rbegin() noexcept
{
    return reverse_iterator(data_ + length_);
}

template <typename T>
typename rvector<T>::const_reverse_iterator 
rvector<T>::rbegin() const noexcept
{
    return const_reverse_iterator(data_ + length_);
}

template <typename T>
typename rvector<T>::reverse_iterator 
rvector<T>::rend() noexcept
{
    return reverse_iterator(data_);
}

template <typename T>
typename rvector<T>::const_reverse_iterator 
rvector<T>::rend() const noexcept
{
    return const_reverse_iterator(data_);
}

template <typename T>
typename rvector<T>::const_iterator 
rvector<T>::cbegin() noexcept
{
    return const_iterator(data_);
}

template <typename T>
typename rvector<T>::const_iterator 
rvector<T>::cend() noexcept
{
    return  const_iterator(data_ + length_);
}

template <typename T>
typename rvector<T>::const_reverse_iterator 
rvector<T>::crbegin() const noexcept
{
    return const_reverse_iterator(data_ + length_);
}

template <typename T>
typename rvector<T>::const_reverse_iterator 
rvector<T>::crend() const noexcept
{
    return const_reverse_iterator(data_);
}

template <typename T>
rvector<T>& rvector<T>::operator=(const rvector<T>& other)
{
    if(UNLIKELY(this == std::addressof(other))) return *this;
    if(other.length_ > length_)
        mm::change_capacity(data_, length_, capacity_, other.capacity_);

    mm::destruct(data_, data_ + length_);
    mm::fill(data_, other.begin(), other.end());
    length_ = other.length_;

    return *this;
}

template <typename T>
rvector<T>& rvector<T>::operator=(rvector<T>&& other) noexcept
{
    std::swap(data_, other.data_);
    std::swap(length_, other.length_);
    std::swap(capacity_, other.capacity_);

    return *this;
}
   
template <typename T>
rvector<T>& rvector<T>::operator=(std::initializer_list<T> ilist)
{
    if(ilist.size() > capacity_)
        mm::change_capacity(data_, length_, capacity_, ilist.size());

    mm::destruct(data_, data_ + length_);
    mm::fill(data_, ilist.begin(), ilist.end());
    length_ = ilist.size();
    return *this;
} 

template <typename T>
void rvector<T>::assign(rvector<T>::size_type count, const T& value)
{
    if(count > capacity_)
        mm::change_capacity(data_, length_, capacity_, count);

    mm::destruct(data_, data_ + length_);
    mm::fill(data_, count, value);
    length_ = count;
}
template <typename T>
template <typename InputIt, typename>
void rvector<T>::assign(InputIt first, InputIt last)
{
    size_t count = std::distance(first, last);
    if(count > capacity_)
        mm::change_capacity(data_, length_, capacity_, count);

    mm::destruct(data_, data_ + length_);
    mm::fill(data_, first, last);
    length_ = count;
}

template <typename T>
void rvector<T>::assign(std::initializer_list<T> ilist)
{
    assign(ilist.begin(), ilist.end());
}

template <typename T>
typename rvector<T>::size_type 
rvector<T>::size() const noexcept
{
    return length_;
}

template <typename T>
typename rvector<T>::size_type 
rvector<T>::max_size() const noexcept
{
    return std::numeric_limits<rvector<T>::size_type>::max() / sizeof(T);
}

template <typename T>
void rvector<T>::resize(rvector<T>::size_type size)
{
    if(size > capacity_)
        mm::change_capacity(data_, length_, capacity_, size);        
    if(size < length_)
    { 
        mm::destruct(data_ + size, data_ + length_);
        length_ = size;
    }
    else if(size > length_)
        mm::fill(data_ + length_, size - length_);
    length_ = size;
}

template <typename T>
void rvector<T>::resize(size_type size, const T& c)
{
    if(size > capacity_)
        mm::change_capacity(data_, length_, capacity_, size);        
    if(size < length_)
    { 
        mm::destruct(data_ + size, data_ + length_);
        length_ = size;
    }
    else if(size > length_)
        mm::fill(data_ + length_, size - length_, c);
    length_ = size;
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
    return length_ == 0;
}

template <typename T>
void rvector<T>::reserve(rvector<T>::size_type n)
{
    if(n <= capacity_) return;
    n = n < 2*capacity_? 2*capacity_: n;
    mm::change_capacity(data_, length_, capacity_, n);
}

template <typename T>
void rvector<T>::shrink_to_fit()
{
    if(capacity_ < map_threshold)
        mm::change_capacity(data_, length_, capacity_, length_);
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
    if(UNLIKELY(n >= length_))
        throw std::out_of_range("Index out of range: " + std::to_string(n));
    return data_[n];
}

template <typename T>
typename rvector<T>::const_reference 
rvector<T>::at(rvector<T>::size_type n) const
{
    if(UNLIKELY(n >= length_))
        throw std::out_of_range("Index out of range: " + std::to_string(n));
    return data_[n];
}

template <typename T>
typename rvector<T>::reference 
rvector<T>::front() noexcept
{
    return data_[0];
}

template <typename T>
typename rvector<T>::const_reference 
rvector<T>::front() const noexcept
{
    return data_[0];
}

template <typename T>
inline
typename rvector<T>::reference 
rvector<T>::back() noexcept
{
    return data_[length_ - 1];
}

template <typename T>
inline
typename rvector<T>::const_reference 
rvector<T>::back() const noexcept
{
    return data_[length_ - 1];
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
    mm::grow(data_, length_, capacity_);
    new (data_ + length_) T(std::forward<Args>(args)...);
    ++length_;
}

template <typename T>
template <class... Args> 
void rvector<T>::fast_emplace_back(Args&&... args)
{
    new (data_ + length_) T(std::forward<Args>(args)...);
    ++length_;
}

template <typename T>
void rvector<T>::push_back(const T& x)
{
    mm::grow(data_, length_, capacity_);
    new (data_ + length_) T(x);
    ++length_;
}

template <typename T>
void rvector<T>::fast_push_back(const T& x)
{
    new (data_ + length_) T(x);
    ++length_;
}

template <typename T>
void rvector<T>::push_back(T&& x)
{
    mm::grow(data_, length_, capacity_);
    new (data_ + length_) T(std::forward<T>(x));
    ++length_;
}

template <typename T>
void rvector<T>::fast_push_back(T&& x)
{
    new (data_ + length_) T(std::forward<T>(x));
    ++length_;
}

template <typename T>
void rvector<T>::pop_back() noexcept
{
    if constexpr(!std::is_trivially_destructible_v<T>)
        back().~T();
    --length_;
}

template <typename T>
void rvector<T>::safe_pop_back() noexcept
{
    if(length_ == 0) return;
    if constexpr(!std::is_trivially_destructible_v<T>)
        back().~T();
    --length_;
}

template <typename T>
template <class... Args> 
typename rvector<T>::iterator 
rvector<T>::emplace(rvector<T>::const_iterator position, 
                    Args&&... args)
{
    auto m = std::distance(cbegin(), position);
    mm::grow(data_, length_, capacity_);
    iterator position_ = begin() + m;
    mm::shiftr_data(position_, (end() - position_));
    new (position_) T(std::forward<Args>(args)...);
    ++length_;
    return position_;
}

template <typename T>
typename rvector<T>::iterator 
rvector<T>::insert(rvector<T>::iterator position, const T& x)
{
    auto m = std::distance(begin(), position);
    mm::grow(data_, length_, capacity_);
    position = begin() + m;
    mm::shiftr_data(position, (end() - position));
    new (position) T(x);
    ++length_;
    return position;
}

template <typename T>
typename rvector<T>::iterator 
rvector<T>::insert(rvector<T>::iterator position, T&& x)
{
    auto m = std::distance(begin(), position);
    mm::grow(data_, length_, capacity_);
    position = begin() + m;
    mm::shiftr_data(position, (end() - position));
    new (position) T(std::forward<T>(x));
    ++length_;
    return position;
}
// TODO: add realloc optimalization for insert
template <typename T>
typename rvector<T>::iterator 
rvector<T>::insert(rvector<T>::iterator position, size_type n, const T& x)
{
    if(length_ + n > capacity_)
    {
        auto m = std::distance(begin(), position);
        size_type new_cap = std::max(length_ + n, capacity_ * 2);
        mm::change_capacity(data_, length_, capacity_, new_cap);
        position = begin() + m;
    }
    auto end_ = end();
    size_type rest = std::distance(position, end_);
    if(rest > n) {
        std::uninitialized_move(end_ - n, end_, end_);
        std::move_backward(position, end_ - n, end_);
        std::fill(position, position + n, x);
    } else {
        std::uninitialized_move(position, end_, position + n);
        std::fill(position, end_, x);
        std::uninitialized_fill(end_, position + n, x);
    }
    length_ += n;
    return position;
}

template <typename T>
template <class InputIterator, typename>
typename rvector<T>::iterator 
rvector<T>::insert (rvector<T>::iterator position, InputIterator first, 
                     InputIterator last)
{
    size_type n = std::distance(first, last);
    if(length_ + n > capacity_)
    {
        auto m = std::distance(begin(), position);
        size_type new_cap = std::max(length_ + n, capacity_ * 2);
        mm::change_capacity(data_, length_, capacity_, new_cap);
        position = begin() + m;
    }
    auto end_ = end();
    size_type rest = std::distance(position, end_);
    if(rest > n) {
        std::uninitialized_move(end_ - n, end_, end_);
        std::move_backward(position, end_ - n, end_);
        std::copy(first, last, position);
    } else {
        std::uninitialized_move(position, end_, position + n);
        std::copy(first, first + rest, position);
        std::uninitialized_copy(first + rest, last, end_);
    }
    length_ += n;
    return position;    
}

template <typename T>
typename rvector<T>::iterator 
rvector<T>::insert(rvector<T>::iterator position, std::initializer_list<T> ilist)
{
    auto first = ilist.begin();
    auto last = ilist.end();
    size_type n = std::distance(first, last);
    if(length_ + n > capacity_)
    {
        auto m = std::distance(begin(), position);
        size_type new_cap = std::max(length_ + n, capacity_ * 2);
        mm::change_capacity(data_, length_, capacity_, new_cap);
        position = begin() + m;  
    }
    auto end_ = end();
    size_type rest = std::distance(position, end_);
    if(rest > n) {
        std::uninitialized_move(end_ - n, end_, end_);
        std::move_backward(position, end_ - n, end_);
        std::move(first, last, position);
    } else {
        std::uninitialized_move(position, end_, position + n);
        std::move(first, first + rest, position);
        std::uninitialized_move(first + rest, last, end_);
    }
    length_ += n;
    return position; 
}

template <typename T>
typename rvector<T>::iterator 
rvector<T>::erase(rvector<T>::iterator position)
{
    if (position + 1 != end())
        std::copy(position + 1, end(), position);
    pop_back();
    return position;
}

template <typename T>
typename rvector<T>::iterator 
rvector<T>::erase(rvector<T>::iterator first, rvector<T>::iterator last)
{
    auto n = std::distance(first, last);
    if (last != end())
        std::move(last, end(), first);
    mm::destruct(end() - n, end());
    length_ -= n;
    return first;
}

template <typename T>
void rvector<T>::swap(rvector<T>& other)
{
    using std::swap;
    swap(data_, other.data_);
    swap(length_, other.length_);
    swap(capacity_, other.capacity_);
}

template <typename T>
void rvector<T>::clear() noexcept
{
    mm::destruct(data_, data_ + length_);
    length_ = 0;
}


template <class T>
bool operator==(const rvector<T>& x, const rvector<T>& y)
{
    if(x.size() != y.size()) return false;
    return std::equal(x.begin(), x.end(), y.begin());
}

template <class T>
bool operator< (const rvector<T>& x,const rvector<T>& y)
{
    return std::lexicographical_compare(x.begin(), x.end(), 
                                        y.begin(), y.end());
}

template <class T>
bool operator!=(const rvector<T>& x,const rvector<T>& y)
{
    return !(x == y);
}

template <class T>
bool operator> (const rvector<T>& x,const rvector<T>& y)
{
    return y < x;
}

template <class T>
bool operator>=(const rvector<T>& x,const rvector<T>& y)
{
    return !(x < y);
}

template <class T>
bool operator<=(const rvector<T>& x,const rvector<T>& y)
{
    return !(y < x);
}

template <class T>
void swap(rvector<T>& x, rvector<T>& y)
{
    x.swap(y);
}
