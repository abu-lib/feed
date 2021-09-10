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
#include <vector>

#include "abu/feed.h"
#include "gtest/gtest.h"

TEST(adapted_range, basic_api_test) {
  std::vector<int> raw_data = {1, 2, 3, 4};

  auto sut = abu::feed::adapt_range(std::begin(raw_data), std::end(raw_data));

  EXPECT_NE(sut, abu::feed::empty);
  EXPECT_EQ(*sut, 1);

  EXPECT_NE(sut, abu::feed::empty);
  EXPECT_EQ(*++sut, 2);

  EXPECT_NE(sut, abu::feed::empty);
  EXPECT_EQ(*++sut, 3);

  EXPECT_NE(sut, abu::feed::empty);
  EXPECT_EQ(*++sut, 4);

  EXPECT_EQ(++sut, abu::feed::empty);
  EXPECT_EQ(sut, abu::feed::end_of_feed);
}

TEST(adapted_range, rollback) {
  std::vector<int> raw_data = {1, 2, 3, 4};

  auto sut = abu::feed::adapt_range(std::begin(raw_data), std::end(raw_data));

  auto cp = sut.checkpoint();

  EXPECT_EQ(*sut, 1);
  EXPECT_EQ(*++sut, 2);
  EXPECT_EQ(*++sut, 3);
  sut.rollback(cp);

  EXPECT_EQ(*sut, 1);
  EXPECT_EQ(*++sut, 2);
  EXPECT_EQ(*++sut, 3);
  EXPECT_EQ(*++sut, 4);

  EXPECT_EQ(++sut, abu::feed::empty);
  EXPECT_EQ(sut, abu::feed::end_of_feed);
}

constexpr int cste_feed_test(abu::FeedOf<int> auto f) {
  int accum = 0;
  while (f != abu::feed::empty) {
    accum += *f;
    ++f;
  }
  return accum;
}

static_assert(10 == cste_feed_test(abu::feed::adapt_range(std::array<int, 4>{
                        1, 2, 3, 4})));

constexpr int cste_feed_rollback_test(abu::FeedOf<int> auto f) {
  int accum = 0;
  auto cp = f.checkpoint();
  while (f != abu::feed::empty) {
    accum += *f;
    ++f;
  }
  f.rollback(cp);
  while (f != abu::feed::empty) {
    accum += *f;
    ++f;
  }

  return accum;
}

static_assert(20 ==
              cste_feed_rollback_test(abu::feed::adapt_range(std::array<int, 4>{
                  1, 2, 3, 4})));