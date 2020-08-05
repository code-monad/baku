#include <fmt/format.h>

#include <baku/utility.hh>
#include <baku/version.hh>
#include <chrono>
#include <regex>
#include <thread>
#include <zip.hpp>

baku::args::args(std::string ext)
    : _mode{baku::mode_t::DUMMY},
      _verbose{false},
      _force{true},
      _default_ext{ext},
      _program("baku", baku::VERSION) {
  _program.add_description(
      "Make raw contents into PNG format, and do with the encoded data "
      "reversely.");
  _program.add_argument("-p", "--pour")
      .help("run in pour mode")
      .default_value(false)
      .implicit_value(true);

  _program.add_argument("-i", "--input", "input").help("input target");

  _program.add_argument("-o", "--output", "output").help("output destination");

  _program.add_argument("-c", "--cache", "cache").help("cache destination");

  _program.add_argument("-m", "--multipler", "multipler").help("fake base");

  _program.add_argument("--metainfo")
      .help("metainfo of encoded objects.(jsonfile)");

  _program.add_argument("--verbose")
      .default_value(false)
      .implicit_value(true)
      .help("turn on verbose");

  _program.add_argument("--force")
      .default_value(false)
      .implicit_value(true)
      .help("force operation for ignored step.");
}

baku::args::~args() {}

void baku::args::parse(int argc, char** argv) {
  _program.parse_args(argc, argv);

  if (auto in = _program.present("input")) {
    _in = std::filesystem::path(*in);
  } else {
    _in = "-";
    _mode = baku::mode_t::RUA;
  }

  if (_mode == baku::mode_t::DUMMY) {
    _mode =
        (_program["--pour"] == true) ? baku::mode_t::POUR : baku::mode_t::FEED;
  }

  _verbose = _program["--verbose"] == true;

  _force = _program["--force"] == true;

  if (auto out = _program.present("output")) {
    _out = std::filesystem::path(*out);
  } else {
    _out = "out.png";
  }

  if (auto cache = _program.present("cache")) {
    _cache = std::filesystem::path(*cache);
  }

  if (auto fake = _program.present("multipler")) {
    _fake = std::filesystem::path(*fake);
  }

  if (auto meta = _program.present("--metainfo")) {
    _meta = std::filesystem::path(*meta);
  } else {
    _meta = "baku.meta";
  }
}

const std::filesystem::path baku::bundle_files(
    const std::filesystem::path& source, const std::filesystem::path& pre_dest,
    const bool verbose, const bool encode_single,
    std::function<const std::string(const std::filesystem::path& source)>
        encode_fn) {
  std::filesystem::path dst = pre_dest;

  try {
    if (std::filesystem::is_directory(source)) {
      if (pre_dest.empty())
        dst = fmt::format("{}{}", source.parent_path().string(), ".zip");

      else if (std::filesystem::is_directory(pre_dest)) {
        dst /= fmt::format("{}{}", source.parent_path().string(), ".zip");
      }

      libzip::archive archive(dst.string(), ZIP_CREATE | ZIP_TRUNCATE);
      auto walk_info = baku::walk_dir(source, verbose);

      fmt::print("Bunded {} files.\n", walk_info.size());
      if (verbose) {
        fmt::print("Caching into {}...\n", dst.string());
      }

      for (auto const& [src_path, dst_path] : walk_info) {
        archive.add(libzip::source_buffer(encode_fn(src_path)), dst_path);
      }

    } else {
      if (dst.empty()) {
        dst = source;
        dst.replace_extension(
            fmt::format("{}.chunk", dst.extension().string()));
      }
      if (!dst.parent_path().empty()) {
        std::filesystem::create_directories(dst.parent_path());
      }

      if (std::filesystem::is_directory(dst)) {
        dst /= fmt::format("{}.chunk", source.string());
      }

      std::ofstream writer(dst, writer.out | writer.binary | writer.trunc);
      if (writer.is_open()) {
        std::string encoded{};
        if (encode_single)
          encoded = encode_fn(source);
        else
          encoded = dummy_encode_fn(source);
        if (verbose) {
          fmt::print("dst:{}\ndata_preview:\n========\n{}", dst.string(),
                     encoded.size() > 100 ? encoded.substr(0, 100) : encoded);
          fmt::print("{}========\n", encoded.size() > 100 ? "...\n" : "");
        }
        writer << encoded;
      }
      writer.close();
    }
  } catch (const std::runtime_error& ex) {
    fmt::print("Failed while creating zip {}:{}", dst.string(), ex.what());
    return {};
  }

  return dst;
}

const std::map<std::filesystem::path, std::string> baku::walk_dir(
    const std::filesystem::path& target, const bool verbose) {
  std::map<std::filesystem::path, std::string> result;
  for (auto& p : std::filesystem::recursive_directory_iterator(target)) {
    if (!std::filesystem::is_directory(p)) {
      std::string zip_path = p.path();

      // std::replace(zip_path.begin(), zip_path.end(), "/", "//");
      zip_path = std::regex_replace(zip_path, std::regex("\\/"), "//");
      if (verbose) fmt::print("{}\n", zip_path);
      result[p.path()] = zip_path;
    }
  }
  return result;
}

const std::vector<std::uint8_t> baku::fake_generation(
    const std::vector<std::uint8_t>& fake_buffer,
    const std::vector<std::uint8_t>& buffer) {
  std::vector<std::uint8_t> result_buffer = fake_buffer;
  std::copy(buffer.crbegin(), buffer.crend(),
            std::back_inserter(result_buffer));
  return result_buffer;
}

const std::string baku::to_shorten(const std::string& sha) {
  std::string shortened;
  if (!sha.empty()) {
    auto fnv = baku::hash::fnv1a<std::uint32_t>::hash(sha.c_str());
    while (fnv) {
      shortened.push_back(baku::ASCII[fnv % 62]);
      fnv /= 62;
    }
    std::reverse(shortened.begin(), shortened.end());
  }
  return shortened;
}
