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

#include <optional>
#include <ranges>

#include "abu/feed/debug.h"
#include "abu/feed/tags.h"
#include "abu/mem.h"

namespace abu::feed {

template <typename T>
concept Chunk = std::ranges::forward_range<T>;

template <Chunk ChunkT>
class stream;

namespace details_ {
template <Chunk ChunkT>
struct stream_node {
  static_assert(std::is_const_v<ChunkT>);

  stream_node() = default;
  stream_node(ChunkT&& in_data) : data_(std::move(in_data)) {}

  std::ranges::iterator_t<ChunkT> begin() const {
    if (data_) {
      return std::ranges::begin(*data_);
    }
    return {};
  }

  std::ranges::sentinel_t<ChunkT> end() const {
    if (data_) {
      return std::ranges::end(*data_);
    }
    return {};
  }

  void mark_final() {
    assume(!next_ && !is_final_);
    is_final_ = true;
  }

  bool is_final() const {
    return is_final_;
  }

  void set_next(mem::ref_count_ptr<stream_node<ChunkT>> next) {
    assume(!next_ && !is_final_);
    next_ = next;
  }

  const mem::ref_count_ptr<stream_node<ChunkT>>& next() const {
    return next_;
  }

 private:
  mem::ref_count_ptr<stream_node<ChunkT>> next_;
  bool is_final_ = false;
  std::optional<ChunkT> data_;
};

template <Chunk ChunkT>
struct stream_checkpoint {
 private:
  friend class stream<ChunkT>;

  stream_checkpoint(std::ranges::iterator_t<const ChunkT> pos,
                    mem::ref_count_ptr<stream_node<const ChunkT>> chunk)
      : position_(std::move(pos)), current_chunk_(std::move(chunk)) {}

  std::ranges::iterator_t<const ChunkT> position_;
  mem::ref_count_ptr<stream_node<const ChunkT>> current_chunk_;
};
}  // namespace details_

template <Chunk ChunkT>
class stream {
  static constexpr const char* moved_err_msg =
      "Using stream feed that was moved";

 public:
  using chunk_type = const ChunkT;
  using node_type = details_::stream_node<chunk_type>;
  using checkpoint_type = details_::stream_checkpoint<ChunkT>;

  using iterator_tag = std::input_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = std::ranges::range_value_t<ChunkT>;

  stream() {
    start_chunk_(mem::make_ref_counted<node_type>());
    tail_ = current_chunk_;
  }

  stream(stream&&) = default;
  stream(const stream&) = delete;

  stream& operator=(stream&&) = default;
  stream& operator=(const stream&) = delete;

  decltype(auto) operator*() const {
    precondition(!is_moved_(), moved_err_msg);
    precondition(position_ != chunk_end_);

    return *position_;
  }

  stream& operator++() {
    precondition(!is_moved_(), moved_err_msg);
    precondition(position_ != chunk_end_ || !current_chunk_->is_final());

    ++position_;
    if (position_ == chunk_end_ && !at_last_chunk_()) {
      start_chunk_(current_chunk_->next());
    }
    return *this;
  }

  void operator++(int) {
    precondition(!is_moved_(), moved_err_msg);
    ++(*this);
  }

  bool operator==(const empty_feed_t&) const {
    precondition(!is_moved_(), moved_err_msg);
    return position_ == chunk_end_ && at_last_chunk_();
  }

  bool operator==(const end_of_feed_t&) const {
    precondition(!is_moved_(), moved_err_msg);
    return position_ == chunk_end_ && current_chunk_->is_final();
  }

  void append(chunk_type&& chunk) {
    precondition(!is_moved_(), moved_err_msg);
    precondition(!tail_->is_final());

    if (std::ranges::empty(chunk)) {
      return;
    }

    auto new_node = mem::make_ref_counted<details_::stream_node<chunk_type>>(
        std::move(chunk));

    if (*this == empty) {
      start_chunk_(new_node);
    }

    tail_->set_next(new_node);
    tail_ = std::move(new_node);
  }

  void finish() {
    precondition(!is_moved_(), moved_err_msg);
    precondition(!tail_->is_final());

    tail_->mark_final();
  }

  checkpoint_type checkpoint() {
    precondition(!is_moved_(), moved_err_msg);
    return checkpoint_type{position_, current_chunk_};
  }

  void rollback(checkpoint_type cp) {
    precondition(!is_moved_(), moved_err_msg);

    position_ = std::move(cp.position_);
    current_chunk_ = std::move(cp.current_chunk_);
    chunk_end_ = current_chunk_->end();

    // This can happen when checkpointing at the end of an empty stream.
    if (position_ == chunk_end_ && !at_last_chunk_()) {
      start_chunk_(current_chunk_->next());
    }
  }

 private:
  bool is_moved_() const {
    return tail_ == nullptr;
  }

  bool at_last_chunk_() const {
    return current_chunk_ == tail_;
  }

  void start_chunk_(mem::ref_count_ptr<node_type> chunk) {
    position_ = chunk->begin();
    chunk_end_ = chunk->end();
    current_chunk_ = std::move(chunk);
  }

  using chunk_iterator_type = std::ranges::iterator_t<node_type>;
  using sentinel_type = std::ranges::sentinel_t<node_type>;

  mem::ref_count_ptr<node_type> current_chunk_;
  chunk_iterator_type position_;
  sentinel_type chunk_end_;

  mem::ref_count_ptr<node_type> tail_;
};

}  // namespace abu::feed

#endif