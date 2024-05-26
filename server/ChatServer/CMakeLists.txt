cmake_minimum_required(VERSION 3.10)

project(ChatServer)

file(GLOB_RECURSE ICD_FILES "./*.h")
file(GLOB_RECURSE SRC_FILES "./*.cpp" "./*.cc")

add_executable(ChatServer ${ICD_FILES}  ${SRC_FILES})

target_link_libraries (ChatServer PUBLIC fmt)
target_link_libraries (ChatServer PUBLIC pthread)
target_link_libraries (ChatServer PUBLIC spdlog)
target_link_libraries (ChatServer PUBLIC jsoncpp)
target_link_libraries (ChatServer PUBLIC protobuf)
target_link_libraries (ChatServer PUBLIC hiredis)
target_link_libraries (ChatServer PUBLIC grpc++)
target_link_libraries (ChatServer PUBLIC grpc)
target_link_libraries (ChatServer PUBLIC mysqlcppconn)
target_link_libraries (ChatServer PUBLIC boost_system)
target_link_libraries (ChatServer PUBLIC boost_filesystem)
