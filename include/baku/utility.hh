#ifndef __BAKU_UTILITY_HH__
#define __BAKU_UTILITY_HH__

#include <argparse/argparse.hpp>

#include <fstream>
#include <filesystem>
#include <functional>
#include <string>
#include <map>


namespace baku {

	enum mode_t {
		DUMMY,
		RUA,
		FEED,
		POUR,
	};
  
	class args{
	public:
		args(std::string ext = ".png");
		~args();
		void parse(int argc, char** argv);
		const mode_t& mode() noexcept { return _mode; }
		const std::filesystem::path& in() noexcept { return _in; }
		const std::filesystem::path& out() noexcept { return _out; }
		const std::filesystem::path& cache() noexcept { return _cache; }
		const std::filesystem::path& fake() noexcept { return _fake; }
	    auto help() { return _program.help(); }
    
	private:
		mode_t _mode;
		std::filesystem::path _in;
		std::filesystem::path _out;
		std::filesystem::path _cache;
		std::filesystem::path _fake;
		std::string _default_ext; // default extension is png
		argparse::ArgumentParser _program;
	};

	constexpr auto dummy_encode_fn =
		[](const std::filesystem::path& source) -> const std::string {
			std::ifstream reader(source, reader.in|reader.binary);
			if(reader.is_open()) {
				return std::string(std::istreambuf_iterator<char>(reader),std::istreambuf_iterator<char>());
			} else {
				return {};
			}
		};
  
	const std::filesystem::path bundle_files(const std::filesystem::path& source,  const std::filesystem::path& pre_dest = {}, const bool encode_single = false, std::function<const std::string(const std::filesystem::path& source)> encode_fn = dummy_encode_fn);

	const std::map<std::filesystem::path, std::string> walk_dir(const std::filesystem::path& target);

	const std::vector<uint8_t> fake_generation(const std::vector<std::uint8_t>& fake_buffer, const std::vector<std::uint8_t>& buffer);
} // namespace baku

#endif
