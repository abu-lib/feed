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

#ifndef ABU_FEED_H_INCLUDED
#define ABU_FEED_H_INCLUDED

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-attributes"
#endif

#include <iterator>
#include <ranges>

#include "abu/feed/forward_range_adaptor.h"
#include "abu/feed/stream.h"
#include "abu/feed/tags.h"

namespace abu {

// Defines a feed.
// usage:
//   void process(Feed auto& data) { ... }
template <typename T>
concept Feed =
    std::input_iterator<T> && std::sentinel_for<abu::feed::empty_feed_t, T> &&
    std::sentinel_for<abu::feed::end_of_feed_t, T>;

// A feed constrained to a specific value type
// usage:
//   void process(FeedOf<char> auto& data) { ... }
template <typename T, typename U>
concept FeedOf = Feed<T> && std::same_as<U, std::iter_value_t<T>>;

namespace feed {
  template <std::forward_iterator I, std::sentinel_for<I> S>
  constexpr Feed auto adapt_range(I iterator, S sentinel) {
    return forward_range_adaptor<I, S>{iterator, sentinel};
  }

  template <typename T>
  constexpr Feed auto adapt_range(const T& range) {
    return adapt_range(std::ranges::begin(range), std::ranges::end(range));
  }

}  // namespace feed
}  // namespace abu

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif