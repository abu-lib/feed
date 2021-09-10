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

TEST(single_chunk_stream, basic_api_test) {
  abu::feed::stream<std::vector<int>> sut;
  EXPECT_EQ(sut, abu::feed::empty);
  EXPECT_NE(sut, abu::feed::end_of_feed);

  sut.append({1, 2, 3, 4});
  sut.finish();

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

TEST(single_chunk_stream, rollback) {
  abu::feed::stream<std::vector<int>> sut;
  sut.append({1, 2, 3, 4});
  sut.finish();

  auto cp = sut.checkpoint();

  EXPECT_EQ(*sut, 1);
  EXPECT_EQ(*++sut, 2);
  {
    auto cp2 = sut.checkpoint();
    EXPECT_EQ(*++sut, 3);
    sut.rollback(cp2);
    EXPECT_EQ(*++sut, 3);
    sut.rollback(cp2);
    EXPECT_EQ(*++sut, 3);
    sut.rollback(cp2);
    EXPECT_EQ(*++sut, 3);
  }

  sut.rollback(cp);

  EXPECT_EQ(*sut, 1);
  EXPECT_EQ(*++sut, 2);
  EXPECT_EQ(*++sut, 3);
  EXPECT_EQ(*++sut, 4);

  EXPECT_EQ(++sut, abu::feed::empty);
  EXPECT_EQ(sut, abu::feed::end_of_feed);
}

TEST(multi_chunks_stream, basic_api_test) {
  abu::feed::stream<std::vector<int>> sut;
  EXPECT_EQ(sut, abu::feed::empty);
  EXPECT_NE(sut, abu::feed::end_of_feed);

  sut.append({1, 2});
  sut.append({3, 4});
  sut.finish();

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

TEST(multi_chunks_stream, rollback) {
  abu::feed::stream<std::vector<int>> sut;
  sut.append({1, 2});
  sut.append({3, 4});
  sut.finish();

  auto cp = sut.checkpoint();

  EXPECT_EQ(*sut, 1);
  EXPECT_EQ(*++sut, 2);
  {
    auto cp2 = sut.checkpoint();
    EXPECT_EQ(*++sut, 3);
    sut.rollback(cp2);
    EXPECT_EQ(*++sut, 3);
    sut.rollback(cp2);
    EXPECT_EQ(*++sut, 3);
    sut.rollback(cp2);
    EXPECT_EQ(*++sut, 3);
  }

  sut.rollback(cp);

  EXPECT_EQ(*sut, 1);
  EXPECT_EQ(*++sut, 2);
  EXPECT_EQ(*++sut, 3);
  EXPECT_EQ(*++sut, 4);

  EXPECT_EQ(++sut, abu::feed::empty);
  EXPECT_EQ(sut, abu::feed::end_of_feed);
}

TEST(streaming_chunks_stream, basic_api_test) {
  abu::feed::stream<std::vector<int>> sut;
  EXPECT_EQ(sut, abu::feed::empty);
  EXPECT_NE(sut, abu::feed::end_of_feed);

  sut.append({1, 2});

  EXPECT_NE(sut, abu::feed::empty);
  EXPECT_EQ(*sut, 1);

  EXPECT_NE(sut, abu::feed::empty);
  EXPECT_EQ(*++sut, 2);

  EXPECT_EQ(++sut, abu::feed::empty);
  EXPECT_NE(sut, abu::feed::end_of_feed);
  sut.append({3, 4});
  EXPECT_NE(sut, abu::feed::empty);
  EXPECT_NE(sut, abu::feed::end_of_feed);

  EXPECT_EQ(*sut, 3);

  EXPECT_NE(sut, abu::feed::empty);
  EXPECT_EQ(*++sut, 4);

  EXPECT_EQ(++sut, abu::feed::empty);
  EXPECT_NE(sut, abu::feed::end_of_feed);

  sut.finish();
  EXPECT_EQ(sut, abu::feed::end_of_feed);
}

TEST(streaming_chunks_stream, rollback) {
  abu::feed::stream<std::vector<int>> sut;
  sut.append({1, 2});

  auto cp = sut.checkpoint();

  EXPECT_EQ(*sut, 1);
  EXPECT_EQ(*++sut, 2);
  {
    auto cp2 = sut.checkpoint();
    sut.append({3, 4});

    EXPECT_EQ(*++sut, 3);
    sut.rollback(cp2);
    EXPECT_EQ(*++sut, 3);
    sut.rollback(cp2);
    EXPECT_EQ(*++sut, 3);
    sut.rollback(cp2);
    EXPECT_EQ(*++sut, 3);
  }
  sut.finish();
  sut.rollback(cp);

  EXPECT_EQ(*sut, 1);
  EXPECT_EQ(*++sut, 2);
  EXPECT_EQ(*++sut, 3);
  EXPECT_EQ(*++sut, 4);

  EXPECT_EQ(++sut, abu::feed::empty);
  EXPECT_EQ(sut, abu::feed::end_of_feed);
}