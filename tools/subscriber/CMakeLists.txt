CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE_DIRECTORIES(${DILL_LIB_HEADER_DIRECTORY})

IF(WIN32)
ADD_DEFINITIONS(-DUNICODE -D_UNICODE)
ENDIF(WIN32)

ADD_EXECUTABLE(dill_subscriber
  subscriber.cpp
)
TARGET_LINK_LIBRARIES(dill_subscriber dill)
SET_PROPERTY(TARGET dill_subscriber PROPERTY RUNTIME_OUTPUT_DIRECTORY ${DILL_BIN_OUTPUT_DIRECTORY})

