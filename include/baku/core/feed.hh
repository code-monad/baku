#ifndef __BAKU_CORE_FEED_HH__
#define __BAKU_CORE_FEED_HH__


#include <selene/img/pixel/PixelTypeAliases.hpp>
#include <selene/img/common/Types.hpp>
#include <baku/core/common.hh>
#include <cstdint>
#include <filesystem>
#include <iterator>
#include <vector>

namespace baku {
  constexpr auto generate_fn(const std::vector<std::uint8_t>& buffer, unsigned width) {
    return [&buffer, width](sln::PixelIndex x, sln::PixelIndex y){
	     sln::PixelRGB_8u pix;
	     auto offset = pos_to_index(width, x.value(), y.value());
	     auto start_iter = std::next(buffer.begin(),offset);
	     for(auto iter = start_iter; iter != buffer.end() && iter != std::next(start_iter +3); iter++) {
	       pix[std::distance(start_iter, iter)] = *iter;
	     }
	     return pix;
	   };
  }


  const std::vector<std::uint8_t> feed(const std::vector<std::uint8_t>& buffer, const std::filesystem::path& target = {});
  
} // namespace baku

#endif
