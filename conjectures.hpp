#include "conjectures/full.hpp"
#include "conjectures/stability.hpp"

namespace conjectures {
template <pos_t size, typename Impl, typename = void> struct All;
template <pos_t size, typename Impl> struct All<size, Impl, std::enable_if_t<(size < 8)>> : Stability<size, Full<size, Impl>> { };
template <pos_t size, typename Impl> struct All<size, Impl, std::enable_if_t<(size >= 8)>> : Full<size, Impl> { };
};
