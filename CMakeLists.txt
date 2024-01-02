cmake_minimum_required(VERSION 3.15)
project(MapReduce)

include(FetchContent)
set(ABSL_ENABLE_INSTALL ON)
FetchContent_Declare(
    gRPC
    GIT_REPOSITORY https://github.com/grpc/grpc
    GIT_TAG        v1.60.0
)
set(FETCHCONTENT_QUIET OFF)
FetchContent_MakeAvailable(gRPC)

set(_PROTOBUF_LIBPROTOBUF libprotobuf)
set(_REFLECTION grpc++_reflection)
set(_PROTOBUF_PROTOC $<TARGET_FILE:protoc>)
set(_GRPC_GRPCPP grpc++)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)

get_filename_component(hw_proto "hello.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/hello.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/hello.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/hello.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/hello.grpc.pb.h")

add_custom_command(
    OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
    COMMAND ${_PROTOBUF_PROTOC}
    ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hw_proto}"
    DEPENDS "${hw_proto}")

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_library(hw_grpc_proto
    ${hw_grpc_srcs}
    ${hw_grpc_hdrs}
    ${hw_proto_srcs}
    ${hw_proto_hdrs})

target_link_libraries(hw_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})

foreach(_target
    master_app mapper_app reducer_app)
    add_executable(${_target} ${_target}.cc)
    target_link_libraries(${_target} 
    hw_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endforeach()

