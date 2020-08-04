#include <baku/core/feed.hh>

#include <cstdint>
#include <selene/img_ops/Generate.hpp>
#include <selene/img_io/IO.hpp>
#include <selene/base/io/FileUtils.hpp>
#include <selene/img/interop/ImageToDynImage.hpp>
#include <cmath>

const std::vector<std::uint8_t> baku::feed(const std::vector<std::uint8_t>& buffer, const std::filesystem::path& target) {
  unsigned width = 10, height = 10;
  if(buffer.size() < 100) {
    width = 10;
  }
  if(buffer.size() < 500) {
    width = 20;
  } else if(buffer.size() < 1000) {
    width = 100;
  } else {
    width = std::sqrt(buffer.size());
  }
  height = static_cast<float>(buffer.size()) / static_cast<float>(width) + 0.5f;
  auto gen = baku::generate_fn(buffer, width);
  auto img_rgb = sln::generate(gen, sln::to_pixel_length(width), sln::to_pixel_length(height));
  std::vector<std::uint8_t> encoded_buffer{};
  sln::write_image(sln::to_dyn_image_view(img_rgb), sln::ImageFormat::PNG,
		   sln::VectorWriter(encoded_buffer));
  if(!target.empty()) {
    sln::write_data_contents(target.string(), encoded_buffer);
  }
  return encoded_buffer;  
}
