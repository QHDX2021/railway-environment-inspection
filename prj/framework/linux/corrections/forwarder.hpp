#pragma once
#include "linux/acquisition/pipeline.hpp"
namespace rail {struct CorrectionChunk{Bytes data;uint64_t received_monotonic_ns{};};class ICorrectionSource{public:virtual~ICorrectionSource()=default;virtual SourceResult next(CorrectionChunk&)=0;};class IGnssSession{public:virtual~IGnssSession()=default;virtual Status write_correction(ByteView)=0;};class CorrectionForwarder{ICorrectionSource&src_;IGnssSession&dst_;uint64_t max_age_;public:CorrectionForwarder(ICorrectionSource&s,IGnssSession&d,uint64_t age):src_(s),dst_(d),max_age_(age){}Status step(uint64_t now);};}
