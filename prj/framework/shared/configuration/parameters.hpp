#pragma once
#include "shared/types/types.hpp"
#include <map>
#include <variant>
#include <string>
namespace rail {
enum class ApplyMode { Immediate, IdleOnly, Restart };
using ParameterValue=std::variant<bool,int64_t,double,std::string>;
struct ParameterDescriptor { std::string name; ParameterValue default_value; std::optional<double> min,max; std::string unit; bool read_only{}; ApplyMode apply{ApplyMode::Immediate}; bool secret{}; };
class ParameterService { public: void define(ParameterDescriptor d); Status set(const std::string&,const ParameterValue&,uint64_t expected_revision); Status read(const std::string&,ParameterValue&) const; uint64_t revision()const{return revision_;} std::map<std::string,ParameterValue> snapshot()const; private: std::map<std::string,ParameterDescriptor> defs_;std::map<std::string,ParameterValue> values_;std::map<std::string,ParameterValue> pending_;uint64_t revision_{}; };
}
