cmake_minimum_required(VERSION 3.15)

project(mapreduce-client)

find_package(absl REQUIRED)
find_package(Protobuf REQUIRED CONFIG)
find_package(gRPC REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../proto/)
set(PROTO_FILE ${PROTO_DIR}/mapreduce.proto)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(Protobuf_PROTOC_EXECUTABLE /usr/local/bin/protoc)

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
    COMMAND ${Protobuf_PROTOC_EXECUTABLE} --grpc_out=${GENERATED_DIR} --proto_path=${PROTO_DIR} --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating gRPC files from ${PROTO_FILE}"
)

add_library(grpc_mapreduce_proto
    ${GENERATED_DIR}/mapreduce.pb.cc
    ${GENERATED_DIR}/mapreduce.grpc.pb.cc
)

add_executable(test_client test_client.cc)
target_link_libraries(test_client PRIVATE grpc_mapreduce_proto gRPC::grpc++ gRPC::grpc++_reflection protobuf::libprotobuf)

target_include_directories(test_client PRIVATE ${GENERATED_DIR})
target_include_directories(test_client PRIVATE ../common)
target_sources(test_client PRIVATE ../common/utils.cc)
target_sources(test_client PRIVATE ../common/mapreduce.cc)