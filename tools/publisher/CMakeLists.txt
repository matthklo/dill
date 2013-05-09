CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE_DIRECTORIES("../../dep/boost_1_53_0")
INCLUDE_DIRECTORIES("../../libdill/inc")

ADD_EXECUTABLE(dill_publisher
  publisher.cpp
)
TARGET_LINK_LIBRARIES(dill_publisher dill)
SET_PROPERTY(TARGET dill_publisher PROPERTY RUNTIME_OUTPUT_DIRECTORY ../../bin)


