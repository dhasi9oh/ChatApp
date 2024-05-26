cmake_minimum_required(VERSION 3.10)

project(StatusServer)

link_directories(/usr/local/lib)

file(GLOB_RECURSE ICD_FILES "./*.h")
file(GLOB_RECURSE SRC_FILES "./*.cpp" "./*.cc")

add_executable(StatusServer ${ICD_FILES}  ${SRC_FILES})

target_link_libraries (StatusServer PUBLIC fmt)
target_link_libraries (StatusServer PUBLIC spdlog)
target_link_libraries (StatusServer PUBLIC jsoncpp)
target_link_libraries (StatusServer PUBLIC protobuf)
target_link_libraries (StatusServer PUBLIC hiredis)
target_link_libraries (StatusServer PUBLIC grpc++)
target_link_libraries (StatusServer PUBLIC grpc)
target_link_libraries (StatusServer PUBLIC upb)
target_link_libraries (StatusServer PUBLIC z)
target_link_libraries (StatusServer PUBLIC ssl)
target_link_libraries (StatusServer PUBLIC mysqlcppconn)
target_link_libraries (StatusServer PUBLIC boost_filesystem)
target_link_libraries (StatusServer PUBLIC boost_system)
