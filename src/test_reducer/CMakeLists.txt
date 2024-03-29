cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE ${PATH_TO_VCPKG}/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

project(test_reducer)

find_package(absl REQUIRED)
find_package(Protobuf REQUIRED CONFIG)
find_package(gRPC REQUIRED)
find_package(google_cloud_cpp_compute REQUIRED)
find_package(google_cloud_cpp_logging REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

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

add_executable(simple_reducer simple_reducer.cc)
target_link_libraries(simple_reducer PRIVATE grpc_mapreduce_proto gRPC::grpc++ gRPC::grpc++_reflection protobuf::libprotobuf google-cloud-cpp::compute google-cloud-cpp::logging)

target_include_directories(simple_reducer PRIVATE ${GENERATED_DIR})
target_include_directories(simple_reducer PRIVATE ../common)
target_sources(simple_reducer PRIVATE ../common/utils.cc)
target_sources(simple_reducer PRIVATE ../common/mapreduce.cc)
target_sources(simple_reducer PRIVATE ../common/reducer.cc)
target_sources(simple_reducer PRIVATE ../common/cloud-utils.cc)

add_executable(word_count_reducer word_count_reducer.cc)
target_link_libraries(word_count_reducer PRIVATE grpc_mapreduce_proto gRPC::grpc++ gRPC::grpc++_reflection protobuf::libprotobuf google-cloud-cpp::compute google-cloud-cpp::logging)

target_include_directories(word_count_reducer PRIVATE ${GENERATED_DIR})
target_include_directories(word_count_reducer PRIVATE ../common)
target_sources(word_count_reducer PRIVATE ../common/utils.cc)
target_sources(word_count_reducer PRIVATE ../common/mapreduce.cc)
target_sources(word_count_reducer PRIVATE ../common/reducer.cc)
target_sources(word_count_reducer PRIVATE ../common/cloud-utils.cc)

add_executable(long_reducer long_reducer.cc)
target_link_libraries(long_reducer PRIVATE grpc_mapreduce_proto gRPC::grpc++ gRPC::grpc++_reflection protobuf::libprotobuf google-cloud-cpp::compute google-cloud-cpp::logging)

target_include_directories(long_reducer PRIVATE ${GENERATED_DIR})
target_include_directories(long_reducer PRIVATE ../common)
target_sources(long_reducer PRIVATE ../common/utils.cc)
target_sources(long_reducer PRIVATE ../common/mapreduce.cc)
target_sources(long_reducer PRIVATE ../common/reducer.cc)
target_sources(long_reducer PRIVATE ../common/cloud-utils.cc)

add_executable(reducer_randomly_crashing reducer_randomly_crashing.cc)
target_link_libraries(reducer_randomly_crashing PRIVATE grpc_mapreduce_proto gRPC::grpc++ gRPC::grpc++_reflection protobuf::libprotobuf google-cloud-cpp::compute google-cloud-cpp::logging)

target_include_directories(reducer_randomly_crashing PRIVATE ${GENERATED_DIR})
target_include_directories(reducer_randomly_crashing PRIVATE ../common)
target_sources(reducer_randomly_crashing PRIVATE ../common/utils.cc)
target_sources(reducer_randomly_crashing PRIVATE ../common/mapreduce.cc)
target_sources(reducer_randomly_crashing PRIVATE ../common/reducer.cc)
target_sources(reducer_randomly_crashing PRIVATE ../common/cloud-utils.cc)
target_sources(reducer_randomly_crashing PRIVATE ../common/test-utils.cc)

add_executable(reducer_inter_write_crashing reducer_inter_write_crashing.cc)
target_link_libraries(reducer_inter_write_crashing PRIVATE grpc_mapreduce_proto gRPC::grpc++ gRPC::grpc++_reflection protobuf::libprotobuf google-cloud-cpp::compute google-cloud-cpp::logging)

target_include_directories(reducer_inter_write_crashing PRIVATE ${GENERATED_DIR})
target_include_directories(reducer_inter_write_crashing PRIVATE ../common)
target_sources(reducer_inter_write_crashing PRIVATE ../common/utils.cc)
target_sources(reducer_inter_write_crashing PRIVATE ../common/mapreduce.cc)
target_sources(reducer_inter_write_crashing PRIVATE ../common/reducer.cc)
target_sources(reducer_inter_write_crashing PRIVATE ../common/cloud-utils.cc)
target_sources(reducer_inter_write_crashing PRIVATE ../common/test-utils.cc)

add_executable(reducer_inter_write_crashing_huge reducer_inter_write_crashing_huge.cc)
target_link_libraries(reducer_inter_write_crashing_huge PRIVATE grpc_mapreduce_proto gRPC::grpc++ gRPC::grpc++_reflection protobuf::libprotobuf google-cloud-cpp::compute google-cloud-cpp::logging)

target_include_directories(reducer_inter_write_crashing_huge PRIVATE ${GENERATED_DIR})
target_include_directories(reducer_inter_write_crashing_huge PRIVATE ../common)
target_sources(reducer_inter_write_crashing_huge PRIVATE ../common/utils.cc)
target_sources(reducer_inter_write_crashing_huge PRIVATE ../common/mapreduce.cc)
target_sources(reducer_inter_write_crashing_huge PRIVATE ../common/reducer.cc)
target_sources(reducer_inter_write_crashing_huge PRIVATE ../common/cloud-utils.cc)
target_sources(reducer_inter_write_crashing_huge PRIVATE ../common/test-utils.cc)
