macro(compileAsC99)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_C_FLAGS "--std=c99 -g ${CMAKE_C_FLAGS}")
      set (CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_CXX_FLAGS}")
    endif()
  else()
    set (CMAKE_C_STANDARD 99)
    set (CMAKE_CXX_STANDARD 11)
  endif()
endmacro(compileAsC99)

cmake_minimum_required(VERSION 2.8.11)
compileAsC99()

set(AZUREIOT_INC_FOLDER ".." "../parson" "/usr/include/azureiot" "/usr/local/include/azureiot" "/usr/include/azureiot/inc" "../lib")

include_directories(${AZUREIOT_INC_FOLDER})

add_subdirectory(lib)

set(sendIoTHub_c_files
  ./parson/parson.c
  ./remote_monitoring.c
  ./rgbLed.c
  ./network.c
  ./lib/libRotaryEncoder.c
  ./lib/BME280.c
  ./lib/lib_mcp3425.c
  ./daemonize/daemonize.c
  ./main.c
  )

set(sendIoTHub_h_files
  ./parson/parson.h
  ./remote_monitoring.h
  ./rgbLed.h
  ./network.h
  ./lib/libRotaryEncoder.h
  ./lib/BME280.h
  ./lib/lib_mcp3425.h
  ./daemonize/daemonize.h
  )




add_executable(sendIoTHub ${sendIoTHub_c_files} ${sendIoTHub_h_files})


target_link_libraries(sendIoTHub
  serializer
  iothub_client
  iothub_client_mqtt_transport
  aziotsharedutil
  umqtt
  pthread
  curl
  ssl
  crypto
  m
  wiringPi
  wiringPiDev
  lib
  )
