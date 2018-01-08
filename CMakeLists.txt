cmake_minimum_required(VERSION 2.8.11)

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
compileAsC99()
project (sendIoTHubSenser)

# インクルードファイルの探索パスにバイナリツリーを追加
include_directories("${PROJECT_BINARY_DIR}")

#プロジェクトディレクトリのファイルを追加
set(sendIoTHub_c_files
  ./remote_monitoring.c
  ./rgbLed.c
  ./network.c
  ./main.c
  )
set(sendIoTHub_h_files
  ./remote_monitoring.h
  ./rgbLed.h
  ./network.h
  )

add_executable(sendIoTHub ${sendIoTHub_c_files} ${sendIoTHub_h_files})

# インストールターゲットを追加
# AzureIoT SDKの追加
set(AZUREIOT_INC_FOLDER
  ".."
  "/usr/include/azureiot"
  "/usr/local/include/azureiot"
  "/usr/include/azureiot/inc"
  )
include_directories(${AZUREIOT_INC_FOLDER})

#サブディレクトリに配置したライブラリの追加
include_directories ("${PROJECT_SOURCE_DIR}/parson")
add_subdirectory (parson)
set (EXTRA_LIBS ${EXTRA_LIBS} parsonFunctions)

include_directories ("${PROJECT_SOURCE_DIR}/lib")
add_subdirectory (lib)
set (EXTRA_LIBS ${EXTRA_LIBS} aquaBoardFunctions)

include_directories ("${PROJECT_SOURCE_DIR}/daemonize")
add_subdirectory (daemonize)
set (EXTRA_LIBS ${EXTRA_LIBS} daemonizeFunctions)

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
  ${EXTRA_LIBS}
  )

#### インストールターゲットを追加
# CMAKE_INSTALL_PREFIX (default=/usr/local)以下に install される
install (TARGETS sendIoTHub DESTINATION bin)
install (FILES "${PROJECT_SOURCE_DIR}/sendIoTHubSenser.h"
DESTINATION include)
install (FILES "${PROJECT_SOURCE_DIR}/remote_monitoring.h"
DESTINATION include)
install (FILES "${PROJECT_SOURCE_DIR}/sendIoTHubSenser.conf"
DESTINATION /etc/)
install (FILES "${PROJECT_SOURCE_DIR}/sendIoTHubSenser.service"
DESTINATION /etc/systemd/system/)
