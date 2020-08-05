#ifndef __BAKU_CORE_COMMON_HH__
#define __BAKU_CORE_COMMON_HH__

#include <cstddef>

namespace baku {

	struct pos_t{
		std::size_t x = 0;
		std::size_t y = 0;
	};

	enum BAKU_TYPE {
		dummy = 0, // dummy type, content is stored as is
		png = 1, // encode contents into png pixels
		fake = 2, // append contents into png
	};
  
	inline std::size_t pos_to_index(std::size_t x, std::size_t y, std::size_t width) {
		return width * y + x;
	}
	pos_t index_to_pos(std::size_t width, std::size_t index);
} // namespace baku

#endif
