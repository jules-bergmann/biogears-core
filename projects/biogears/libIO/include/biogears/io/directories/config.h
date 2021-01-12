#pragma once

/**************************************************************************************
Copyright 2021 Applied Research Associates, Inc.
Licensed under the Apache License, Version 2.0 (the "License"); you may not use
this file except in compliance with the License. You may obtain a copy of the License
at:
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software distributed under
the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.
**************************************************************************************/

#include <biogears/filesystem/path.h>
#include <biogears/io-exports.h>

//!
//!  Functions for finding resource files that were part of the current biogears release
//!  These functions are encoded as part of libbiogears_io
//!
//!  Functions are exposed as either biogears::filesystem::path or char const *
//!  TODO: Migrate Biogears C++17 and replace biogears::filesystem with std::filesystem

namespace biogears {
namespace io {

  //C string variants
  BIOGEARS_IO_API bool generate_config_directory(const char* file);
  BIOGEARS_IO_API bool does_embeded_config_file_exists(const char* file);

  BIOGEARS_IO_API char const* find_config_file(const char* file);
  BIOGEARS_IO_API char const* get_config_file_hash(const char* file);

  BIOGEARS_IO_API char const* const* list_config_files();
  BIOGEARS_IO_API size_t config_file_count();

  BIOGEARS_IO_API char const* get_embeded_config_file(const char* file);

  //biogears::filesystem::path overloads
  BIOGEARS_IO_API bool generate_config_directory(biogears::filesystem::path file);
  BIOGEARS_IO_API bool does_embeded_config_file_exists(biogears::filesystem::path file);

  BIOGEARS_IO_API char const* get_config_file_hash(biogears::filesystem::path file);
  BIOGEARS_IO_API char const* get_embeded_config_file(biogears::filesystem::path file);
}
}