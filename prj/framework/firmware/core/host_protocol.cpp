#include "firmware/core/host_protocol.hpp"

namespace rail {

Status HostProtocol::handle(const Message& request, Message& response) {
  response = {MessageType::Status, request.request_id, {}};
  switch (request.type) {
    case MessageType::Capabilities:
      response.type = MessageType::Capabilities;
      return Status::success();
    case MessageType::Status:
      response.payload = {static_cast<uint8_t>(state_machine_.state())};
      return Status::success();
    case MessageType::TaskStart: {
      auto code = state_machine_.dispatch(AcquisitionEvent::Start);
      if (code != ErrorCode::Ok) return Status::failure(code, "cannot start acquisition");
      code = state_machine_.dispatch(AcquisitionEvent::Started);
      if (code != ErrorCode::Ok) return Status::failure(code, "cannot enter recording");
      response.payload = {static_cast<uint8_t>(state_machine_.state())};
      return Status::success();
    }
    case MessageType::TaskStop: {
      auto code = state_machine_.dispatch(AcquisitionEvent::Stop);
      if (code != ErrorCode::Ok) return Status::failure(code, "cannot stop acquisition");
      code = state_machine_.dispatch(AcquisitionEvent::Stopped);
      if (code != ErrorCode::Ok) return Status::failure(code, "cannot leave recording");
      response.payload = {static_cast<uint8_t>(state_machine_.state())};
      return Status::success();
    }
    default:
      response.type = MessageType::Error;
      return Status::failure(ErrorCode::NotSupported, "message not supported by firmware core");
  }
}

Status HostProtocol::poll() {
  Message request;
  auto status = transport_.receive(request);
  if (!status.ok()) return status;

  Message response;
  status = handle(request, response);
  if (!status.ok()) {
    response = {MessageType::Error, request.request_id,
                {static_cast<uint8_t>(status.code)}};
  }
  return transport_.send(response);
}

}  // namespace rail
