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

#ifndef ABU_FEED_TAGS_H_INCLUDED
#define ABU_FEED_TAGS_H_INCLUDED

#include <concepts>

namespace abu::feed {

// ***** Universal feed sentinels *****

// This sentinel denotes that the feed has run out of data
struct empty_feed_t {};
constexpr empty_feed_t empty;

// This sentinel denotes that the feed has reached its end.
struct end_of_feed_t {};
constexpr end_of_feed_t end_of_feed;

}  // namespace abu::feed

#endif