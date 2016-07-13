#ifndef _INCLUDED_DS_SUFFIX_ARRAY_HPP
#define _INCLUDED_DS_SUFFIX_ARRAY_HPP

#include <tudocomp/ds/ITextDSProvider.hpp>

#include <divsufsort.h>
#include <divsufsort64.h>

#include <sdsl/int_vector.hpp>

#include <tudocomp/io.h>
#include <tudocomp/sdslex/int_vector_wrapper.hpp>
#include <tudocomp/util.h>

namespace tudocomp {

using io::InputView;

class SuffixArray {

public:
    typedef sdsl::int_vector<> iv_t;

private:
    iv_t m_sa;

public:
    inline SuffixArray() {
    }

    const iv_t& sa = m_sa;

    inline iv_t::value_type operator[](iv_t::size_type i) const {
        return m_sa[i];
    }

    inline iv_t::size_type size() const {
        return m_sa.size();
    }

    void construct(ITextDSProvider& t) {
        size_t len = t.size();

        uint8_t* copy = new uint8_t[len + 1];
        for(size_t i = 0; i < len; i++) {
            copy[i] = t[i];
        }
        copy[len] = 0;

        //Use divsufsort to construct
        int32_t *sa = new int32_t[len + 1];
        divsufsort(copy, sa, len + 1);

        delete[] copy;

        //Bit compress using SDSL
        size_t w = bitsFor(len + 1);
        m_sa = iv_t(len + 1, 0, w);

        for(size_t i = 0; i < len + 1; i++) {
            size_t s = sa[i];

            m_sa[i]  = s;
        }

        delete[] sa;
    }
};

}

#endif

