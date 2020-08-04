#include <baku/baku.hh>
#include <fmt/core.h>

#include <cstdint>


#include <selene/img_io/IO.hpp>
#include <selene/base/io/FileReader.hpp>
#include <selene/base/io/FileWriter.hpp>
#include <selene/img/typed/ImageTypeAliases.hpp>
#include <selene/img/interop/ImageToDynImage.hpp>

#include <lodepng.h>
#include <pngwriter.h>

int main(int argc, char *argv[]) {
    baku::args args{};
    try{
      args.parse(argc, argv);
    } catch(const std::runtime_error& ex){
      fmt::print("Error Parsing arguments: {}\n", ex.what());
      return -1;
    }

    switch(args.mode()){
    case baku::mode_t::FEED:
      {
      fmt::print("Running in [feed] mode\n");
      auto bundled = baku::bundle_files(args.in(), args.cache());

      std::ifstream reader(bundled, reader.in | reader.binary);
      if(reader.is_open()){
	std::vector<std::uint8_t> buffer((std::istreambuf_iterator<char>(reader)), std::istreambuf_iterator<char>());
	auto encoded = baku::feed(buffer);
	if(args.fake().empty()){
	  std::ofstream writer(args.out(), writer.out| writer.binary| writer.trunc);
	  if(writer.is_open()) {
	    fmt::print("Writing to {}...", args.out().string());
	    std::copy(encoded.begin(), encoded.end(), std::ostreambuf_iterator<char>(writer));
	  }
	} else {
	  fmt::print("Multipler using fake images {}", args.fake().string());
	  sln::DynImage img_data = sln::read_image(sln::FileReader(args.fake().string()));
	  if(!img_data.is_valid()){
	    fmt::print("Image {} could not be decoded.", args.fake().string());
	    return -1;
	  }

	  sln::write_image(img_data, sln::ImageFormat::PNG, sln::FileWriter(args.fake().string())); // re-coded to png

	  
	  std::vector<unsigned char> buffer;
	  std::vector<unsigned char> image;
	  lodepng::load_file(buffer, args.fake().string());
	  lodepng::State state;
	  unsigned w,h;
	  state.encoder.text_compression = 1;
	  state.decoder.remember_unknown_chunks = 1;
	  if(lodepng::decode(image, w, h, state, buffer)) {
	    fmt::print("Image {} could not be decoded.", args.fake().string());
	    return -1;
	  }

	  

	  fmt::print("writing to {}...\n", args.out().string());

	  lodepng_add_text(&(state.info_png), "chunk_datas", std::string(encoded.begin(), encoded.end()).c_str());

	  if(lodepng::encode(buffer, image, w, h, state)) {
	    fmt::print("Failed to encoded to {}.", args.out().string());
	  }

	  lodepng::save_file(buffer, args.out().string());
	}
      }
      }
      break;
    case baku::mode_t::POUR:
      fmt::print("Running in [pour] mode\n");
      //if(!args.fake().empty()){
	std::vector<unsigned char> buffer;
	std::vector<unsigned char> image;
	lodepng::load_file(buffer, args.in().string());
	lodepng::State state;
	state.decoder.remember_unknown_chunks = 1;
	state.decoder.read_text_chunks = 1;
	unsigned w,h;
	if(lodepng::decode(image, w, h, state, buffer)) {
	  fmt::print("Image {} could not be decoded.", args.in().string());
	  return -1;
	}
	fmt::print("texts:{}, {}", state.info_png.text_num, state.info_png.itext_num);
	fmt::print("size:{}, w:{}, h:{}", buffer.size(), w, h);
	//}
      break;
    }
    
  return 0;
}
