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

template<typename T, typename R = void>
using T_Copy = std::enable_if_t<std::is_trivially_copy_constructible<T>::value, R>;
template<typename T, typename R = void>
using NT_Copy = std::enable_if_t<!std::is_trivially_copy_constructible<T>::value, R>;

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

    void set_ptrs(size_type size_, size_type capacity_) noexcept;
    template<typename = T_Copy<T>>
    T* realloc_(size_type n) const;
    template<typename = NT_Copy<T>, typename=void>
    T* realloc_(size_type n) const;

    

    void change_capacity(size_type n)
    {
        size_type new_capacity = mm::fix_capacity<T>(n);
        size_type size_ = size();
        size_type capacity_ = capacity();

        if(new_capacity < map_threshold and 
            capacity_ > map_threshold)
            return;

        if(begin_)
            begin_ = realloc_(new_capacity);
        else
            begin_ = mm::allocate<T>(new_capacity);
        end_ = begin_ + size_;
        cap_end_ = begin_ + capacity_;
    }

// grow
    void grow()
    {
        if(end_ < cap_end_) return;
        change_capacity(capacity()*2 + 1);
    }

private:
	T* begin_;
	T* end_;
    T* cap_end_;
public:
    constexpr static size_t map_threshold = 4096 / sizeof(T);
};

template <typename T>
inline
void rvector<T>::set_ptrs(size_type size_, size_type capacity_) noexcept
{
    begin_ = mm::allocate<T>(capacity_);
    end_ = begin_ + size_;
    cap_end_ = begin_ + capacity_;
}

template <typename T>
template <typename>
T* rvector<T>::realloc_(size_type n) const
{
    if((n > map_threshold) != (capacity() > map_threshold))
    {
        T* new_data = mm::allocate<T>(n);
        memcpy(new_data, begin_, size() * sizeof(T));
        mm::deallocate(begin_, capacity());
        return new_data;
    }
    else
    {
        if(capacity() > map_threshold)
            return (T*) mremap(begin_, capacity()*sizeof(T), 
                                n*sizeof(T), MREMAP_MAYMOVE);
        else
            return (T*) realloc(begin_, n*sizeof(T));
    }
}

template <typename T>
template <typename, typename>
T* rvector<T>::realloc_(size_type n) const
{
    if(capacity() > map_threshold)
    {
        void* new_data = mremap(begin_, capacity()*sizeof(T), 
                            n*sizeof(T), 0);
        if(new_data != (void*)-1)
        {
            return (T*) new_data;
        }
    }
    T* new_data = mm::allocate<T>(n);
    std::uninitialized_move(begin_, end_, new_data);
    mm::destruct(begin_, end_);
    mm::deallocate(begin_, capacity());
    return new_data;
}


template<typename T>
rvector<T>::rvector() noexcept
  : begin_(nullptr),
    end_(nullptr),
    cap_end_(nullptr)
{
}

template<typename T>
rvector<T>::rvector(rvector<T>::size_type size_)
{
    size_type capacity_ = mm::fix_capacity<T>(size_);
    this->set_ptrs(size_, capacity_);
    mm::fill(begin_, end_);
}

template<typename T>
rvector<T>::rvector(typename rvector<T>::size_type size_, const T& value)
{
    size_type capacity_ = mm::fix_capacity<T>(size_);

    this->set_ptrs(size_, capacity_);
    mm::fill(begin_, end_, value);
}

template <typename T>
rvector(typename rvector<T>::size_type size_, const T& v) -> rvector<T>;

template <typename T>
template <class InputIterator, typename>
rvector<T>::rvector(InputIterator first, InputIterator last)
{
    size_type size_ = std::distance(first, last);
    size_type capacity_ = mm::fix_capacity<T>(size());

    this->set_ptrs(size_, capacity_);
    mm::fill(begin_, first, last);
}

template <class InputIterator, typename = typename std::iterator_traits<InputIterator>::value_type>
rvector(InputIterator first, InputIterator last) -> 
    rvector<typename std::iterator_traits<InputIterator>::value_type>;

template<typename T>
rvector<T>::rvector(const rvector<T>& other)
{
    size_type size_ = std::distance(other.begin(), other.end());
    size_type capacity_ = mm::fix_capacity<T>(size());

    this->set_ptrs(size_, capacity_);
    mm::fill(begin_, other.begin(), other.end());
}

template <typename T>
rvector(const rvector<T>& other) -> rvector<T>;

