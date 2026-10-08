#pragma once
#include "linux/corrections/forwarder.hpp"
namespace rail {class SimulationCorrectionSource:public ICorrectionSource{Bytes data_;bool done_{};public:explicit SimulationCorrectionSource(Bytes b):data_(std::move(b)){}SourceResult next(CorrectionChunk&c)override{if(done_)return {Status::success(),true};done_=true;c={data_,0};return {Status::success(),false};}};class SimulationGnssSession:public IGnssSession{Bytes bytes_;public:Status write_correction(ByteView b)override{bytes_.assign(b.data,b.data+b.size);return Status::success();}const Bytes&bytes()const{return bytes_;}};}
