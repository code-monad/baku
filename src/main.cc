#include <fmt/core.h>

#include <algorithm>
#include <baku/baku.hh>
#include <chrono>
#include <digestpp.hpp>
#include <iostream>
#include <selene/base/io/FileReader.hpp>
#include <selene/base/io/FileWriter.hpp>
#include <selene/img/interop/ImageToDynImage.hpp>
#include <selene/img/typed/ImageTypeAliases.hpp>
#include <selene/img_io/IO.hpp>
#include <sstream>
#include <thread>

int main(int argc, char* argv[]) {
  baku::args args{};
  try {
    args.parse(argc, argv);
  } catch (const std::runtime_error& ex) {
    fmt::print("Error Parsing arguments: {}\n", ex.what());
    return -1;
  }

  baku::metainfo metainfo{};

  if (!args.meta().empty()) {
    if (std::filesystem::exists(args.meta())) {
      if (!std::filesystem::is_regular_file(args.meta())) {
        fmt::print(
            "[{}] does not seems to be a valid metainfo file source (Maybe a "
            "dir?)\n",
            args.meta().string());
        return -1;
      }
      metainfo = baku::metainfo::parse(args.meta());
    } else {
      fmt::print("Non metainfo exists,may create a new one.\n");
      try {
        std::ofstream touch(args.meta(),
                            touch.in | touch.out | touch.trunc | touch.binary);
        if (!touch.is_open()) {
          fmt::print("WARN: creation of [{}] may failed.\n",
                     args.meta().string());
        }
      } catch (const std::exception& ex) {
        fmt::print("Failed to create metainfo file {}: {}",
                   args.meta().string(), ex.what());
        return -1;
      }
    }
  }

  switch (args.mode()) {
    case baku::mode_t::DUMMY:
    case baku::mode_t::RUA:
      fmt::print("{}", args.help().str());
      break;
    case baku::mode_t::FEED: {
      fmt::print("Running in [FEED] mode\n");
      bool finished = false;
      auto wait_fn = [&finished] {
        while (!finished) {
          std::this_thread::sleep_for(std::chrono::seconds(1));
          if (!finished) fmt::print(".");
          std::cout.flush();
        }
        fmt::print("\n");
      };
      auto bundle_wait_worker = std::thread(wait_fn);
      auto bundled =
          baku::bundle_files(args.in(), args.cache(), args.verbose());
      finished = true;
      bundle_wait_worker.join();
      finished = false;
      baku::meta meta;
      meta.name = std::filesystem::is_directory(args.in())
                      ? bundled.filename()
                      : args.in().filename();
      std::ifstream reader(bundled, reader.in | reader.binary);
      if (reader.is_open()) {
        std::vector<std::uint8_t> buffer(
            (std::istreambuf_iterator<char>(reader)),
            std::istreambuf_iterator<char>());

        meta.sha = digestpp::sha512(256)
                       .absorb(buffer.begin(), buffer.end())
                       .hexdigest();
        meta.short_id = baku::to_shorten(meta.sha);
        meta.size = buffer.size();

        if (args.verbose()) {
          fmt::print("SHA:{}\n", meta.sha);
          fmt::print("SHORTEND:{}\n", meta.short_id);
        }

        if (args.fake().empty()) {
          meta.type = baku::BAKU_TYPE::png;
          fmt::print("Encoding!");
          std::cout.flush();
          auto encode_wait_worker = std::thread(wait_fn);
          auto encoded = baku::feed(buffer);
          finished = true;
          encode_wait_worker.join();
          meta.chunk_sha = digestpp::sha512(256)
                               .absorb(encoded.begin(), encoded.end())
                               .hexdigest();
          fmt::print("\nchunk sha:{}\n", meta.chunk_sha);
          finished = false;
          std::ofstream writer(args.out(),
                               writer.out | writer.binary | writer.trunc);
          if (writer.is_open()) {
            fmt::print("Writing to {}...", args.out().string());
            finished = false;
            auto write_wait_worker = std::thread(wait_fn);
            std::copy(encoded.begin(), encoded.end(),
                      std::ostreambuf_iterator<char>(writer));
            finished = true;
            write_wait_worker.join();
          } else {
            fmt::print("Failed to create or write to {}!", args.out().string());
            return -1;
          }
        } else {
          meta.type = baku::BAKU_TYPE::fake;
          const auto& encoded = buffer;
          if (args.verbose())
            fmt::print("Multipler using fake images {}\n",
                       args.fake().string());
          sln::DynImage fake_data =
              sln::read_image(sln::FileReader(args.fake().string()));
          if (!fake_data.is_valid()) {
            fmt::print("Image {} could not be decoded.\n",
                       args.fake().string());
            return -1;
          }

          // sln::write_image(img_data, sln::ImageFormat::PNG,
          // sln::FileWriter(args.fake().string())); // re-coded to png
          std::vector<std::uint8_t> fake_buffer;
          sln::write_image(fake_data, sln::ImageFormat::PNG,
                           sln::VectorWriter(fake_buffer));

          auto out_buffer = baku::fake_generation(fake_buffer, encoded);

          meta.chunk_sha = digestpp::sha512(256)
                               .absorb(out_buffer.begin(), out_buffer.end())
                               .hexdigest();

          std::ofstream writer(args.out(),
                               writer.out | writer.trunc | writer.binary);
          if (!writer.is_open()) {
            fmt::print("Failed to open {}!\n", args.out().string());
            return -1;
          }

          if (args.verbose())
            fmt::print("Writing to {}...\n", args.out().string());
          std::copy(out_buffer.cbegin(), out_buffer.cend(),
                    std::ostreambuf_iterator<char>(writer));
          writer.close();
          if (args.verbose())
            fmt::print("Wrote {} bytes!\n",
                       out_buffer.size() - fake_buffer.size());
        }
      }

      if (!metainfo.contains(meta)) {
        meta.timestamp =
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch())
                .count();
        metainfo.add(meta, true);
      } else {
        if (args.force()) {
          fmt::print(stderr, "force updating {}...\n", meta.short_id);
        } else {
          fmt::print(stderr, "{} already exists in {}, ignored adding to it.\n",
                     meta.short_id, args.meta().string());
        }
      }

      fmt::print("{}\n", meta.short_id);
    } break;

    case baku::mode_t::POUR:
      fmt::print("Running in [pour] mode\n");
      if (!std::filesystem::exists(args.in())) {
        fmt::print("{} does not seems to exist!", args.in().string());
        return -1;
      }
      std::ifstream reader(args.in(), reader.in | reader.binary);
      if (!reader.is_open()) {
        fmt::print("Failed to open {}!\n", args.in().string());
        return -1;
      }

      std::vector<std::uint8_t> raw_content(
          (std::istreambuf_iterator<char>(reader)), {});

      unsigned long long actual_size = 0;

      auto chunk_sha = digestpp::sha512(256)
                           .absorb(raw_content.begin(), raw_content.end())
                           .hexdigest();
      fmt::print("chunk sha: {}\n", chunk_sha);
      auto record = metainfo.contains_by_sha(chunk_sha);
      if (record != metainfo.end()) {
        if (args.verbose()) fmt::print("Found file in meta\n");
        std::vector<std::uint8_t> original_buffer =
            baku::pour(raw_content, record->second.type, record->second.size);
        std::copy(original_buffer.begin(), original_buffer.end(),
                  std::ostream_iterator<char>(std::cout, ""));
      } else {
        if (!args.fake().empty()) {
          if (!std::filesystem::is_regular_file(args.fake())) {
            fmt::print(
                "One must specify a file as multipler base in pour mode if no "
                "meta info provided.\n");
            return -1;
          }
        } else {
          fmt::print("regular pour");
        }
      }

      break;
  }

  metainfo.dump(args.meta());

  return 0;
}
