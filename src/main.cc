#include <baku/baku.hh>

#include <fmt/core.h>

#include <selene/img_io/IO.hpp>
#include <selene/base/io/FileReader.hpp>
#include <selene/base/io/FileWriter.hpp>
#include <selene/img/typed/ImageTypeAliases.hpp>
#include <selene/img/interop/ImageToDynImage.hpp>

#include <algorithm>

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
			if(args.fake().empty()){
				auto encoded = baku::feed(buffer);
				std::ofstream writer(args.out(), writer.out| writer.binary| writer.trunc);
				if(writer.is_open()) {
					fmt::print("Writing to {}...", args.out().string());
					std::copy(encoded.begin(), encoded.end(), std::ostreambuf_iterator<char>(writer));
				}
			} else {
				const auto& encoded = buffer;
				fmt::print("Multipler using fake images {}\n", args.fake().string());
				sln::DynImage img_data = sln::read_image(sln::FileReader(args.fake().string()));
				if(!img_data.is_valid()){
					fmt::print("Image {} could not be decoded.\n", args.fake().string());
					return -1;
				}

				//sln::write_image(img_data, sln::ImageFormat::PNG, sln::FileWriter(args.fake().string())); // re-coded to png

				std::ifstream fake_stream(args.fake(), fake_stream.in | fake_stream.binary);

				if(!fake_stream.is_open()) {
					fmt::print("Failed to open {}!\n", args.fake().string());
					return -1;
				}

				std::ofstream writer(args.out(), writer.out | writer.trunc | writer.binary);
				if(!writer.is_open()) {
					fmt::print("Failed to open {}!\n", args.out().string());
					return -1;
				}
	  
				fmt::print("Writing to {}...\n", args.out().string());
			    writer << fake_stream.rdbuf();
				fake_stream.close();
				std::copy(encoded.crbegin(), encoded.crend(), std::ostreambuf_iterator<char>(writer));
				writer.close();
				fmt::print("Wrote {} bytes!", encoded.size());
			}
		}
	}
	break;
    case baku::mode_t::POUR:
		fmt::print("Running in [pour] mode\n");
		std::ifstream reader(args.in(), reader.in | reader.binary);
		if(!reader.is_open()) {
			fmt::print("Failed to open {}!\n", args.in().string());
			return -1;
		}
		
		std::vector<char> raw_content((std::istreambuf_iterator<char>(reader)),
                               std::istreambuf_iterator<char>());

		if(!args.fake().empty()) {
			if(!std::filesystem::is_regular_file(args.fake())) {
				fmt::print("One must specify a file as multipler base in pour mode if no meta info provided.\n");
				return -1;
			}

			unsigned long long actual_size = std::filesystem::file_size(args.in()) - std::filesystem::file_size(args.fake());
			fmt::print("Raw Size:{}\n", raw_content.size());
		
		
			std::ofstream writer(args.out(), writer.out | writer.binary | writer.trunc);

			if(!writer.is_open()) {
				fmt::print("Failed to open {}!\n", args.out().string());
				return -1;
			}
		
			std::copy_n(raw_content.crbegin(), actual_size, std::ostreambuf_iterator<char>(writer));
		
			writer.close();
			fmt::print("Wrote {} bytes!", actual_size);
		}
		
		break;
    }
    
	return 0;
}
