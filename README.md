# Abu Feed

[![CI](https://github.com/abu-lib/feed/actions/workflows/ci.yml/badge.svg)](https://github.com/abu-lib/feed/actions/workflows/ci.yml)

This is part of the [Abu](http://github.com/abu-lib/abu) meta-project.

## What are feeds?

1) Feeds are input ranges.
2) Feeds distinguish being out of data and having reached the end of the data.
3) Feeds can be rolled-back
3) Feeds can be created from:
    - an input/sentinel iterator pair
    - a sequence of "chunks"

## Why feeds?

Feeds were implemented to support resumable parsers in 
[abu-parse](http://github.com/FrancoisChabot/abu-parse).

Specifically, they allows for code that can transparently ingest complete or 
partial ranges, which is what lets `abu-parse` offer both recursive descent and
state-machine based parsers from a single codebase.

## Reading from feeds

On the consumer side of things, feeds behave mostly like a bog-standard input
iterator. On top of that:
- It supports 2 different sentinel values:
    - `abu::feed::empty` means that the feed has currently run out of data, but *may* 
       be resumable eventually.
    - `abu::feed::end_of_feed` means that the feed has reached the true end of the 
       data.
- `checkpoint()` and `rollback(checkpoint)` 

You can use the `abu::Feed` and `abu::FeedOf<T>` concepts to constrain consumers.

Example:

```cpp
void consumer(abu::FeedOf<int> auto& data) {
    auto checkpoint = data.checkpoint();

    while(data != abu::feed::empty) {
        std::cout << "reading " << *data << "\n";

        if(*data == 0) {
            data = checkpoint;
            throw std::runtime_error("we hit a 0");
        }

        ++data;
    }

    if(data == abu::feed::end_of_feed) {
        std::cout << "end of the feed reached!\n";
    }
}
```

In practice, `abu-feed` really shines when dealing with stateful and 
interuptible processes. Which would typically look like this:

```
template<abu::Feed FeedT>
struct some_process {
    using value_type = std::iter_value_type<FeedT>;
    
    FeedT& feed;

    bool resume() {
        while(feed != abu::feed::empty) {
            // ...
        }
    }
};
```

## Building feeds

### Adapted ranges

`auto adapt_range(range);` and `auto adapt_range(iterator, sentinel);` will 
create a feed that wraps the passed range.

```
int main() {
    std::vector<int> vec = {1, 2, 3, 4};

    // Will use iterators as checkpoints.
    auto vec_feed = abu::feed::adapt_range(vec);
    consumer(vec_feed);

    // Will maintain a minimally required rollback buffer.
    auto cin_feed = abu::feed::adapt_range(
        std::istream_iterator<int>{std::cin}, 
        std::istream_iterator<int>{}
    );

    consumer(cin_feed);
}
```
### Streams

Streams present a series of "chunks" as a feed. The stream will take ownership
of the chunk and will destroy them once they can be guranteed to not be needed
anymore.

```
template<std::ranges::forward_range Chunk>
class stream {
public:
    void append(Chunk&& chunk);
    void finish();

    /* Feed interface */
};
```

A few notes on streams:
- Added chunks are let go as soon as no rollbacks to them is possible. If memory
  usage is a concern, consider adding smaller chunks more frequently.

## FAQ

### Why are feeds not forward ranges?

They used to be, but that required some unfortunate compromises. Feeds are meant
to go at the heart 

### What if I don't want a stream to detroy the data once it's done with it?

Use a proxy with shared (or without any) ownership of the underlying data. 

```
struct my_chunk {
    std::shared_ptr<std::vector> data;

    auto begin() const { return data->cbegin();}
    auto end() const { return data->cend();}
};
```