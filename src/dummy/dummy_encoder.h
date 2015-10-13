#ifndef DUMMY_CODER_H
#define DUMMY_CODER_H

#include "tudocomp.h"

// Put every C++ code in this project into a common namespace
// in order to not pollute the global one
namespace dummy {

using namespace tudocomp;

class DummyCoder: public Coder {
public:
    virtual void code(Rules, Input input, std::ostream&) override final;
    virtual void decode(std::istream& inp, std::ostream& out) override final;
    virtual size_t min_encoded_rule_length(size_t input_size) override final;
};

}

#endif