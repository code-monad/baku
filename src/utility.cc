#include <baku/utility.hh>
#include <baku/version.hh>
#include <fmt/format.h>
#include <zip.hpp>
#include <regex>
baku::args::args(std::string ext) : _default_ext{ext},  _program("baku", baku::VERSION) {
  _program.add_argument("-p", "--pour")
    .help("run in pour mode")
    .default_value(false)
    .implicit_value(true);

  _program.add_argument("-i", "--input", "input")
    .help("input target")
    .required();

  _program.add_argument("-o", "--output", "output")
    .help("output destination");

  _program.add_argument("-c", "--cache", "cache")
    .help("cache destination");

  _program.add_argument("-m", "--multipler", "multipler")
    .help("fake base");
}

baku::args::~args() {}

void baku::args::parse(int argc, char** argv) {
  _program.parse_args(argc, argv);
  _mode = (_program["--pour"] == true)? baku::mode_t::POUR : baku::mode_t::FEED;
  _in = std::filesystem::path(_program.get<std::string>("input"));
  if(auto out = _program.present("output")) {
    _out = std::filesystem::path(*out);
  } else {
    _out = "out.png";
  }

  if(auto cache = _program.present("cache")) {
    _cache = std::filesystem::path(*cache);
  }

   if(auto fake = _program.present("multipler")) {
    _fake = std::filesystem::path(*fake);
  }
}

const std::filesystem::path
baku::bundle_files(const std::filesystem::path &source,
                   const std::filesystem::path &pre_dest,
		   std::function<const std::string(const std::filesystem::path& source)> encode_fn) {
  std::filesystem::path dst = pre_dest;
  

  try {
    if(std::filesystem::is_directory(source)) {
      if(pre_dest.empty())
	dst = fmt::format("{}{}",source.parent_path().string(), ".zip");

      else if(std::filesystem::is_directory(pre_dest)) {
	dst /= fmt::format("{}{}",source.parent_path().string(), ".zip");
      }
      
      libzip::archive archive(dst.string(), ZIP_CREATE|ZIP_TRUNCATE);
      auto walk_info = baku::walk_dir(source);
      fmt::print("Bunded {} files.\n", walk_info.size());
      fmt::print("Caching into {}...\n", dst.string());
      for(auto const& [src_path, dst_path]: walk_info) {
	archive.add(libzip::source_buffer(encode_fn(src_path)), dst_path);
      }
    } else {
      if(dst.empty()) {
	dst = source;
	dst.replace_extension(fmt::format("{}.chunk",dst.extension().string()));
      }
      if(!dst.parent_path().empty()) {
	std::filesystem::create_directories(dst.parent_path());
      }
      
      if(std::filesystem::is_directory(dst)) {
	dst /= fmt::format("{}.chunk", source.string());
      } else {
	
      }

      std::ofstream writer(dst, writer.out | writer.binary | writer.trunc);
      if(writer.is_open()) {
	auto encoded = encode_fn(source);
	fmt::print("dst:{}\ndata_preview:{}",dst.string(), encoded.size() > 100? encoded.substr(0,100): encoded);
	fmt::print("{}\n", encoded.size() > 100? "...":"");
	writer << encoded;
      }
      writer.close();
    }
  } catch(const std::runtime_error& ex) {
    fmt::print("Failed while creating zip {}:{}", dst.string(), ex.what());
    return {};
  }
  
  return dst;
}

const std::map<std::filesystem::path, std::string>
baku::walk_dir(const std::filesystem::path &target) {
  std::map<std::filesystem::path, std::string> result;
  for(auto& p: std::filesystem::recursive_directory_iterator(target)) {
    if(!std::filesystem::is_directory(p)) {
      std::string zip_path = p.path();
      
      //std::replace(zip_path.begin(), zip_path.end(), "/", "//");
      zip_path = std::regex_replace(zip_path, std::regex("\\/"), "//");
      fmt::print("{}\n", zip_path);
      result[p.path()] = zip_path;
    }
  }
  return result;
}
