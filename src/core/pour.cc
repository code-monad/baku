#include <fmt/core.h>
#include <fmt/ranges.h>
#include <lodepng.h>

#include <baku/core/common.hh>
#include <baku/core/pour.hh>
#include <cstdint>
#include <stdexcept>

std::vector<std::uint8_t>& baku::decode_png(
    std::vector<std::uint8_t>& decode_buffer,
    const std::vector<uint8_t>& raw_content, unsigned width,
    unsigned long long actual_size) {
  return decode_buffer;
}

std::vector<std::uint8_t> baku::pour(
    const std::vector<std::uint8_t>& raw_content, baku::BAKU_TYPE type,
    unsigned long long actual_size) {
  std::vector<std::uint8_t> result;
  switch (type) {
    case baku::BAKU_TYPE::png: {
      std::vector<std::uint8_t> image{};
      unsigned width, height;
      if (lodepng::decode(image, width, height, raw_content)) {
        throw std::runtime_error("Failed to decode buffer as PNG.");
      }

      decode_png(result, image, width, actual_size);

      fmt::print("SIZE:{}, acutal:{}", result.size(), actual_size);
    } break;
    case baku::BAKU_TYPE::fake:
      std::copy_n(raw_content.crbegin(), actual_size,
                  std::back_inserter(result));
      break;
    case baku::BAKU_TYPE::dummy:
      return raw_content;  // if a dummy buffer, just return it
      break;
    default:
      break;
  }

  return result;
}
