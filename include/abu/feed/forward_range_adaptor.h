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


#ifndef ABU_FEED_FORWARD_RANGE_ADAPTOR_H
#define ABU_FEED_FORWARD_RANGE_ADAPTOR_H

#include "abu/feed/debug.h"
#include "abu/feed/tags.h"

namespace abu::feed {

template <std::forward_iterator I, std::sentinel_for<I> S>
class forward_range_adaptor {
 public:
  using checkpoint_type = I;

  using iterator_tag = std::input_iterator_tag;
  using difference_type = std::iter_difference_t<I>;
  using value_type = std::iter_value_t<I>;

  constexpr forward_range_adaptor() = default;
  explicit constexpr forward_range_adaptor(I begin, S end)
      : position_(std::move(begin)), end_(std::move(end)) {}

  constexpr decltype(auto) operator*() const {
    precondition(position_ != end_);

    return *position_;
  }

  constexpr forward_range_adaptor& operator++() {
    precondition(position_ != end_);

    ++position_;
    return *this;
  }

  constexpr void operator++(int) {
    ++(*this);
  }

  constexpr bool operator==(const empty_feed_t&) const {
    return position_ == end_;
  }

  constexpr bool operator==(const end_of_feed_t&) const {
    return position_ == end_;
  }

  constexpr checkpoint_type checkpoint() {
    return position_;
  }

  constexpr void rollback(checkpoint_type cp) {
    position_ = std::move(cp);
  }

 private:
  I position_;
  [[no_unique_address]] S end_;
};

}  // namespace abu::feed

#endif