/*
 * BSD 2-Clause License
 * 
 * Copyright (c) 2022, Erwan DUHAMEL
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef STE_UNORDERED_BIMAP_HPP
#define STE_UNORDERED_BIMAP_HPP

#include <unordered_map>
#include <iostream>
#include <algorithm>

/// Standard Libary Extensions developped by Erellu. See the others here https://github.com/Erellu.
namespace ste
{

/**
 *  @brief Unordered bidirectionnal map.
 *  One key is associated to one value and vice-versa.
 *
 *  @note The following functions are not supported:
 *
 *  - non-const versions of at(), use assign or extract / insert instead.
 *  - emplace
 *  - emplace_hint
 *  - equal_range
 *  - merge
 *  - non-const versions of operator[], use assign or extract / insert instead.
 *  - try_emplace
 *
*/
template<typename key_t, typename T>
class unordered_bimap
{

public:

    //--------------------------------------------------------
    // Types

    /// Placeholder for the data.
    struct placeholder_t
    {
        friend constexpr bool operator==(const placeholder_t&, const placeholder_t&)
        {
            return true;
        }

        friend constexpr bool operator!=(const placeholder_t&, const placeholder_t&)
        {
            return false;
        }
    };

    using key_type        = key_t;
    using mapped_type     = T;

    using value_type      = std::pair<key_type, mapped_type>;
    using reference       = value_type&;
    using const_reference = const value_type&;

    /// Hasher for key. Always returns 0 so all hash collide and compare struct is used.
    struct hash_t
    {
        std::size_t operator()(const value_type& a) const
        {
            return 0;
        }
    };

    /// Compare operator for the key. Compares the a.first and b.first if a.first has a value,
    /// else a.second and b.second.
    struct compare_t
    {
        bool operator()(const value_type& a, const value_type& b) const
        {
            return (a.first == b.first) || (a.second == b.second);
        }
    };

    using hasher          = hash_t;
    using key_equal       = compare_t;

    using map_t =  std::unordered_map<value_type, placeholder_t, hash_t, compare_t>;

    using size_type            = typename map_t::size_type;
    using difference_type      = typename map_t::difference_type;
    using allocator_type       = typename map_t::allocator_type;
    using iterator             = typename map_t::iterator;
    using const_iterator       = typename map_t::const_iterator;
    using local_iterator       = typename map_t::local_iterator;
    using const_local_iterator = typename map_t::const_local_iterator;

    #if __cplusplus > 201402L
    using node_type          = typename map_t::node_type;
    using insert_return_type = typename map_t::insert_return_type;
    #endif //__cplusplus > 201402L


    //--------------------------------------------------------
    // Construction, desctruction, assignment

    unordered_bimap() = default;
    unordered_bimap(const unordered_bimap&) = default;
    unordered_bimap(unordered_bimap&&) = default;

    explicit unordered_bimap(
          size_type buckets,
          const hasher& hash = hasher(),
          const key_equal& equal = key_equal())
    : m_data(buckets, hash, equal)
    { }

    template<typename input_iterator_t>
    unordered_bimap(
              input_iterator_t first, input_iterator_t last,
              size_type buckets = 0,
              const hasher& hash = hasher(),
              const key_equal& equal = key_equal())
    : m_data(first, last, buckets, hash, equal)
    { }

    unordered_bimap(
            std::initializer_list<value_type> list,
            size_type buckets = 0,
            const hasher& hash = hasher(),
            const key_equal& equal = key_equal())
        : m_data({}, buckets, hash, equal)
    {
        for(auto&& item : list)
        {
            m_data.insert({item, {}});
        }
    }

    unordered_bimap(size_type buckets)
    : unordered_bimap(buckets, hasher(), key_equal())
    { }

    unordered_bimap(size_type buckets, const hasher& hash)
    : unordered_bimap(buckets, hash, key_equal())
    { }

    template<typename input_iterator_t>
    unordered_bimap(input_iterator_t first, input_iterator_t last,  size_type buckets)
    : unordered_bimap(first, last, buckets, hasher(), key_equal())
    { }

    template<typename input_iterator_t>
    unordered_bimap(input_iterator_t first, input_iterator_t last, size_type buckets, const hasher& hash)
      : unordered_bimap(first, last, buckets, hash, key_equal())
    { }

    unordered_bimap(std::initializer_list<value_type> list,  size_type buckets)
    : unordered_bimap(list, buckets, hasher(), key_equal())
    {
    }

    unordered_bimap(std::initializer_list<value_type> list, size_type buckets, const hasher& hash)
    : unordered_bimap(list, buckets, hash, key_equal())
    {
    }

