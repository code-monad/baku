#ifndef __BAKU_META_HH__
#define __BAKU_META_HH__

#include <baku/core/common.hh>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>

namespace baku {

struct meta {
  BAKU_TYPE type = BAKU_TYPE::dummy;
  std::string short_id{};  // shor hash -- generated from sha
  std::string sha{};       // sha hash
  std::string chunk_sha{};
  std::string name{};
  std::string url{};
  unsigned long long size;
  unsigned long timestamp{};
  std::vector<meta> chunks{};
};

nlohmann::json to_detail(const baku::meta& info);
void from_detail(const nlohmann::json& json, baku::meta& info);

void to_json(nlohmann::json& json, const meta&);
void from_json(const nlohmann::json& json, meta&);

class metainfo {
 public:
  static metainfo parse(const std::filesystem::path& source);
  metainfo() = default;
  metainfo(const std::filesystem::path& source);
  ~metainfo();
  void add(const meta& item, const bool to_raw = false);
  void parse_raw(const std::string& raw);
  const std::string get_raw(const unsigned indent = 0);
  const bool contains(const meta& item);
  const bool contains(const std::string& short_id);
  std::map<std::string, meta>::iterator contains_by_sha(const std::string& sha);
  void dump(const std::filesystem::path& destination,
            const unsigned indent = 2);
  auto end() { return _detail.end(); }

 private:
  nlohmann::json _parsed;
  std::string _raw;
  std::map<std::string, meta> _detail;
};
}  // namespace baku

#endif
