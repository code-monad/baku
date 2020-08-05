#include <baku/meta.hh>

#include <fmt/format.h>

#include <fstream>


nlohmann::json baku::to_detail(const baku::meta& info) {
    auto result =  nlohmann::json{
		{"id", info.short_id},
		{"hash", info.sha},
		{"chunk_hash", info.chunk_sha},
		{"url", info.url},
		{"size", info.size},
		{"type", info.type},
		{"name", info.name}
	};
	
	if(info.chunk_sha.empty() || info.sha == info.chunk_sha) {
		result.erase("chunk_hash");
	}
	return result;
}


void baku::to_json(nlohmann::json& json, const baku::meta& info) {
	json = to_detail(info);
	if(!info.chunks.empty()) {
		json["chunks"] = nlohmann::json::array();
		for(auto& chunk: info.chunks) {
			json["chunks"].push_back(to_detail(chunk));
		}
	}
}

void baku::from_detail(const nlohmann::json& json, baku::meta& info) {
	json.at("id").get_to(info.short_id);
	json.at("hash").get_to(info.sha);
	if(json.contains("chunk_hash"))
		json.at("chunk_hash").get_to(info.chunk_sha);
	json.at("url").get_to(info.url);
	json.at("size").get_to(info.size);
	json.at("name").get_to(info.name);
	json.at("type").get_to<baku::BAKU_TYPE>(info.type);
}

void baku::from_json(const nlohmann::json& json, baku::meta& info) {
	from_detail(json, info);
	if(json.contains("chunks")) {
		for(auto& chunk: json["chunks"]) {
			baku::meta chunk_meta;
		    from_detail(chunk, chunk_meta);
			info.chunks.push_back(chunk_meta);
		}
	}
}


baku::metainfo::metainfo(const std::filesystem::path& source){
	if(source.empty()) {
		throw std::logic_error("Path is empty. Use default initialization and metainfo::parse_raw if you do not want read from file");
	}
	std::string raw;
	std::ifstream reader(source, reader.in | reader.binary);
	if(reader.is_open()){
		raw = std::string((std::istreambuf_iterator<char>(reader)), std::istreambuf_iterator<char>());
	}
	parse_raw(raw);
}

baku::metainfo::~metainfo() {}

baku::metainfo baku::metainfo::parse(const std::filesystem::path& source) {
	return metainfo(source);
}

void baku::metainfo::parse_raw(const std::string& raw) {
	_raw = raw;
	if(!_raw.empty()) {
		_parsed = nlohmann::json::parse(raw);
		for(auto& info : _parsed["data"]) {
			add(info.get<baku::meta>());
		}
	}
}

void baku::metainfo::add(const baku::meta& item, const bool to_raw) {
	if(to_raw) _parsed["data"].push_back(item);
	_detail[item.short_id] = item;
}

const bool baku::metainfo::contains(const baku::meta& item) {
	return _detail.find(item.short_id) != _detail.end();
}

std::map<std::string, baku::meta>::iterator baku::metainfo::contains_by_sha(const std::string& sha) {
    for(auto iter = _detail.begin(); iter != _detail.end(); iter++) {
		auto& meta = iter->second;
		if(sha == meta.sha) {
			fmt::print("sha {} appears as an original data. check it by yourself!");
		}
		
		if(sha == meta.chunk_sha) {
			return iter;
		}
	}
	
	return _detail.end();
}

const bool baku::metainfo::contains(const std::string& short_id) {
	return _detail.find(short_id) != _detail.end();
}

const std::string baku::metainfo::get_raw(const unsigned indent) {
	return _parsed.dump(indent);
}
void baku::metainfo::dump(const std::filesystem::path& destination, const unsigned indent) {
	if(!destination.empty()) {
		std::ofstream writer(destination, writer.out| writer.binary| writer.trunc);
		if(writer.is_open()) {
			writer << get_raw(indent);
		}
		writer.close();
	}
}
