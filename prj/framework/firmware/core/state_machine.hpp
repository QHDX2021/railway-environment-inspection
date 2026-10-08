#pragma once
#include "shared/types/types.hpp"
namespace rail { enum class AcquisitionState{Idle,Starting,Recording,Stopping,Fault}; enum class AcquisitionEvent{Start,Started,Stop,Stopped,Fault,Reset};
class AcquisitionStateMachine{AcquisitionState state_{AcquisitionState::Idle};public:ErrorCode dispatch(AcquisitionEvent);AcquisitionState state()const{return state_;}};
enum class PowerEvent{ExternalLost,DataFlushed,OsShutdownComplete,Timeout,Reset};enum class PowerAction{None,StopAcquisition,NotifyHost,RequestShutdown,CutPower,Fault};
class PowerStateMachine{bool shutdown_requested_{};public:PowerAction dispatch(PowerEvent);}; }
