#pragma once

#include <memory>
#include <type_traits>

namespace util {

    template < typename Class >
    class Singleton {
    public:
        using shared = std::shared_ptr< Class >;

        static void set(shared newPtr) {
            instance_.swap(newPtr);
        }

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
        static shared instance_;

        Singleton() = default;
    };

    template < typename Class >
    typename Singleton< Class >::shared Singleton< Class >::instance_ = nullptr;

} // namespace util
