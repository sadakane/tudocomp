#pragma once

#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <tudocomp/meta/AlgorithmConfig.hpp>
#include <tudocomp/meta/Meta.hpp>

namespace tdc {
namespace meta {

/// \brief Error type for registry related errors.
class RegistryError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

template<typename T>
class Registry {
private:
    using ctor_t = std::function<std::unique_ptr<T>(AlgorithmConfig&&)>;

    TypeDesc m_root_type;
    AlgorithmLib m_lib;
    std::unordered_map<std::string, ctor_t> m_reg;

public:
    inline Registry() : m_root_type(T::type_desc()) {
    }

    inline Registry(const TypeDesc& root_type) : m_root_type(root_type) {
    }

    template<typename Algo>
    inline void register_algorithm() {
        auto meta = Algo::meta();
        auto type = meta.decl()->type();
        if(!type.subtype_of(m_root_type)) {
            throw RegistryError(std::string(
                "trying to register algorithm of type ") +
                type.name() +
                std::string(", expected ") +
                m_root_type.name());
        }

        auto sig = meta.signature()->str();
        auto it = m_reg.find(sig);
        if(it == m_reg.end()) {
            add_to_lib(m_lib, meta);
            m_reg.emplace(sig, [](AlgorithmConfig&& cfg) {
                return std::make_unique<Algo>(std::move(cfg));
            });
        } else {
            throw RegistryError(std::string("already registered: ") + sig);
        }
    }

    class Entry;
    class Selection {
    private:
        friend class Entry;
        friend class Registry;

        std::shared_ptr<const AlgorithmDecl> m_decl;
        std::unique_ptr<T> m_instance;

        inline Selection(
            std::shared_ptr<const AlgorithmDecl> decl,
            std::unique_ptr<T>&& instance)
            : m_decl(decl), m_instance(std::move(instance)) {
        }

    public:
        inline Selection() {
        }

        inline Selection(Selection&& other)
            : m_decl(std::move(other.m_decl)),
              m_instance(std::move(other.m_instance)) {
        }

        inline Selection& operator=(Selection&& other) {
            m_decl = std::move(other.m_decl);
            m_instance = std::move(other.m_instance);
            return *this;
        }

        inline operator bool() const {
            return bool(m_instance);
        }

        inline std::shared_ptr<const AlgorithmDecl> decl() const {
            return m_decl;
        }

        inline T& instance() {
            return *m_instance;
        }

        inline T& operator*() {
            return instance();
        }

        inline T* operator->() {
            return m_instance.get();
        }
    };

    class Entry {
    private:
        friend class Registry;

        std::shared_ptr<const AlgorithmDecl> m_decl;
        const ctor_t* m_ctor;
        AlgorithmConfig m_cfg;

        inline Entry(
            std::shared_ptr<const AlgorithmDecl> decl,
            const ctor_t& ctor,
            AlgorithmConfig&& cfg) : m_decl(decl), m_ctor(&ctor), m_cfg(cfg) {
        }

    public:
        inline std::shared_ptr<const AlgorithmDecl> decl() const {
            return m_decl;
        }

        inline const AlgorithmConfig& config() const {
            return m_cfg.decl();
        }

        inline Selection select() const {
            return Selection(m_decl, (*m_ctor)(AlgorithmConfig(m_cfg)));
        }
    };

    inline Entry find(ast::NodePtr<ast::Object> obj) const {
        auto decl = m_lib.find(obj->name(), m_root_type);
        if(!decl) {
            throw RegistryError(
                std::string("unknown algorithm: ") + obj->name());
        }

        auto cfg = AlgorithmConfig(decl, obj, m_lib);
        auto sig = cfg.signature();
        auto reg_entry = m_reg.find(sig->str());

        if(reg_entry == m_reg.end()) {
            throw RegistryError(
                std::string("unregistered instance: ") + sig->str());
        }

        return Entry(decl, reg_entry->second, std::move(cfg));
    }

    inline Selection select(ast::NodePtr<ast::Object> obj) const {
        return find(obj).select();
    }

    inline Selection select(const std::string& str) const {
        auto obj = ast::convert<ast::Object>(ast::Parser::parse(str));
        return select(obj);
    }

    template<typename C>
    inline Selection select(const std::string& options = "") const {
        auto meta = C::meta();
        auto decl = meta.decl();
        auto parsed = ast::convert<ast::Object>(
            ast::Parser::parse(decl->name() + paranthesize(options)));
        auto obj = parsed->inherit(meta.signature());
        auto cfg = AlgorithmConfig(
            decl, obj, m_lib + meta.known());

        return Selection(decl, std::make_unique<C>(std::move(cfg)));
    }

    inline std::string generate_doc_string(const std::string& title) const {
        // TODO: implement
        std::stringstream ss;
        for(auto e : m_reg) {
            ss << e.first << std::endl;
        }
        return ss.str();
    }

    inline std::vector<std::string> signatures() const {
        std::vector<std::string> sigs;
        for(auto e : m_reg) sigs.emplace_back(e.first);
        return sigs;
    }
};

} //namespace meta

template<typename T>
using Registry = meta::Registry<T>;

} //namespace tdc