#ifndef __BAKU_CORE_COMMON_HH__
#define __BAKU_CORE_COMMON_HH__

namespace baku {

  struct pos_t{
    std::size_t x = 0;
    std::size_t y = 0;
  };
  
  inline std::size_t pos_to_index(std::size_t x, std::size_t y, std::size_t width) {
    return width * y + x;
  }
  pos_t index_to_pos(std::size_t width, std::size_t index);
} // namespace baku

#endif
