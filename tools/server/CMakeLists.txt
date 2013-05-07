CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(dill_server)

INCLUDE_DIRECTORIES("../../dep/boost_1_53_0")
INCLUDE_DIRECTORIES("../../libdill/inc")

ADD_EXECUTABLE(dill_server
  server.cpp
)
TARGET_LINK_LIBRARIES(dill_server dill)
SET_PROPERTY(TARGET dill_server PROPERTY RUNTIME_OUTPUT_DIRECTORY ../../build/bin)


