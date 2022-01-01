#ifndef SPONGE_LIBSPONGE_SLIDING_WINDOW_HH
#define SPONGE_LIBSPONGE_SLIDING_WINDOW_HH

#include <set>
#include <vector>

class SlidingWindow {
  public:
    SlidingWindow(size_t init_window_size);

    size_t get_slidable_size() const;

    uint64_t left_edge() const { return _left_edge; }

    size_t slide_window(size_t slide_size);

    std::vector<std::pair<uint64_t, uint64_t>> fill_into(uint64_t block_begin, uint64_t block_end);

    void reset_window_size(size_t window_size);

    size_t window_size() const { return _window_size; }

  private:
    std::vector<std::pair<uint64_t, uint64_t>> get_uncovered_subblocks(uint64_t block_begin, uint64_t block_end) const;

    size_t _window_size;
    uint64_t _left_edge;
    std::set<uint64_t> _block_begins;
    std::set<uint64_t> _block_ends;
};

#endif