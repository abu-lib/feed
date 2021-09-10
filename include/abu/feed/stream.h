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


#ifndef ABU_FEED_STREAM_H
#define ABU_FEED_STREAM_H

#include <memory>
#include <ranges>

#include "abu/feed/debug.h"
#include "abu/feed/tags.h"

namespace abu::feed {

template <typename T>
concept Chunk = std::ranges::forward_range<T> && std::default_initializable<T>;

template <Chunk ChunkT>
class stream;

namespace details_ {
template <Chunk ChunkT>
struct stream_node {
  stream_node() = default;
  stream_node(ChunkT&& in_data) : data(std::move(in_data)) {}

  ChunkT data;
  std::shared_ptr<stream_node<const ChunkT>> next;
  bool is_final_chunk = false;
};

template <Chunk ChunkT>
struct stream_checkpoint {
 private:
  friend class stream<ChunkT>;

  stream_checkpoint(std::ranges::iterator_t<const ChunkT> pos,
                    std::shared_ptr<stream_node<const ChunkT>> chunk)
      : position_(std::move(pos)), current_chunk_(std::move(chunk)) {}

  std::ranges::iterator_t<const ChunkT> position_;
  std::shared_ptr<stream_node<const ChunkT>> current_chunk_;
};
}  // namespace details_

template <Chunk ChunkT>
class stream {
 public:
  using chunk_type = const ChunkT;
  using node_type = details_::stream_node<chunk_type>;
  using checkpoint_type = details_::stream_checkpoint<ChunkT>;

  using iterator_tag = std::input_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = std::ranges::range_value_t<ChunkT>;

  constexpr stream()
      : current_chunk_(std::make_shared<node_type>()),
        position_(std::ranges::begin(current_chunk_->data)),
        chunk_end_(std::ranges::end(current_chunk_->data)),
        tail_(current_chunk_) {}

  constexpr decltype(auto) operator*() const {
    if (position_ == chunk_end_) {
      assume(!!current_chunk_->next);
      current_chunk_ = current_chunk_->next;
      position_ = std::ranges::begin(current_chunk_->data);
      chunk_end_ = std::ranges::end(current_chunk_->data);
    }
    return *position_;
  }

  constexpr stream& operator++() {
    ++position_;
    if (position_ == chunk_end_ && current_chunk_->next) {
      current_chunk_ = current_chunk_->next;
      position_ = std::ranges::begin(current_chunk_->data);
      chunk_end_ = std::ranges::end(current_chunk_->data);
    }
    return *this;
  }

  constexpr void operator++(int) {
    ++(*this);
  }

  constexpr bool operator==(const empty_feed_t&) const {
    return position_ == chunk_end_ && !current_chunk_->next;
  }

  constexpr bool operator==(const end_of_feed_t&) const {
    return position_ == chunk_end_ && current_chunk_->is_final_chunk;
  }

  void append(chunk_type&& chunk) {
    auto new_node =
        std::make_shared<details_::stream_node<chunk_type>>(std::move(chunk));
    tail_->next = new_node;
    tail_ = std::move(new_node);
  }

  void finish() {
    tail_->is_final_chunk = true;
  }

  constexpr checkpoint_type checkpoint() {
    return checkpoint_type{position_, current_chunk_};
  }

  constexpr void rollback(checkpoint_type cp) {
    position_ = std::move(cp.position_);
    current_chunk_ = std::move(cp.current_chunk_);
    chunk_end_ = std::ranges::end(current_chunk_->data);
  }

 private:
  using chunk_iterator_type = std::ranges::iterator_t<const ChunkT>;
  using sentinel_type = std::ranges::sentinel_t<const ChunkT>;

  mutable std::shared_ptr<node_type> current_chunk_;
  mutable chunk_iterator_type position_;
  mutable sentinel_type chunk_end_;

  std::shared_ptr<node_type> tail_;
};

}  // namespace abu::feed

#endif