#ifndef TUDOCOMP_REGISTRY_H
#define TUDOCOMP_REGISTRY_H

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <istream>
#include <map>
#include <sstream>
#include <streambuf>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <boost/utility/string_ref.hpp>
#include <glog/logging.h>

#include <tudocomp/Env.hpp>
#include <tudocomp/Compressor.hpp>

namespace tudocomp_driver {

using namespace tudocomp;

struct AlgorithmInfo;

struct AlgorithmDb {
    std::string kind;
    std::vector<AlgorithmInfo> sub_algo_info;

    inline void print_to(std::ostream& out, int indent);

    inline std::vector<std::string> id_product();
};

struct AlgorithmInfo {
    /// Human readable name
    std::string name;
    /// Used as id string for command line and output filenames
    std::string shortname;
    /// Description text
    std::string description;
    std::vector<AlgorithmDb> sub_algo_info;

    inline void print_to(std::ostream& out, int indent);

};

inline std::vector<std::string> cross(std::vector<std::vector<std::string>>&& vs) {
    auto remaining = vs;
    if (remaining.size() == 0) {
        return {};
    }

    std::vector<std::string> first = std::move(remaining[0]);
    remaining.erase(remaining.begin());

    auto next = cross(std::move(remaining));

    if (next.size() == 0) {
        return first;
    } else {
        std::vector<std::string> r;
        for (auto& x : first) {
            for (auto& y : next) {
                r.push_back(x + "." + y);
            }
        }
        return r;
    }
}

inline std::vector<std::string> algo_cross_product(AlgorithmDb& subalgo) {
    std::vector<std::string> r;
    for (auto& algo : subalgo.sub_algo_info) {

        auto& name = algo.shortname;
        std::vector<std::vector<std::string>> subalgos;
        for (auto& subalgo : algo.sub_algo_info) {
            subalgos.push_back(algo_cross_product(subalgo));
        }

        if (subalgos.size() == 0) {
            r.push_back(name);
        } else {
            for (auto x : cross(std::move(subalgos))) {
                r.push_back(name + "." + x);
            }
        }
    }
    return r;
}

inline std::vector<std::string> AlgorithmDb::id_product() {
    return algo_cross_product(*this);
}

const int ALGO_IDENT = 2;

inline std::string kind_str(int indent, std::string kind) {
    std::stringstream out;
    out << std::setw(indent) << "" << "[" << kind << "]";
    return out.str();
}
inline void AlgorithmDb::print_to(std::ostream& out, int indent) {
    out << kind_str(indent, kind) << "\n";
    for (auto& e : sub_algo_info) {
        e.print_to(out, indent + ALGO_IDENT);
    }
}

inline std::string shortname_str(int indent, std::string shortname) {
    std::stringstream out;
    out << std::setw(indent) << "" << shortname;
    return out.str();
}
inline void AlgorithmInfo::print_to(std::ostream& out, int indent) {
    out << shortname_str(indent, shortname) << " \t | "
        << name << ": "
        << description << "\n";
    for (auto& e : sub_algo_info) {
        e.print_to(out, indent + ALGO_IDENT);
    }
}

inline boost::string_ref pop_algorithm_id(boost::string_ref& algorithm_id) {
    auto idx = algorithm_id.find('.');
    int dot_size = 1;
    if (idx == boost::string_ref::npos) {
        idx = algorithm_id.size();
        dot_size = 0;
    }
    boost::string_ref r = algorithm_id.substr(0, idx);
    algorithm_id.remove_prefix(idx + dot_size);
    return r;
}

struct SubRegistry;

struct Registry {
    AlgorithmDb* m_algorithms;

    std::unordered_map<
        std::string,
        std::function<std::unique_ptr<Compressor>(Env&)>> m_compressors;

    // AlgorithmInfo

    template<class F>
    void compressor(std::string id, F f) {
        auto g = [=](Env& env) {
            using C = decltype(f(env));

            return std::unique_ptr<Compressor> {
                new C(f(env))
            };
        };

        m_compressors[id] = g;
    }

    inline SubRegistry algo(std::string id, std::string title, std::string desc);

    std::vector<std::string> check_for_undefined_compressors() {
        std::vector<std::string> r;
        for (auto& s : m_algorithms->id_product()) {
            if (m_compressors.count(s) == 0) {
                r.push_back(s);
            }
        }
        return r;
    }

    std::vector<AlgorithmInfo> get_sub_algos() {
        return m_algorithms->sub_algo_info;
    }
};

struct SubRegistry {
    std::vector<AlgorithmInfo>* m_vector;
    int m_index;

    template<class G>
    inline void sub_algo(std::string name, G g) {
        auto& x = (*m_vector)[m_index].sub_algo_info;
        x.push_back({name});
        Registry y { &x[x.size() - 1ul] };
        g(y);
    }

};

inline SubRegistry Registry::algo(std::string id, std::string title, std::string desc) {
    m_algorithms->sub_algo_info.push_back({title, id, desc});

    return SubRegistry {
        &m_algorithms->sub_algo_info,
        m_algorithms->sub_algo_info.size() - 1ul
    };
}

inline std::unique_ptr<Compressor> select_algo_or_exit(Registry& reg,
                                                       Env& env,
                                                       std::string a_id) {

    if (reg.m_compressors.count(a_id) == 0) {
        throw std::runtime_error("Unknown algorithm: " + a_id);
    } else {
        return reg.m_compressors[a_id](env);
    }
}

void register_algos(Registry& registry);

}

#endif