    unordered_bimap& operator=(const unordered_bimap& other)     = default;
    unordered_bimap& operator=(unordered_bimap&& other) noexcept = default;

    unordered_bimap& operator=(std::initializer_list<value_type> ilist)
    {
        clear();
        for(auto&& item : ilist)
        {
            m_data.insert({item, {}});
        }
    }

    ~unordered_bimap(){}

    allocator_type get_allocator() const noexcept
    {
        return m_data.get_allocator();
    }

    //--------------------------------------------------------
    // Iterators

    iterator begin() noexcept
    {
        return m_data.begin();
    }

    const_iterator begin() const noexcept
    {
        return m_data.begin();
    }

    const_iterator cbegin() const noexcept
    {
        return m_data.cbegin();
    }

    iterator end() noexcept
    {
        return m_data.end();
    }

    const_iterator end() const noexcept
    {
        return m_data.end();
    }

    const_iterator cend() const noexcept
    {
        return m_data.cbegin();
    }

    //--------------------------------------------------------
    // Capacity

    bool empty() const
    {
        return m_data.empty();
    }

    std::size_t size() const
    {
        return m_data.size();
    }

    std::size_t max_size() const
    {
        return m_data.max_size();
    }

    //--------------------------------------------------------
    // Modifiers

    void clear()
    {
        m_data.clear();
    }

    std::pair<iterator, bool> insert(const key_type& k, const T& t)
    {
        return m_data.insert({{k, t}, {}});
    }

    std::pair<iterator, bool> insert(const value_type& v)
    {
        return m_data.insert({v, {}});
    }

    std::pair<iterator, bool> insert(key_type&& k, T&& t)
    {
        return m_data.insert({{k, t}, {}});
    }

    iterator insert(value_type&& v)
    {
        return m_data.insert({v, {}});
    }

    void insert(std::initializer_list<value_type> list)
    {
        for(auto&& item : list)
        {
            m_data.insert({item, {}});
        }
    }

    #if __cplusplus > 201402L
    insert_return_type insert(node_type&& nh)
    {
        return m_data.insert(nh);
    }

    iterator insert(const_iterator hint, node_type&& nh)
    {
        return m_data.insert(hint, nh);
    }

    template <class M>
    std::pair<iterator, bool> insert_or_assign(const key_type& k, M&& obj)
    {
        return m_data.insert_or_assign({{k, obj}, {}});
    }
    template <class M>
    std::pair<iterator, bool> insert_or_assign( key_type&& k, M&& obj )
    {
        return m_data.insert_or_assign({{k, obj}, {}});
    }
    template <class M>
    iterator insert_or_assign( const_iterator hint, const key_type& k, M&& obj )
    {
        return m_data.insert_or_assign(hint, {{k, obj}, {}});
    }
    template <class M>
    iterator insert_or_assign( const_iterator hint, key_type&& k, M&& obj )
    {
        return m_data.insert_or_assign(hint, {{k, obj}, {}});
    }
    #endif // __cplusplus > 201402L

    void swap(unordered_bimap& other) noexcept
    {
        m_data.swap(other.m_data);
    }

    iterator erase(iterator it)
    {
        return m_data.erase(it);
    }

    iterator erase(const_iterator it)
    {
        return m_data.erase(it);
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        return m_data.erase(first, last);
    }

    /// Erases the item with the key provided.
    size_type erase_key(const key_type& k)
    {
        const_iterator it = find_key(k);
        if(it != end())
        {
            erase(it);
            return 1;
        }

        return 0;
    }

    /// Erases the item with the value provided.
    size_type erase_value(const mapped_type& t)
    {
        const auto it = find_value(t);
        if(it != end())
        {
            erase(it);
            return 1;
        }

        return 0;
    }

    ///Sets key value. Does nothing if no key matches.
    void assign_value(const key_type& k, const mapped_type& t)
    {
        const_iterator it = find_key(k);
        if(it != end())
        {
            erase(it);
            insert(value_type{k, t});
        }
    }

    void assign_value(key_type&& k, mapped_type&& t)
    {
        const_iterator it = find_key(k);
        if(it != end())
        {
            const auto& v = (*it).first;
            erase(it);
            insert(k, t);
        }
    }

    ///Sets value key. Does nothing if no value matches.
    void assign_key(const mapped_type& t, const key_type& k)
    {
        const_iterator it = find_value(t);
        if(it != end())
        {
            erase(it);
            insert(k, t);
        }
    }

    ///Sets value key. Does nothing if no value matches.
    void assign_key(mapped_type&& t, key_type&& k)
    {
        const_iterator it = find_value(t);
        if(it != end())
        {
            erase(it);
            insert(k, t);
        }
    }

