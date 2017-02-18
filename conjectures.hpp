#include "conjectures/full.hpp"
#include "conjectures/stability.hpp"

namespace conjectures {
template <pos_t size, typename Impl>
using All = Stability<size, Full<size, Impl>>;
};
