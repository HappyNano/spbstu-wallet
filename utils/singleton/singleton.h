#pragma once

#include <memory>
#include <type_traits>

namespace util {

    /**
     * @class Singleton
     * @brief Template class implementing the Singleton design pattern with shared pointer management.
     *
     * This template class provides a thread-safe implementation of the Singleton pattern
     * using std::shared_ptr. It allows for lazy initialization and explicit instance replacement.
     * The singleton instance is automatically created on first access if the template parameter
     * class is default-constructible.
     *
     * @tparam Class The class type for which to create a singleton instance
     */
    template < typename Class >
    class Singleton {
    public:
        using shared = std::shared_ptr< Class >;

        /**
         * @brief Sets a new instance as the singleton.
         *
         * Replaces the current singleton instance with the provided shared pointer.
         * This allows for dependency injection or instance replacement during testing.
         *
         * @param newPtr The new shared pointer instance to be used as the singleton
         */
        static void set(shared newPtr) {
            instance_.swap(newPtr);
        }

        /**
         * @brief Gets the current singleton instance.
         *
         * If the class is default-constructible and no instance exists yet,
         * a new instance is automatically created. Otherwise, returns the
         * current instance (which may be nullptr if not previously set).
         *
         * @return shared Shared pointer to the singleton instance
         */
        static shared get() {
            if constexpr (std::is_default_constructible_v< Class >) {
                if (!instance_) {
                    instance_ = std::make_shared< Class >();
                }
            }
            return instance_;
        }

        virtual ~Singleton() = default;

    protected:
        /**
         * @brief The shared pointer that holds the singleton instance.
         */
        static shared instance_;

        Singleton() = default;
    };

    template < typename Class >
    typename Singleton< Class >::shared Singleton< Class >::instance_ = nullptr;

} // namespace util
