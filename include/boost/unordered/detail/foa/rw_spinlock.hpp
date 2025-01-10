#ifndef BOOST_UNORDERED_DETAIL_FOA_RW_SPINLOCK_HPP_INCLUDED
#define BOOST_UNORDERED_DETAIL_FOA_RW_SPINLOCK_HPP_INCLUDED

#include <atomic>
#include <cstdint>
#include <thread>

#if defined(_MSC_VER)
    #include <windows.h>
#elif defined(__x86_64__) || defined(__i386__)
    #include <immintrin.h>
#endif

namespace boost {
namespace unordered {
namespace detail {
namespace foa {

class rw_spinlock {
private:
    static inline void cpu_pause() noexcept {
        #if defined(_MSC_VER)
            YieldProcessor();
        #elif defined(__x86_64__) || defined(__i386__)
            _mm_pause();
        #elif defined(__arm__) || defined(__aarch64__)
            asm volatile("yield" ::: "memory");
        #else
            std::atomic_signal_fence(std::memory_order_seq_cst);
        #endif
    }

    static constexpr std::uint32_t locked_exclusive_mask = 1u << 31;
    static constexpr std::uint32_t writer_pending_mask = 1u << 30;
    static constexpr std::uint32_t reader_lock_count_mask = writer_pending_mask - 1;

    std::atomic<std::uint32_t> state_ = {};

    static void yield(unsigned k) noexcept {
        unsigned const sleep_every = 1024;
        k %= sleep_every;

        if (k < 5) {
            unsigned const pause_count = 1u << k;
            for (unsigned i = 0; i < pause_count; ++i) {
                cpu_pause();
            }
        }
        else if (k < sleep_every - 1) {
            std::this_thread::yield();
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

public:
    bool try_lock_shared() noexcept {
        std::uint32_t st = state_.load(std::memory_order_relaxed);

        if (st >= reader_lock_count_mask) {
            return false;
        }

        std::uint32_t newst = st + 1;
        return state_.compare_exchange_strong(st, newst, std::memory_order_acquire, std::memory_order_relaxed);
    }

    void lock_shared() noexcept {
        for (unsigned k = 0; ; ++k) {
            std::uint32_t st = state_.load(std::memory_order_relaxed);

            if (st < reader_lock_count_mask) {
                std::uint32_t newst = st + 1;
                if (state_.compare_exchange_weak(st, newst, std::memory_order_acquire, std::memory_order_relaxed)) 
                    return;
            }

            yield(k);
        }
    }

    void unlock_shared() noexcept {
        state_.fetch_sub(1, std::memory_order_release);
    }

    bool try_lock() noexcept {
        std::uint32_t st = state_.load(std::memory_order_relaxed);

        if ((st & locked_exclusive_mask) || (st & reader_lock_count_mask)) {
            return false;
        }

        std::uint32_t newst = locked_exclusive_mask;
        return state_.compare_exchange_strong(st, newst, std::memory_order_acquire, std::memory_order_relaxed);
    }

    void lock() noexcept {
        for (unsigned k = 0; ; ++k) {
            std::uint32_t st = state_.load(std::memory_order_relaxed);

            if (st & locked_exclusive_mask) {
            }
            else if ((st & reader_lock_count_mask) == 0) {
                std::uint32_t newst = locked_exclusive_mask;
                if (state_.compare_exchange_weak(st, newst, std::memory_order_acquire, std::memory_order_relaxed)) 
                    return;
            }
            else if (st & writer_pending_mask) {
            }
            else {
                std::uint32_t newst = st | writer_pending_mask;
                state_.compare_exchange_weak(st, newst, std::memory_order_relaxed, std::memory_order_relaxed);
            }

            yield(k);
        }
    }

    void unlock() noexcept {
        state_.store(0, std::memory_order_release);
    }
};

} // namespace foa
} // namespace detail
} // namespace unordered
} // namespace boost

#endif // BOOST_UNORDERED_DETAIL_FOA_RW_SPINLOCK_HPP_INCLUDED
