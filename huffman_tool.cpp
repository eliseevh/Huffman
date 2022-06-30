#include "decoder.h"
#include "encoder.h"
#include <cxxopts.hpp>
#include <fstream>
#include <iostream>
#include <string>

namespace {
constexpr int MAX_PERCENTS = 100;

void help(cxxopts::Options const& options, cxxopts::ParseResult const& result) {
  std::cout << options.help() << std::endl;
  std::cout << "In decompression mode, if input file was not compressed by "
               "that tool than an error might occur"
            << std::endl;
  if (result.count("output") != 1 || result.count("input") != 1) {
    std::cerr << "Both output and input files must be passed as arguments"
              << std::endl;
  }
  if (result.count("compress") + result.count("decompress") != 1) {
    std::cerr << "Exactly one of --compress and --decompress options must "
                 "be passed"
              << std::endl;
  }
  if (!result.unmatched().empty()) {
    std::cerr << "Unknown options:";
    for (std::string const& option : result.unmatched()) {
      std::cerr << " " << option;
    }
    std::cerr << std::endl;
  }
}

void error(std::string const& type, std::string const& what) {
  std::cerr << type << " error: " << what << std::endl;
  exit(1);
}

void ensure_open(std::ifstream const& stream) {
  if (!stream.is_open()) {
    error("I/O", "cannot open input file");
  }
}

void ensure_open(std::ofstream const& stream) {
  if (!stream.is_open()) {
    error("I/O", "cannot open output file");
  }
}

constexpr std::array<char const*, 4> SIZES = {" bytes", " KB", " MB", " GB"};
constexpr double size_factor = 1024;

std::string show_size(size_t size_in_bytes) {
  std::string result;
  if (static_cast<double>(size_in_bytes) < size_factor) {
    result.append(std::to_string(size_in_bytes)).append(SIZES[0]);
    return result;
  }
  double size = static_cast<double>(size_in_bytes) / size_factor;
  for (size_t i = 1; i < SIZES.size(); ++i) {
    if (size < size_factor) {
      result.append(std::to_string(size)).append(SIZES[i]);
      break;
    } else {
      size /= size_factor;
    }
  }
  if (result.empty()) {
    result.append(std::to_string(size)).append(SIZES.back());
  }
  return result;
}
void show_files_info(std::string const& input_filename, size_t input_size,
                     std::string const& output_filename, size_t output_size) {
  std::cout << "Input file: " << input_filename
            << ", size: " << show_size(input_size)
            << "\nOutput file: " << output_filename
            << ", size: " << show_size(output_size) << std::endl;
}
void show_compression_rate(
    size_t compressed_size, // NOLINT(bugprone-easily-swappable-parameters)
    size_t decompressed_size, bool compression_mode) {
  if (decompressed_size != 0) {
    double comp_rate = static_cast<double>(compressed_size) /
                       static_cast<double>(decompressed_size);
    std::cout << "Compressed file " << (compression_mode ? "is " : "was ");
    if (comp_rate < 1) {
      std::cout << (MAX_PERCENTS - static_cast<int>(MAX_PERCENTS * comp_rate))
                << "% less";
    } else {
      std::cout << static_cast<int>(MAX_PERCENTS * comp_rate) - MAX_PERCENTS
                << "% bigger";
    }
    std::cout << std::endl;
  }
}
} // namespace
int main(int argc, char** argv) {
  cxxopts::Options options(
      "huffman-tool",
      "Tool for compressing and decompressing files using Huffman algorithm");
  options.add_options()
      ("i, info", "Show information about files")
      ("d,decompress", "Decompressing mode")
      ("c,compress", "Compressing mode")
      ("input", "Input file name",
               cxxopts::value<std::string>(), "filename")
      ("output", "Output file name",
               cxxopts::value<std::string>(), "filename")
      ("h,help", "Print help message");

  try {
    auto result = options.parse(argc, argv);

    if (result.count("output") != 1 || result.count("input") != 1 ||
        result.count("compress") + result.count("decompress") != 1 ||
        !result.unmatched().empty()) {
      help(options, result);
      return 1;
    } else if (result.count("help") != 0) {
      help(options, result);
      return 0;
    }

    bool compress = result.count("compress") == 1;
    bool show_info = result.count("info") >= 1;

    std::string input_filename = result["input"].as<std::string>();
    std::string output_filename = result["output"].as<std::string>();

    if (compress) {
      std::ifstream count_stream(input_filename, std::ios::binary);
      ensure_open(count_stream);

      huffman::encoder encoder_;
      encoder_.add_chars(count_stream);

      count_stream.close();

      std::ifstream input_stream(input_filename, std::ios::binary);
      ensure_open(input_stream);

      std::ofstream output_stream(output_filename, std::ios::binary);
      ensure_open(output_stream);

      encoder_.encode(input_stream, output_stream);
      if (show_info) {
        size_t input_size = encoder_.get_input_size();
        size_t output_size = encoder_.get_output_size();
        show_files_info(input_filename, input_size, output_filename,
                        output_size);
        show_compression_rate(output_size, input_size, true);
      }
    } else {
      std::ifstream input_stream(input_filename, std::ios::binary);
      ensure_open(input_stream);

      std::ofstream output_stream(output_filename, std::ios::binary);
      ensure_open(output_stream);

      huffman::decoder decoder_;
      try {
        auto [input_size, output_size] =
            decoder_.decode(input_stream, output_stream);
        if (show_info) {
          show_files_info(input_filename, input_size, output_filename,
                          output_size);
          show_compression_rate(input_size, output_size, false);
        }
      } catch (std::runtime_error const& e) {
        error("Decoding", e.what());
      }
    }
  } catch (cxxopts::OptionParseException const& e) {
    error("Parsing arguments", e.what());
  } catch (std::fstream::failure const& e) {
    error("I/O", e.what());
  }
}
