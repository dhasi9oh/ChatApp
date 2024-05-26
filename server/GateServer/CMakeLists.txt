cmake_minimum_required(VERSION 3.10)

project(GateServer)

file(GLOB_RECURSE ICD_FILES "./*.h")
file(GLOB_RECURSE SRC_FILES "./*.cpp" "./*.cc")

add_executable(GateServer ${ICD_FILES}  ${SRC_FILES})

target_link_libraries (GateServer PUBLIC fmt)
target_link_libraries (GateServer PUBLIC spdlog)
target_link_libraries (GateServer PUBLIC jsoncpp)
target_link_libraries (GateServer PUBLIC protobuf)
target_link_libraries (GateServer PUBLIC hiredis)
target_link_libraries (GateServer PUBLIC grpc++)
target_link_libraries (GateServer PUBLIC grpc)
target_link_libraries (GateServer PUBLIC mysqlcppconn)
target_link_libraries (GateServer PUBLIC boost_filesystem)
target_link_libraries (GateServer PUBLIC boost_system)
