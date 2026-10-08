#include "shared/configuration/parameters.hpp"
#include <cmath>
namespace rail {
void ParameterService::define(ParameterDescriptor d){values_[d.name]=d.default_value;defs_[d.name]=std::move(d);}
static bool same(const ParameterValue&a,const ParameterValue&b){return a==b;}
Status ParameterService::set(const std::string& n,const ParameterValue& v,uint64_t rev){auto it=defs_.find(n);if(it==defs_.end())return Status::failure(ErrorCode::InvalidArgument,"unknown parameter");if(rev!=revision_)return Status::failure(ErrorCode::Conflict,"revision conflict");if(it->second.read_only)return Status::failure(ErrorCode::InvalidState,"read only");if(it->second.min||it->second.max){if(!std::holds_alternative<int64_t>(v)&&!std::holds_alternative<double>(v))return Status::failure(ErrorCode::InvalidArgument,"numeric value required");double x=std::holds_alternative<int64_t>(v)?double(std::get<int64_t>(v)):std::get<double>(v);if((it->second.min&&x<*it->second.min)||(it->second.max&&x>*it->second.max))return Status::failure(ErrorCode::InvalidArgument,"out of range");}if(it->second.apply==ApplyMode::Restart){pending_[n]=v;return Status::success();}values_[n]=v;revision_++;return Status::success();}
Status ParameterService::read(const std::string& n,ParameterValue& v)const{auto it=values_.find(n);if(it==values_.end())return Status::failure(ErrorCode::InvalidArgument,"unknown parameter");v=it->second;return Status::success();}
std::map<std::string,ParameterValue> ParameterService::snapshot()const{std::map<std::string,ParameterValue> out;for(auto&[n,v]:values_)if(!defs_.at(n).secret)out[n]=v;return out;}
}
