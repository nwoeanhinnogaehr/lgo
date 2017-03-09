#include "conjectures/full.hpp"
#include "conjectures/stability.hpp"
#include "conjectures/telomere.hpp"

namespace conjectures {
template <pos_t size, typename Impl, typename = void> struct All;
template <pos_t size, typename Impl> struct All<size, Impl, std::enable_if_t<(size < 8)>> : Telomere<size, Stability<size, Full<size, Impl>>> { };
template <pos_t size, typename Impl> struct All<size, Impl, std::enable_if_t<(size >= 8)>> : Telomere<size, Full<size, Impl>> { };
};
