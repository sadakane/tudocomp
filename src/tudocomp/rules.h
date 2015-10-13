#ifndef RULES_H
#define RULES_H

#include <iterator>
#include <initializer_list>

#include "sdsl/int_vector.hpp"
#include "sdsl/bits.hpp"
#include "sdsl/util.hpp"
#include "glog/logging.h"
#include "rule.h"

namespace tudocomp {

class GrowableIntVector {
public:
    typedef sdsl::int_vector<0>::iterator iterator;
    typedef sdsl::int_vector<0>::const_iterator const_iterator;

    typedef sdsl::int_vector<0>::reference reference;
    typedef sdsl::int_vector<0>::const_reference const_reference;

    typedef sdsl::int_vector<0>::size_type size_type;
    typedef sdsl::int_vector<0>::difference_type difference_type;

private:
    sdsl::int_vector<0> vec;
    size_type m_size;

public:
    inline GrowableIntVector(): vec(0,0,0), m_size(0) {
    }

    inline void push_back(uint64_t value) {
        uint64_t value_width = sdsl::bits::hi(value) + 1;

        size_type old_cap = vec.size();

        DLOG(INFO) << "value = " << value;
        DLOG(INFO) << "value_width = " << value_width;
        DLOG(INFO) << "vec.width() = " << int(vec.width());
        DLOG(INFO) << "vec.size() = " << int(vec.size());

        // grow width if neccessary
        if (value_width > vec.width()) {
            sdsl::util::expand_width(vec, value_width);

            DLOG(INFO) << "new vec.width() = " << int(vec.width());
        }

        // grow capacity if neccessary (vec.size() is the capacity)
        if (m_size == vec.size()) {
            // Needs cast to signed m_size in the middle so that it properly
            // calculates -1 for the m_size == 0 case.
            size_type cap = m_size + (difference_type(m_size) - 1) / 2 + 1;
            vec.resize(cap);

            DLOG(INFO) << "new vec.size() = " << int(vec.size());
        }

        DCHECK(old_cap <= vec.size());

        vec[m_size] = std::move(value);
        m_size++;
    }

    inline const iterator begin() {
        return vec.begin();
    }

    inline const iterator end() {
        return vec.end();
    }

    inline const const_iterator begin() const {
        return vec.begin();
    }

    inline const const_iterator end() const {
        return vec.end();
    }

    inline size_type size() const {
        return m_size;
    }

    inline auto operator[](const size_type& idx) -> reference {
        DLOG(INFO) << "["<<idx<<" / "<<size()<<"] +";
        reference tmp = vec[idx];
        DLOG(INFO) << "["<<idx<<" / "<<size()<<"] -";
        return tmp;
    }

    inline auto operator[](const size_type& idx) const -> const_reference
    {
        DLOG(INFO) << "const ["<<idx<<" / "<<size()<<"] +";
        const_reference tmp = vec[idx];
        DLOG(INFO) << "const ["<<idx<<" / "<<size()<<"] -";
        return tmp;
    }
};

class RulesIterator;
class RulesReference;

/// A specialized container class of Rules
class Rules {
public:
    typedef RulesIterator iterator;
    typedef RulesIterator const_iterator;

    typedef RulesReference reference;
    typedef RulesReference const_reference;

    typedef sdsl::int_vector<0>::size_type size_type;
    typedef sdsl::int_vector<0>::difference_type difference_type;

private:
    GrowableIntVector targets;
    GrowableIntVector sources;
    GrowableIntVector nums;
    friend RulesReference;

public:
    inline Rules() {}

    inline Rules(std::initializer_list<Rule> rules) {
        for (Rule rule : rules) {
            push_back(rule);
        }
    }

    inline const size_type size() {
        return targets.size();
    }

    inline void push_back(Rule value) {
        targets.push_back(value.target);
        sources.push_back(value.source);
        nums.push_back(value.num);
    }
    inline const iterator begin();
    inline const iterator end();
    inline reference operator[](const size_type& i);
};

/// A reference to an element in a Rules container
class RulesReference {
public:
    typedef Rule value_type;
private:
    sdsl::int_vector<0>::size_type m_i;
    Rules* m_rules;
public:
    inline RulesReference(sdsl::int_vector<0>::size_type i,
                                Rules* rules):
        m_i(i), m_rules(rules) {
    };