    //--------------------------------------------------------
    // Lookup

    const value_type& at_key(const key_type& k) const
    {
        const auto it = find_key(k);
        if(it == end())
        {
            throw std::out_of_range("No entry with such key.");
        }
        return (*it).first;
    }

    const value_type& at_value(const mapped_type& k) const
    {
        const auto it = find_value(k);
        if(it == end())
        {
            throw std::out_of_range("No entry with such value.");
        }
        return (*it).first;
    }

    const value_type& operator[](const key_type& k)
    {
        const_iterator it = find_key(k);
        if(it == end())
        {
            const auto& item_it = insert(k, T()).first;
            return (*item_it).first;
        }

        return (*it).first;
    }

    const value_type& operator[](const value_type& t)
    {
        const_iterator it = find_value(t);
        if(it == end())
        {
            const auto& item_it =  insert(key_type(), t).first;
            return (*item_it).first;
        }

        return (*it).first;
    }

    std::size_t count_key(const key_type& k) const
    {
        return m_data.count({k, mapped_type()});
    }

    std::size_t count_value(const mapped_type& t) const
    {
        return m_data.count({{}, t});
    }

    const_iterator find_key(const key_type& k) const
    {
        using v_t = typename map_t::value_type;
        const auto predicate = [&](const v_t& v) -> bool
        {
            return v.first.first == k;
        };

        return std::find_if(m_data.begin(), m_data.end(), predicate);
    }

    const_iterator find_value(const mapped_type& t) const
    {
        using v_t = typename map_t::value_type;
        const auto predicate = [&](const v_t& v) -> bool
        {
            return v.first.second == t;
        };

        return std::find_if(m_data.begin(), m_data.end(), predicate);
    }

    bool contains_key(const key_type& k) const
    {
        return find_key(k) != end();
    }

    bool contains_value(const mapped_type& t) const
    {
        return find_value(t) != end();
    }

    //--------------------------------------------------------
    // Bucket interface

    local_iterator begin(size_type n)
    {
        return m_data.begin(n);
    }

    const_local_iterator begin(size_type n) const
    {
        return m_data.begin(n);
    }

    const_local_iterator cbegin(size_type n) const
    {
        return m_data.cbegin(n);
    }

    local_iterator end(size_type n)
    {
        return m_data.end(n);
    }

    const_local_iterator end(size_type n) const
    {
        return m_data.end(n);
    }

    const_local_iterator cend(size_type n) const
    {
        return m_data.cend(n);
    }

    //--------------------------------------------------------
    // Hash policy

    float load_factor() const
    {
        return m_data.load_factor();
    }

    float max_load_factor() const
    {
        return m_data.max_load_factor();
    }

    void max_load_factor(float ml)
    {
        m_data.max_load_factor(ml);
    }

    void rehash(size_type count)
    {
        m_data.rehash(count);
    }

    void reserve(size_type count)
    {
        m_data.reserve(count);
    }

    //--------------------------------------------------------
    // Observers

    hasher hash_function() const
    {
        return m_data.hash_function();
    }

    key_equal key_eq() const
    {
        return m_data.key_eq();
    }

    //--------------------------------------------------------
    // Stream

    friend std::ostream& operator<< (std::ostream& out, const unordered_bimap& m)
    {
        out << "{";
        for(const auto& [data, placeholder] : m)
        {
            std::ignore = placeholder;

            out << "{" << data.first << " , " << data.second << "},";
        }
        out << "}";

        return out;
    }

    //--------------------------------------------------------
    // Comparison

    friend bool operator==(const unordered_bimap& a, const unordered_bimap& b)
    {
        return a.m_data == b.m_data;
    }

    friend bool operator!=(const unordered_bimap& a, const unordered_bimap& b)
    {
        return a.m_data != b.m_data;
    }

private:

    /// Underlaying map.
    map_t m_data;
};

/**
 *  @brief Erases all elements that satisfy the predicate pred from the container.
 *  @param m Container from which to erase.
 *  @param p Predicate that returns true if the element should be erased.
 *  @returns The number of erased elements.
 */
template<typename key_t, typename T, typename predicate_t>
inline typename unordered_bimap<key_t, T>::size_type erase_if(unordered_bimap<key_t, T>&m, predicate_t predicate)
{
    using size_type = typename unordered_bimap<key_t, T>::size_type;

    const size_type old_size = m.size();

    for (auto i = m.begin(), last = m.end(); i != last; )
    {
      if (predicate(*i))
      {
        i = m.erase(i);
      }
      else
      {
        ++i;
      }
    }

    return old_size - m.size();
}

} //namespace ste

#endif // STE_UNORDERED_BIMAP_HPP
