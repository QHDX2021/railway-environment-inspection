#pragma once
#include "linux/acquisition/pipeline.hpp"
namespace rail { class SimulationSource:public ISampleSource{size_t limit_,current_{};bool stopped_{};public:explicit SimulationSource(size_t n):limit_(n){}SourceResult next(Record&)override;void stop()override{stopped_=true;}}; }
