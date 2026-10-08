cmake_minimum_required(VERSION 3.20)
project(rail_inspection_framework LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(CTest)

add_library(rail_protocol shared/protocol/frame.cpp)
target_include_directories(rail_protocol PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(rail_messages shared/protocol/messages.cpp)
target_include_directories(rail_messages PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(rail_time shared/types/time.cpp)
target_include_directories(rail_time PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(rail_control shared/configuration/parameters.cpp firmware/core/state_machine.cpp)
target_include_directories(rail_control PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(rail_control PUBLIC rail_protocol)

add_library(rail_firmware firmware/core/imu_capture.cpp firmware/core/host_protocol.cpp)
target_include_directories(rail_firmware PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(rail_firmware PUBLIC rail_control rail_messages)

add_library(rail_recording shared/recording/codec.cpp shared/recording/reader.cpp linux/storage/task_writer.cpp)
target_include_directories(rail_recording PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(rail_recording PUBLIC rail_protocol)

add_library(rail_acquisition linux/acquisition/pipeline.cpp adapters/simulation/sample_source.cpp)
target_include_directories(rail_acquisition PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(rail_acquisition PUBLIC rail_recording)

add_library(rail_devices
  linux/devices/byte_device.cpp
  linux/devices/pcb_device.cpp
  linux/devices/gnss_device.cpp
  linux/devices/lidar_device.cpp
  linux/devices/camera_device.cpp
  linux/acquisition/device_manager.cpp)
target_include_directories(rail_devices PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(rail_corrections
  linux/corrections/forwarder.cpp
  linux/corrections/ntrip_client.cpp
  linux/control/control_lock.cpp
  linux/control/service.cpp
  linux/control/http_server.cpp
  adapters/simulation/gnss_session.cpp)
target_include_directories(rail_corrections PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(rail_corrections PUBLIC rail_control)

add_library(rail_processing
  desktop/playback/playback.cpp
  desktop/processing/interfaces.cpp
  desktop/processing/coordinate_frames.cpp
  desktop/processing/calibration.cpp
  desktop/processing/trajectory.cpp
  desktop/processing/point_cloud.cpp
  desktop/processing/dataset_manifest.cpp
  desktop/processing/ai_analyzer.cpp
  desktop/processing/change_analyzer.cpp)
target_include_directories(rail_processing PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(rail_processing PUBLIC rail_recording rail_time)

add_library(rail_desktop
  desktop/application/job.cpp
  desktop/control/device_client.cpp
  desktop/download/task_downloader.cpp)
target_include_directories(rail_desktop PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(rail_desktop PUBLIC rail_processing rail_corrections)

add_executable(capture_demo apps/capture_demo/main.cpp)
target_link_libraries(capture_demo PRIVATE rail_acquisition rail_recording)
add_executable(replay_cli apps/replay_cli/main.cpp)
target_link_libraries(replay_cli PRIVATE rail_processing rail_recording)

if(BUILD_TESTING)
  function(add_rail_test name source libs)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE ${libs})
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME ${name} COMMAND ${name})
  endfunction()
  add_rail_test(protocol_test tests/protocol_test.cpp rail_protocol)
  add_rail_test(time_model_test tests/time_model_test.cpp rail_time)
  add_rail_test(messages_test tests/messages_test.cpp rail_messages)
  add_rail_test(task_lifecycle_test tests/task_lifecycle_test.cpp rail_processing)
  add_rail_test(firmware_core_test tests/firmware_core_test.cpp rail_firmware)
  add_rail_test(device_adapter_test tests/device_adapter_test.cpp rail_devices)
  add_rail_test(control_api_test tests/control_api_test.cpp rail_corrections)
  add_rail_test(desktop_control_test tests/desktop_control_test.cpp rail_desktop)
  add_rail_test(job_test tests/job_test.cpp rail_desktop)
  add_rail_test(calibration_test tests/calibration_test.cpp rail_processing)
  add_rail_test(trajectory_test tests/trajectory_test.cpp rail_processing)
  add_rail_test(point_cloud_test tests/point_cloud_test.cpp rail_processing)
  add_rail_test(ai_analyzer_test tests/ai_analyzer_test.cpp rail_processing)
  add_rail_test(change_analyzer_test tests/change_analyzer_test.cpp rail_processing)
  add_rail_test(control_test tests/control_test.cpp rail_control)
  add_rail_test(recording_test tests/recording_test.cpp rail_recording)
  add_rail_test(acquisition_test tests/acquisition_test.cpp rail_acquisition)
  add_rail_test(corrections_test tests/corrections_test.cpp rail_corrections)
  add_rail_test(playback_test tests/playback_test.cpp rail_processing)
  add_rail_test(processing_test tests/processing_test.cpp rail_processing)
endif()