template<typename T>
rvector<T>::rvector(rvector<T>&& other) noexcept
 : begin_(other.begin_),
 end_(other.end_),
 cap_end_(other.cap_end_)
{
    other.begin_ =  nullptr;
    other.end_ = nullptr;
    other.cap_end_ = nullptr;
}

template <typename T>
rvector(rvector<T>&& other) -> rvector<T>;

template<typename T>
rvector<T>::rvector(std::initializer_list<T> ilist)
{
    size_type size_ = std::distance(ilist.begin(), ilist.end());
    size_type capacity_ = mm::fix_capacity<T>(size());

    this->set_ptrs(size_, capacity_);
    mm::fill(begin_, ilist.begin(), ilist.end());
}

template<typename T>
rvector(std::initializer_list<T> ilist) -> rvector<T>;

template<typename T>
rvector<T>::~rvector()
{
    mm::destruct(begin_, end_);
    mm::deallocate(begin_, capacity());
}

template <typename T>
inline
typename rvector<T>::iterator 
rvector<T>::begin() noexcept
{
    return begin_;
}

template <typename T>
inline
typename rvector<T>::const_iterator 
rvector<T>::begin() const noexcept
{
    return begin_;
}

template <typename T>
inline
typename rvector<T>::iterator 
rvector<T>::end() noexcept
{
    return end_;
}

template <typename T>
inline
typename rvector<T>::const_iterator 
rvector<T>::end() const noexcept
{
    return end_;
}

template <typename T>
inline
typename rvector<T>::reverse_iterator 
rvector<T>::rbegin() noexcept
{
    return reverse_iterator(end_);
}

template <typename T>
inline
typename rvector<T>::const_reverse_iterator 
rvector<T>::rbegin() const noexcept
{
    return const_reverse_iterator(end_);
}

template <typename T>
inline
typename rvector<T>::reverse_iterator 
rvector<T>::rend() noexcept
{
    return reverse_iterator(begin_);
}

template <typename T>
inline
typename rvector<T>::const_reverse_iterator 
rvector<T>::rend() const noexcept
{
    return const_reverse_iterator(begin_);
}

template <typename T>
inline
typename rvector<T>::const_iterator 
rvector<T>::cbegin() noexcept
{
    return const_iterator(begin_);
}

template <typename T>
inline
typename rvector<T>::const_iterator 
rvector<T>::cend() noexcept
{
    return  const_iterator(end_);
}

template <typename T>
inline
typename rvector<T>::const_reverse_iterator 
rvector<T>::crbegin() const noexcept
{
    return const_reverse_iterator(end_);
}

template <typename T>
inline
typename rvector<T>::const_reverse_iterator 
rvector<T>::crend() const noexcept
{
    return const_reverse_iterator(begin_);
}

template <typename T>
rvector<T>& rvector<T>::operator=(const rvector<T>& other)
{
    if(UNLIKELY(this == std::addressof(other))) return *this;
    if(other.size() > size())
        change_capacity(other.capacity());

    mm::destruct(begin_, end_);
    mm::fill(begin_, other.begin(), other.end());
    end_ = begin_ + other.size();

    return *this;
}

template <typename T>
rvector<T>& rvector<T>::operator=(rvector<T>&& other)
{
    std::swap(begin_, other.begin_);
    std::swap(end_, other.end_);
    std::swap(cap_end_, other.cap_end_);

    return *this;
}
   
template <typename T>
rvector<T>& rvector<T>::operator=(std::initializer_list<T> ilist)
{
    if(ilist.size() > capacity())
        change_capacity(ilist.size());

    mm::destruct(begin_, end_);
    mm::fill(begin_, ilist.begin(), ilist.end());
    end_ = begin_ + ilist.size();

    return *this;
} 

template <typename T>
void rvector<T>::assign(rvector<T>::size_type count, const T& value)
{
    if(count > capacity())
        change_capacity(count);

    mm::destruct(begin_, end_);
    mm::fill(begin_, count, value);
    end_ = begin_ + count;
}

template <typename T>
template <typename InputIt, typename>
void rvector<T>::assign(InputIt first, InputIt last)
{
    size_t count = std::distance(first, last);
    if(count > capacity())
        change_capacity(count);

    mm::destruct(begin_, end_);
    mm::fill(begin_, first, last);
    end_ = begin_ + count;
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
    return std::distance(begin_, end_);
}