    inline RulesReference& operator=(value_type x) {
        m_rules->targets[m_i] = x.target;
        m_rules->sources[m_i] = x.source;
        m_rules->nums[m_i] = x.num;
        return *this;
    };

    inline RulesReference& operator=(const RulesReference& x) {
        return *this = value_type(x);
    };

    inline operator value_type() const {
        Rule r {
            m_rules->targets[m_i],
            m_rules->sources[m_i],
            m_rules->nums[m_i],
        };
        return r;
    }

    inline bool operator==(const RulesReference& x)const {
        return value_type(*this) == value_type(x);
    }

    inline bool operator!=(const RulesReference& x)const {
        return value_type(*this) != value_type(x);
    }
};

/// A Rules iterator
class RulesIterator: public std::iterator<
    std::input_iterator_tag,
    Rule,
    sdsl::int_vector<0>::difference_type,
    RulesReference*,
    RulesReference>
{
    sdsl::int_vector<0>::size_type m_i;
    Rules* m_rules;
public:
    // iterator
    inline RulesIterator(sdsl::int_vector<0>::size_type i, Rules* rules): m_i(i), m_rules(rules) {};
    inline RulesIterator(const RulesIterator& it): m_i(it.m_i), m_rules(it.m_rules) {};
    inline RulesIterator& operator++() {
        ++m_i;
        return *this;
    }

    inline RulesIterator operator++(int) {
        RulesIterator tmp(*this);
        operator++();
        return tmp;
    }

    // input/output iterator
    inline bool operator==(const RulesIterator& rhs) {
        return m_i == rhs.m_i;
    }
    inline bool operator!=(const RulesIterator& rhs) {
        return m_i != rhs.m_i;
    }

    inline RulesReference operator*() {
        return RulesReference(m_i, m_rules);
    }

    // forward iterator
    inline RulesIterator(): m_i(0), m_rules(nullptr) { }

    // bidirectional iterator
    inline RulesIterator& operator--() {
        --m_i;
        return *this;
    }

    inline RulesIterator operator--(int) {
        RulesIterator tmp(*this);
        operator--();
        return tmp;
    }

    // random access iterator
    inline friend RulesIterator operator+(RulesIterator lhs,
                                          const difference_type rhs) {
        lhs += rhs;
        return lhs;
    }
    inline friend RulesIterator operator+(const difference_type lhs,
                                          RulesIterator rhs) {
        rhs += lhs;
        return rhs;
    }
    inline friend RulesIterator operator-(RulesIterator lhs,
                                          const difference_type rhs) {
        lhs -= rhs;
        return lhs;
    }
    inline difference_type operator-(const RulesIterator& rhs) const {
        difference_type tmp = m_i - rhs.m_i;
        return tmp;
    }

    inline bool operator<(const RulesIterator& rhs) const {
        return m_i < rhs.m_i;
    }

    inline bool operator>(const RulesIterator& rhs) const {
        return rhs < *this;
    }

    inline bool operator<=(const RulesIterator& rhs) const {
        return !(*this > rhs);
    }

    inline bool operator>=(const RulesIterator& rhs) const {
        return !(*this < rhs);
    }

    inline RulesIterator& operator+=(const difference_type rhs) {
        m_i += rhs;
        return *this;
    }

    inline RulesIterator& operator-=(const difference_type rhs) {
        m_i -= rhs;
        return *this;
    }

    inline RulesReference operator[](const difference_type i) {
        RulesIterator tmp(*this);
        tmp += i;
        return *tmp;
    }
};

inline void swap(RulesReference x,
                 RulesReference y)
{
    Rule tmp = x;
    x = y;
    y = tmp;
}

inline void swap(Rule& x,
                 RulesReference y)
{
    Rule tmp = x;
    x = y;
    y = tmp;
}

inline void swap(RulesReference x,
                 Rule& y)
{
    Rule tmp = x;
    x = y;
    y = tmp;
}

inline const Rules::iterator Rules::begin() {
    return RulesIterator(0, this);
}

inline const Rules::iterator Rules::end() {
    return RulesIterator(size(), this);
}

inline Rules::reference Rules::operator[](const Rules::size_type& i) {
    return RulesReference(i, this);
}

}

#endif