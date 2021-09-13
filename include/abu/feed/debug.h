
// Copyright 2021 Francois Chabot

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef ABU_MEM_FEED_H_INCLUDED
#define ABU_MEM_FEED_H_INCLUDED

#include <source_location>

#include "abu/debug.h"

#ifndef ABU_FEED_CHECK_PRECONDITIONS
#ifdef NDEBUG
#define ABU_FEED_CHECK_PRECONDITIONS false
#else
#define ABU_FEED_CHECK_PRECONDITIONS true
#endif
#endif

#ifndef ABU_FEED_CHECK_ASSUMPTIONS
#define ABU_FEED_CHECK_ASSUMPTIONS false
#endif

namespace abu::feed {
namespace details_ {
constexpr ::abu::debug::config dbg_cfg = {
    .check_assumptions = ABU_FEED_CHECK_ASSUMPTIONS,
    .check_preconditions =
        ABU_FEED_CHECK_PRECONDITIONS || ABU_FEED_CHECK_ASSUMPTIONS};
}

inline constexpr void assume(bool condition,
                             std::string_view msg = {},
                             const abu::debug::source_location location =
                                 abu::debug::source_location::current()) {
  return abu::debug::assume<details_::dbg_cfg>(condition, msg, location);
}

inline constexpr void precondition(bool condition,
                                   std::string_view msg = {},
                                   const abu::debug::source_location location =
                                       abu::debug::source_location::current()) {
  return abu::debug::precondition<details_::dbg_cfg>(condition, msg, location);
}
}  // namespace abu::feed

#endif