template <typename T>
typename rvector<T>::size_type 
rvector<T>::max_size() const noexcept
{
    return std::numeric_limits<rvector<T>::size_type>::max() / sizeof(T);
}

template <typename T>
void rvector<T>::resize(rvector<T>::size_type size_)
{
    if(size_ > capacity())
        change_capacity(size_);        
    if(size_ < size())
        mm::destruct(begin_ + size_, end_);
    else if(size_ > size())
        mm::fill(end_, end_ + size_ - size());
    end_ = begin_ + size_;
}

template <typename T>
void rvector<T>::resize(size_type size_, const T& c)
{
    if(size_ > capacity())
        change_capacity(size_);        
    if(size_ < size())
        mm::destruct(begin_ + size_, end_);
    else if(size_ > size())
        mm::fill(end_, end_ + size_ - size(), c);
    end_ = begin_ + size_;
}

template <typename T>
typename rvector<T>::size_type 
rvector<T>::capacity() const noexcept
{
    return std::distance(begin_, cap_end_);
}


template <typename T>
bool rvector<T>::empty() const noexcept
{
    return begin_ == end_;
}

template <typename T>
void rvector<T>::reserve(rvector<T>::size_type n)
{
    if(n <= capacity()) return;
    n = n < 2*capacity()? 2*capacity(): n;
    change_capacity(n);
}

template <typename T>
void rvector<T>::shrink_to_fit()
{
    change_capacity(size());
}

template <typename T>
typename rvector<T>::reference 
rvector<T>::operator[](rvector<T>::size_type n)
{
    return begin_[n];
}

template <typename T>
typename rvector<T>::const_reference 
rvector<T>::operator[](rvector<T>::size_type n) const
{
    return begin_[n];
}

template <typename T>
typename rvector<T>::reference 
rvector<T>::at(rvector<T>::size_type n)
{
    if(UNLIKELY(n >= size()))
        throw std::out_of_range("Index out of range: " + std::to_string(n));
    return begin_[n];
}

template <typename T>
typename rvector<T>::const_reference 
rvector<T>::at(rvector<T>::size_type n) const
{
    if(UNLIKELY(n >= size()))
        throw std::out_of_range("Index out of range: " + std::to_string(n));
    return begin_[n];
}

template <typename T>
typename rvector<T>::reference 
rvector<T>::front() noexcept
{
    return *begin_;
}

template <typename T>
typename rvector<T>::const_reference 
rvector<T>::front() const noexcept
{
    return *begin_;
}

template <typename T>
inline
typename rvector<T>::reference 
rvector<T>::back() noexcept
{
    return *(end_ - 1);
}

template <typename T>
inline
typename rvector<T>::const_reference 
rvector<T>::back() const noexcept
{
    return *(end_ - 1);
}

template <typename T>
T* rvector<T>::data() noexcept
{
    return begin_;
}

template <typename T>
const T* rvector<T>::data() const noexcept
{
    return begin_;
}

template <typename T>
template <class... Args> 
void rvector<T>::emplace_back(Args&&... args)
{
    grow();
    new (end_) T(std::forward<Args>(args)...);
    end_++;
}

template <typename T>
template <class... Args> 
void rvector<T>::fast_emplace_back(Args&&... args)
{
    new (end_) T(std::forward<Args>(args)...);
    end_++;
}

template <typename T>
void rvector<T>::push_back(const T& x)
{
    grow();
    new (end_) T(x);
    end_++;
}

template <typename T>
void rvector<T>::fast_push_back(const T& x)
{
    new (end_) T(x);
    end_++;
}

template <typename T>
void rvector<T>::push_back(T&& x)
{
    grow();
    new (end_) T(std::forward<T>(x));
    end_++;
}

template <typename T>
void rvector<T>::fast_push_back(T&& x)
{
    new (end_) T(std::forward<T>(x));
    end_++;
}

template <typename T>
void rvector<T>::pop_back() noexcept
{
    if constexpr(!std::is_trivially_destructible_v<T>)
        back().~T();
    end_--;
}

template <typename T>
void rvector<T>::safe_pop_back() noexcept
{
    if(empty()) return;
    if constexpr(!std::is_trivially_destructible_v<T>)
        back().~T();
    end_--;
}

template <typename T>
template <class... Args> 
typename rvector<T>::iterator 
rvector<T>::emplace(rvector<T>::const_iterator position, 
                    Args&&... args)
{
    size_type n = position - begin_;
    grow();
    iterator position_ = begin_ + n;
    mm::shiftr_data(position_, (end_ - position_));
    new (position_) T(std::forward<Args>(args)...);
    end_++;
    return position_;
}

