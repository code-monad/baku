#ifndef __BAKU_CORE_POUR_HH__
#define __BAKU_CORE_POUR_HH__

#include <baku/core/common.hh>
#include <vector>

namespace baku {
	std::vector<std::uint8_t> pour(const std::vector<char>& raw_content, BAKU_TYPE type = BAKU_TYPE::png, unsigned long long actual_size = 0);
} // namespace baku

#endif
