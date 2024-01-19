cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE ${PATH_TO_VCPKG}/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

project(mapreduce-worker)

find_package(absl REQUIRED)
find_package(Protobuf REQUIRED CONFIG)
find_package(gRPC REQUIRED)
find_package(google_cloud_cpp_common REQUIRED)
find_package(google_cloud_cpp_compute REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../common)  # Add common directory

set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../proto/)
set(PROTO_FILE ${PROTO_DIR}/mapreduce.proto)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(Protobuf_PROTOC_EXECUTABLE $<TARGET_FILE:protobuf::protoc>)
set(CMAKE_CXX_STANDARD 20)

message(PROTO_DIR="${PROTO_DIR}")

file(MAKE_DIRECTORY ${GENERATED_DIR})

add_custom_command(
    OUTPUT ${GENERATED_DIR}/mapreduce.pb.cc ${GENERATED_DIR}/mapreduce.pb.h
    COMMAND ${Protobuf_PROTOC_EXECUTABLE} --cpp_out=${GENERATED_DIR} --proto_path=${PROTO_DIR} ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating C++ files from ${PROTO_FILE}"
)

add_custom_command(
    OUTPUT ${GENERATED_DIR}/mapreduce.grpc.pb.cc ${GENERATED_DIR}/mapreduce.grpc.pb.h
    COMMAND ${Protobuf_PROTOC_EXECUTABLE} --grpc_out=${GENERATED_DIR} --proto_path=${PROTO_DIR} --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin> ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating gRPC files from ${PROTO_FILE}"
)

add_library(grpc_mapreduce_proto
    ${GENERATED_DIR}/mapreduce.pb.cc
    ${GENERATED_DIR}/mapreduce.grpc.pb.cc
)

add_executable(main main.cc)
target_link_libraries(main PRIVATE grpc_mapreduce_proto gRPC::grpc++ gRPC::grpc++_reflection protobuf::libprotobuf google-cloud-cpp::compute)

target_include_directories(main PRIVATE ${GENERATED_DIR})
target_sources(main PRIVATE health-check-service.cc)
target_sources(main PRIVATE ../common/utils.cc)
target_sources(main PRIVATE ../common/mapreduce.cc)
target_sources(main PRIVATE ../common/cloud-utils.cc)
target_sources(main PRIVATE ../common/job-manager.cc)
target_sources(main PRIVATE ../common/data-structures.cc)
target_sources(main PRIVATE ../common/load-balancer.cc)
