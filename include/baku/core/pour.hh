#ifndef __BAKU_CORE_POUR_HH__
#define __BAKU_CORE_POUR_HH__

#include <baku/core/common.hh>
#include <cstdint>
#include <vector>

namespace baku {

std::vector<std::uint8_t>& decode_png(std::vector<uint8_t>& decode_buffer,
                                      const std::vector<uint8_t>& raw_content,
                                      unsigned width,
                                      unsigned long long actual_size = 0);

std::vector<std::uint8_t> pour(const std::vector<uint8_t>& raw_content,
                               BAKU_TYPE type = BAKU_TYPE::png,
                               unsigned long long actual_size = 0);
}  // namespace baku

#endif
