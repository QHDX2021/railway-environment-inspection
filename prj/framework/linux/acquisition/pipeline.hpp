#pragma once
#include "linux/storage/task_writer.hpp"
#include <atomic>
namespace rail { struct SourceResult{Status status;bool eof{};bool ok()const{return status.ok();}}; class ISampleSource{public:virtual~ISampleSource()=default;virtual SourceResult next(Record&)=0;virtual void stop()=0;}; struct PipelineOptions{size_t queue_capacity{64};std::atomic_bool* cancel{};};struct RunReport{bool ok{};size_t produced{};size_t saved{};size_t lost{};Status status;};class Pipeline{public:RunReport run(ISampleSource&,IRecordingWriter&,const PipelineOptions&);}; }
