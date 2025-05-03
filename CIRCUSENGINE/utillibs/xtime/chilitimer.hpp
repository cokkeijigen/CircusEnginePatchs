#pragma once
#include <chrono>

namespace utils::xtime {

    class chilitimer
    {
        using steady_clock = std::chrono::steady_clock;
        std::chrono::steady_clock::time_point last{};
    public:

        inline chilitimer() noexcept
        {
            this->last = steady_clock::now();
        }
        
        template<class R = float>
        requires std::is_floating_point<R>::value
        inline auto mark() noexcept -> R
        {
            const auto old{ this->last };
            this->last = steady_clock::now();
            std::chrono::duration<R> result
            {
                this->last - old
            };
            return result.count();
        }

        template<class R = float>
        requires std::is_floating_point<R>::value
        inline auto peek() const noexcept -> R
        {
            std::chrono::duration<R> result
            {
                steady_clock::now() - this->last
            };
            return result.count();
        }
    };

}