template <typename T>
typename rvector<T>::iterator 
rvector<T>::insert(rvector<T>::iterator position, const T& x)
{
    auto m = std::distance(begin_, position);
    grow();
    position = begin_ + m;
    mm::shiftr_data(position, (end_ - position));
    new (position) T(x);
    end_++;
    return position;
}

template <typename T>
typename rvector<T>::iterator 
rvector<T>::insert(rvector<T>::iterator position, T&& x)
{
    auto m = std::distance(begin_, position);
    grow();
    position = begin_ + m;
    mm::shiftr_data(position, (end_ - position));
    new (position) T(std::forward<T>(x));
    end_++;
    return position;
}
// TODO: add realloc optimalization for insert
template <typename T>
typename rvector<T>::iterator 
rvector<T>::insert(rvector<T>::iterator position, size_type n, const T& x)
{
    if(size() + n > capacity())
    {
        auto m = std::distance(begin_, position);
        size_type new_cap = std::max(size() + n, capacity() * 2 + 1);
        change_capacity(new_cap);
        position = begin_ + m;
    }
    // mm::shiftr_data(position, (end() - position_), n);
    size_type rest = std::distance(position, end_);
    if(rest > n) {
        std::uninitialized_copy(position + rest - n, end_, end_);
        std::copy_backward(position, position + rest-n, end_);
        std::fill(position, position + n, x);
    } else {
        std::uninitialized_copy(position, end_, end_ + n - rest);
        std::fill(position, end_, x);
        std::uninitialized_fill(end_, end_ + n - rest, x);
    }
    end_ += n;
    return position;
}

template <typename T>
template <class InputIterator, typename>
typename rvector<T>::iterator 
rvector<T>::insert (rvector<T>::iterator position, InputIterator first, 
                     InputIterator last)
{
    size_type n = std::distance(first, last);
    if(size() + n > capacity())
    {
        auto m = std::distance(begin_, position);
        size_type new_cap = std::max(size() + n, capacity() * 2 + 1);
        change_capacity(new_cap);
        position = begin_ + m;
    }
    size_type rest = std::distance(position, end_);
    if(rest > n) {
        std::uninitialized_copy(position + rest - n, end_, end_);
        std::copy_backward(position, position + rest-n, end_);
        std::copy(first, last, position);
    } else {
        std::uninitialized_copy(position, end_, end_ + n - rest);
        std::copy(first, first + rest, position);
        std::uninitialized_copy(first + rest, last, end_);
    }
    end_ += n;
    return position;    
}

template <typename T>
typename rvector<T>::iterator 
rvector<T>::insert(rvector<T>::iterator position, std::initializer_list<T> ilist)
{
    auto first = ilist.begin();
    auto last = ilist.end();
    size_type n = std::distance(first, last);
    if(size() + n > capacity())
    {
        auto m = std::distance(begin_, position);
        size_type new_cap = std::max(size() + n, capacity() * 2 + 1);
        change_capacity(new_cap);
        position = begin_ + m;
    }
    size_type rest = std::distance(position, end_);
    if(rest > n) {
        std::uninitialized_copy(position + rest - n, end_, end_);
        std::copy_backward(position, position + rest-n, end_);
        std::copy(first, last, position);
    } else {
        std::uninitialized_copy(position, end_, end_ + n - rest);
        std::copy(first, first + rest, position);
        std::uninitialized_copy(first + rest, last, end_);
    }
    end_ += n;
    return position; 
}

template <typename T>
typename rvector<T>::iterator 
rvector<T>::erase(rvector<T>::iterator position)
{
    if (position + 1 != end_)
        std::copy(position + 1, end_, position);
    pop_back();
    return position;
}

template <typename T>
typename rvector<T>::iterator 
rvector<T>::erase(rvector<T>::iterator first, rvector<T>::iterator last)
{
    auto n = std::distance(first, last);
    if (last != end_)
        std::copy(last, end_, first);
    mm::destruct(end_ - n, end_);
    end_ -= n;
    return first;
}

template <typename T>
void rvector<T>::swap(rvector<T>& other)
{
    using std::swap;
    swap(begin_, other.begin_);
    swap(end_, other.end_);
    swap(cap_end_, other.cap_end_);
}

template <typename T>
void rvector<T>::clear() noexcept
{
    mm::destruct(begin_, end_);
    end_ = begin_;
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
