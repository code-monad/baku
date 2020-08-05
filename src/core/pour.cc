#include <baku/core/pour.hh>


std::vector<std::uint8_t> baku::pour(const std::vector<char>& raw_content, baku::BAKU_TYPE type, unsigned long long actual_size) {
	std::vector<std::uint8_t> result;
	switch(type) {
	case baku::BAKU_TYPE::png:
		break;
	case baku::BAKU_TYPE::fake:
		std::copy_n(raw_content.crbegin(), actual_size, std::back_inserter(result));
		break;
	case baku::BAKU_TYPE::dummy:
		std::copy(raw_content.cbegin(), raw_content.cend(), std::back_inserter(result));
		break;
	default:
		break;
	}

	return result;
}
