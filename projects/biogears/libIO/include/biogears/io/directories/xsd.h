#pragma once

#include <biogears/io-exports.h>

#include <cstddef>
//!
//!  Functions for finding resource files that were part of the current biogears release
//!  These functions are encoded as part of libbiogears_io
//!
//!  Functions are exposed as either biogears::filesystem::path or char const *
//!  TODO: Migrate Biogears C++17 and replace biogears::filesystem with std::filesystem

namespace biogears {
namespace io {

  BIOGEARS_IO_API bool generate_xsd_directory(const char* file);
  BIOGEARS_IO_API bool does_embedded_xsd_file_exist(const char* file);

  BIOGEARS_IO_API char const* get_xsd_file_sha1(const char* file);

  BIOGEARS_IO_API char const** list_xsd_files();
  BIOGEARS_IO_API size_t xsd_file_count();

  BIOGEARS_IO_API char const* get_embedded_xsd_file(const char* file, size_t& content_size);
  BIOGEARS_IO_API size_t get_embedded_xsd_file_size(const char* file);

  BIOGEARS_IO_API char const* get_default_xsd_directory();
  BIOGEARS_IO_API size_t get_embedded_xsd_file_size(const char* file);
}
}