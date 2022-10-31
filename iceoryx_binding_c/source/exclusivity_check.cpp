
#include "iceoryx_binding_c/internal/exclusivity_check.hpp"

#include <atomic>
#include <stdexcept>
#include <array>

const size_t excc_is_exclusive_size = 16807;
std::array<std::atomic_bool, excc_is_exclusive_size> excc_is_exclusive;

void checkExclusive(void *ptr) {
    if (excc_is_exclusive[(unsigned long)ptr%excc_is_exclusive_size].exchange(true))
        throw std::runtime_error("IOX OPERATION NOT EXCLUSIVE!");
}

void uncheckExclusive(void *ptr) {
    if (!excc_is_exclusive[(unsigned long)ptr%excc_is_exclusive_size].exchange(0))
        throw std::runtime_error("EARLY RELEASE OF EXCLUSIVE CHECKER!");
}