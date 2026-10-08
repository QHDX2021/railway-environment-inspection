#pragma once

#include "firmware/core/state_machine.hpp"
#include "shared/protocol/messages.hpp"

namespace rail {

class IHostTransport {
 public:
  virtual ~IHostTransport() = default;
  virtual Status send(const Message&) = 0;
  virtual Status receive(Message&) = 0;
};

class HostProtocol {
 public:
  explicit HostProtocol(IHostTransport& transport) : transport_(transport) {}

  Status poll();
  AcquisitionState state() const { return state_machine_.state(); }

 private:
  Status handle(const Message& request, Message& response);

  IHostTransport& transport_;
  AcquisitionStateMachine state_machine_;
};

}  // namespace rail
