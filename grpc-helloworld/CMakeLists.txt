cmake_minimum_required(VERSION 3.15)

project(grpc_hello_world)

find_package(absl CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/helloworld.proto)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

file(MAKE_DIRECTORY ${GENERATED_DIR})

add_custom_command(
    OUTPUT ${GENERATED_DIR}/helloworld.pb.cc ${GENERATED_DIR}/helloworld.pb.h
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS --cpp_out=${GENERATED_DIR} --proto_path=${CMAKE_CURRENT_SOURCE_DIR} ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating C++ files from ${PROTO_FILE}"
)

add_custom_command(
    OUTPUT ${GENERATED_DIR}/helloworld.grpc.pb.cc ${GENERATED_DIR}/helloworld.grpc.pb.h
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS --grpc_out=${GENERATED_DIR} --proto_path=${CMAKE_CURRENT_SOURCE_DIR} --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating gRPC files from ${PROTO_FILE}"
)

add_library(grpc_hello_world_proto
    ${GENERATED_DIR}/helloworld.pb.cc
    ${GENERATED_DIR}/helloworld.grpc.pb.cc
)

add_executable(helloworld_server src/helloworld_server.cc)
target_link_libraries(helloworld_server PRIVATE grpc_hello_world_proto gRPC::grpc++ gRPC::grpc++_reflection protobuf::libprotobuf)

add_executable(helloworld_client src/helloworld_client.cc)
target_link_libraries(helloworld_client PRIVATE grpc_hello_world_proto gRPC::grpc++ gRPC::grpc++_reflection protobuf::libprotobuf)

target_include_directories(helloworld_server PRIVATE ${GENERATED_DIR})
target_include_directories(helloworld_client PRIVATE ${GENERATED_DIR})