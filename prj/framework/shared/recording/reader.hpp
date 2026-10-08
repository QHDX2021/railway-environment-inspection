#pragma once
#include "shared/recording/codec.hpp"
#include <filesystem>
#include <functional>
namespace rail { enum class ReadMode{Strict,Recover}; struct ReadReport{bool ok{};size_t records{};size_t bad_offset{};bool complete{};}; class RecordReader{public:ReadReport read(const std::filesystem::path&,ReadMode,const std::function<void(const Record&)>&);}; }
