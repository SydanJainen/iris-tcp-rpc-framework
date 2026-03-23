#ifndef IRIS_CONTAINER_H
#define IRIS_CONTAINER_H

#include <any>
#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace iris {

template <typename T>
concept Resolvable = (std::copy_constructible<T> || std::move_constructible<T>)
                     && !std::is_pointer_v<T>;

template <typename T>
concept SharedPtrType = requires {
    typename T::element_type;
    requires std::is_class_v<typename T::element_type>;
} && std::is_copy_constructible_v<T>;


class Container {
public:
    /**
     * Register a singleton provider. The factory is called once on first
     * resolve; subsequent resolves return the cached instance.
     *
     * @tparam T The type to register (must satisfy Resolvable concept).
     * @param name Unique identifier for this dependency.
     * @param factory Callable that produces an instance of T.
     */
    template <Resolvable T>
    void register_singleton(const std::string& name,
                            std::function<T()> factory) {
        factories_[name] = [f = std::move(factory)]() -> std::any {
            return f();
        };
        is_singleton_[name] = true;
    }

    /**
     * Register a factory provider. The factory is called on every resolve,
     * producing a fresh instance each time.
     *
     * @tparam T The type to register (must satisfy Resolvable concept).
     * @param name Unique identifier for this dependency.
     * @param factory Callable that produces an instance of T.
     */
    template <Resolvable T>
    void register_factory(const std::string& name,
                          std::function<T()> factory) {
        factories_[name] = [f = std::move(factory)]() -> std::any {
            return f();
        };
        is_singleton_[name] = false;
    }

    /**
     * Resolve a dependency by name.
     *
     * For singletons, the factory is invoked on first call and the result
     * is cached. For factories, a new instance is created every time.
     *
     * @tparam T The expected type of the dependency.
     * @param name The identifier used during registration.
     * @return Reference to the resolved instance.
     * @throws std::runtime_error if the name is not registered.
     * @throws std::bad_any_cast if the stored type does not match T.
     */
    template <typename T>
    T& resolve(const std::string& name) {
        auto factory_it = factories_.find(name);
        if (factory_it == factories_.end()) {
            throw std::runtime_error(
                "Container: no provider registered for '" + name + "'");
        }

        if (is_singleton_[name]) {
            auto singleton_it = singletons_.find(name);
            if (singleton_it == singletons_.end()) {
                singletons_[name] = factory_it->second();
            }
            return std::any_cast<T&>(singletons_[name]);
        }

        transients_[name] = factory_it->second();
        return std::any_cast<T&>(transients_[name]);
    }

    /**
     * Check whether a provider is registered under the given name.
     *
     * @param name The identifier to look up.
     * @return True if a provider exists, false otherwise.
     */
    bool has(const std::string& name) const {
        return factories_.find(name) != factories_.end();
    }

private:

    std::map<std::string, std::function<std::any()>> factories_;
    std::map<std::string, std::any> singletons_;
    std::map<std::string, bool> is_singleton_;
    std::map<std::string, std::any> transients_;
};

}

#endif
