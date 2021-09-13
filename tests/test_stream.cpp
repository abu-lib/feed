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

#include <array>
#include <numeric>
#include <vector>

#include "abu/feed.h"
#include "gtest/gtest.h"

namespace {
auto feed_read(abu::Feed auto& f) {
  auto result = *f;
  ++f;
  return result;
}
}  // namespace

TEST(single_chunk_stream, basic_api_test) {
  abu::feed::stream<std::vector<int>> sut;
  EXPECT_EQ(sut, abu::feed::empty);
  EXPECT_NE(sut, abu::feed::end_of_feed);

  sut.append({1, 2, 3, 4});
  sut.finish();

  EXPECT_NE(sut, abu::feed::empty);
  EXPECT_NE(sut, abu::feed::end_of_feed);

  EXPECT_EQ(feed_read(sut), 1);
  EXPECT_EQ(feed_read(sut), 2);
  EXPECT_EQ(feed_read(sut), 3);
  EXPECT_EQ(feed_read(sut), 4);

  EXPECT_EQ(sut, abu::feed::empty);
  EXPECT_EQ(sut, abu::feed::end_of_feed);
}

TEST(single_chunk_stream, rollback) {
  abu::feed::stream<std::vector<int>> sut;
  sut.append({1, 2, 3, 4});
  sut.finish();

  auto cp = sut.checkpoint();

  EXPECT_EQ(feed_read(sut), 1);
  EXPECT_EQ(feed_read(sut), 2);
  {
    auto cp2 = sut.checkpoint();
    EXPECT_EQ(feed_read(sut), 3);
    sut.rollback(cp2);
    EXPECT_EQ(feed_read(sut), 3);
    sut.rollback(cp2);
    EXPECT_EQ(feed_read(sut), 3);
  }

  sut.rollback(cp);

  EXPECT_EQ(feed_read(sut), 1);
  EXPECT_EQ(feed_read(sut), 2);
  EXPECT_EQ(feed_read(sut), 3);
  EXPECT_EQ(feed_read(sut), 4);

  EXPECT_EQ(sut, abu::feed::empty);
  EXPECT_EQ(sut, abu::feed::end_of_feed);
}

TEST(stream, pre_filled) {
  abu::feed::stream<std::vector<int>> sut;
  EXPECT_EQ(sut, abu::feed::empty);
  EXPECT_NE(sut, abu::feed::end_of_feed);

  sut.append({1, 2});
  sut.append({3, 4});
  sut.finish();

  EXPECT_EQ(feed_read(sut), 1);
  EXPECT_EQ(feed_read(sut), 2);
  EXPECT_EQ(feed_read(sut), 3);
  EXPECT_EQ(feed_read(sut), 4);

  EXPECT_EQ(sut, abu::feed::empty);
  EXPECT_EQ(sut, abu::feed::end_of_feed);
}

TEST(stream, empty_chunks) {
  abu::feed::stream<std::vector<int>> sut;
  EXPECT_EQ(sut, abu::feed::empty);
  EXPECT_NE(sut, abu::feed::end_of_feed);

  sut.append({1, 2});
  sut.append({});
  sut.append({3, 4});
  sut.finish();

  EXPECT_EQ(feed_read(sut), 1);
  EXPECT_EQ(feed_read(sut), 2);
  EXPECT_EQ(feed_read(sut), 3);
  EXPECT_EQ(feed_read(sut), 4);

  EXPECT_EQ(sut, abu::feed::empty);
  EXPECT_EQ(sut, abu::feed::end_of_feed);
}

TEST(stream, move_stream) {
  abu::feed::stream<std::vector<int>> sut;
  EXPECT_EQ(sut, abu::feed::empty);
  EXPECT_NE(sut, abu::feed::end_of_feed);

  sut.append({1, 2});
  sut.append({3, 4});
  sut.finish();

  EXPECT_EQ(feed_read(sut), 1);
  EXPECT_EQ(feed_read(sut), 2);
  EXPECT_EQ(feed_read(sut), 3);

  auto cp = sut.checkpoint();
  auto new_sut = std::move(sut);

  EXPECT_DEATH(*sut, "moved");
  EXPECT_DEATH(sut++, "moved");
  EXPECT_DEATH(++sut, "moved");
  EXPECT_DEATH((void)(sut == abu::feed::empty), "moved");
  EXPECT_DEATH((void)(sut == abu::feed::end_of_feed), "moved");
  EXPECT_DEATH(sut.append({}), "moved");
  EXPECT_DEATH(sut.finish(), "moved");
  EXPECT_DEATH(sut.checkpoint(), "moved");
  EXPECT_DEATH(sut.rollback(cp), "moved");

  EXPECT_EQ(feed_read(new_sut), 4);

  EXPECT_EQ(new_sut, abu::feed::empty);
  EXPECT_EQ(new_sut, abu::feed::end_of_feed);
}

TEST(stream, resume) {
  abu::feed::stream<std::vector<int>> sut;
  sut.append({1, 2});

  EXPECT_EQ(feed_read(sut), 1);
  EXPECT_EQ(feed_read(sut), 2);
  EXPECT_EQ(sut, abu::feed::empty);
  EXPECT_NE(sut, abu::feed::end_of_feed);

  sut.append({3, 4});
  EXPECT_NE(sut, abu::feed::empty);
  EXPECT_NE(sut, abu::feed::end_of_feed);

  EXPECT_EQ(feed_read(sut), 3);
  EXPECT_EQ(feed_read(sut), 4);

  EXPECT_EQ(sut, abu::feed::empty);
  EXPECT_NE(sut, abu::feed::end_of_feed);

  sut.finish();
  EXPECT_EQ(sut, abu::feed::end_of_feed);
}
