#pragma once

#include <memory>

namespace util {

    template < typename Class >
    class Singleton {
    public:
        using shared = std::shared_ptr< Class >;

        static shared get() {
            if (!instance_) {
                instance_ = std::make_shared< Class >();
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
