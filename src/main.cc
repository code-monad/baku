#include <cstdint>
#include <exception>
#include <iostream>
#include <fmt/core.h>
#include <filesystem>
#include <iterator>
#include <iterator>
#include <cmath>
#include <zip.hpp>
#include <selene/img/pixel/PixelTypeAliases.hpp>
#include <selene/img/typed/ImageTypeAliases.hpp>
#include <selene/img/interop/ImageToDynImage.hpp>
#include <selene/base/io/VectorWriter.hpp>
#include <selene/base/io/FileUtils.hpp>
#include <selene/img_io/IO.hpp>
#include <selene/img_ops/Generate.hpp>
#include <fstream>
#include <nlohmann/json.hpp>
#include <argparse/argparse.hpp>

using json = nlohmann::json;

int main(int argc, char *argv[]) {
  argparse::ArgumentParser program("baku");
  program.add_argument("-m", "mode");
  program.add_argument("-i", "input");
  program.add_argument("-o");
  //try {
    program.parse_args(argc, argv);
    std::string mode = program.present("mode")? program.get("mode"): "feed";
    fmt::print("Running on {} mode\n", mode);

    std::filesystem::path in_path(program.get("-i"));
    if(!std::filesystem::exists(in_path)) {
      fmt::print("Target [{}] does not seems to exists.", in_path.string());
      return -1;
    }

    std::filesystem::path out_path(program.present("-o")? program.get("-o"): "cache.png");
    std::fstream meta_info("meta.json");
    json meta;
    if(meta_info.peek() != std::ifstream::traits_type::eof()) {
      meta_info >> meta;
    }
    
    if(!std::filesystem::is_directory(in_path)) {
      //std::fstream reader(in_path, reader.in | reader.binary);
      
      //if(reader.is_open()){
	auto cache_path = out_path;
	cache_path.replace_extension(".zip");
	fmt::print("cache_path:{},out_path:{}\n", cache_path.string(), out_path.string());
	{
	  libzip::archive archive(cache_path, ZIP_CREATE);
	  archive.add(libzip::source_file(in_path.string()), in_path.string());
	}
	
	std::fstream cache_reader(cache_path, cache_reader.in|cache_reader.binary);
	if (cache_reader.is_open()){
	  std::vector<std::uint8_t> datas{std::istream_iterator<char>(cache_reader), std::istream_iterator<char>()};
	  fmt::print("bytes:{}\n", datas.size());

	  unsigned width = 10, height = 10;
	  if(datas.size() < 100) {
	    width = 10;
	  }
	  if(datas.size() < 500) {
	    width = 20;
	  } else if(datas.size() < 1000) {
	    width = 100;
	  } else {
	    width = std::sqrt(datas.size());
	  }
	  height = static_cast<float>(datas.size()) / static_cast<float>(width) + 0.5f;
	  
	  auto pos_to_index = [](unsigned width, sln::PixelIndex x, sln::PixelIndex y) { return (y.value() * width + x.value()); };
	  auto img_gen = [&datas, &pos_to_index, &width](sln::PixelIndex x, sln::PixelIndex y) {
			   sln::PixelRGB_8u pix;
			   auto offset = pos_to_index(width, x, y);
			   auto start_iter = std::next(datas.begin(),offset);
			   for(auto iter = start_iter; iter != datas.end() && iter != std::next(start_iter +3); iter++) {
			     pix[std::distance(start_iter, iter)] = *iter;
			   }
			   return pix;
			 };
	  //sln::PixelIndex a = 90,b = 90;
	  
	  auto img_rgb = sln::generate(img_gen, sln::to_pixel_length(width), sln::to_pixel_length(height));
	  sln::write_image(sln::to_dyn_image_view(img_rgb), sln::ImageFormat::PNG,
			   sln::VectorWriter(datas));
	  sln::write_data_contents(out_path.string(), datas);
	  meta[in_path.string()]["size"] = datas.size();
	  std::ofstream out("meta.json");
	  out << meta.dump(4)<< std::endl;
	} else {
	  fmt::print("Failed!!!!!!\n");
	}
	//}
    } else {
      fmt::print("{} is a directory.", in_path.string());
    }
    
    
    //} catch (const std::exception& ex) {
    //fmt::print("Failed: {}", ex.what());
    //throw;
    //return -1;
    //}
  return 0;
}
