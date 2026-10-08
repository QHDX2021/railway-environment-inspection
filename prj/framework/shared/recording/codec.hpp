#pragma once
#include "shared/recording/record.hpp"
#include <ostream>
namespace rail { Status write_record(std::ostream&,const Record&); Status read_record(const uint8_t*,size_t,Record&,size_t& consumed); }
