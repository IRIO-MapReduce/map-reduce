cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE ${PATH_TO_VCPKG}/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

project(mapreduce-client)

find_package(absl REQUIRED)
find_package(Protobuf REQUIRED CONFIG)
find_package(gRPC REQUIRED)
find_package(google_cloud_cpp_compute REQUIRED)
find_package(google_cloud_cpp_logging REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../common)  # Add common directory

set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../proto/)
set(PROTO_FILE ${PROTO_DIR}/mapreduce.proto)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(Protobuf_PROTOC_EXECUTABLE $<TARGET_FILE:protobuf::protoc>)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O3")

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

add_executable(test_client test_client.cc)
target_link_libraries(test_client PRIVATE grpc_mapreduce_proto gRPC::grpc++ gRPC::grpc++_reflection protobuf::libprotobuf google-cloud-cpp::compute google-cloud-cpp::logging)

target_include_directories(test_client PRIVATE ${GENERATED_DIR})
target_include_directories(test_client PRIVATE ../common)
target_sources(test_client PRIVATE ../common/utils.cc)
target_sources(test_client PRIVATE ../common/mapreduce.cc)
target_sources(test_client PRIVATE ../common/cloud-utils.cc)

add_executable(client client.cc)
target_link_libraries(client PRIVATE grpc_mapreduce_proto gRPC::grpc++ gRPC::grpc++_reflection protobuf::libprotobuf google-cloud-cpp::compute google-cloud-cpp::logging)

target_include_directories(client PRIVATE ${GENERATED_DIR})
target_include_directories(client PRIVATE ../common)
target_sources(client PRIVATE ../common/utils.cc)
target_sources(client PRIVATE ../common/mapreduce.cc)
target_sources(client PRIVATE ../common/cloud-utils.cc)

add_executable(testbed testbed.cc)
target_link_libraries(testbed PRIVATE grpc_mapreduce_proto gRPC::grpc++ gRPC::grpc++_reflection protobuf::libprotobuf google-cloud-cpp::compute google-cloud-cpp::logging)

target_include_directories(testbed PRIVATE ${GENERATED_DIR})
target_include_directories(testbed PRIVATE ../common)
target_sources(testbed PRIVATE ../common/utils.cc)
target_sources(testbed PRIVATE ../common/mapreduce.cc)
target_sources(testbed PRIVATE ../common/cloud-utils.cc)

add_executable(gen_synthetic_test gen_synthetic_test.cc)

target_include_directories(gen_synthetic_test PRIVATE ${GENERATED_DIR})
target_include_directories(gen_synthetic_test PRIVATE ../common)
target_sources(gen_synthetic_test PRIVATE ../common/test-utils.cc)
