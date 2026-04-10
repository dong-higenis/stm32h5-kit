cmake_minimum_required(VERSION 3.13)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools")
include(arm-none-eabi-gcc)


# 프로젝트 이름은 최종 출력 파일 이름으로 사용된다.
#
set(PRJ_NAME stm32h5-fw)


project(${PRJ_NAME}
  LANGUAGES ASM C CXX
)


set(EXECUTABLE ${PRJ_NAME}.elf)



# 지정한 폴더에 있는 파일만 포함한다.
#
file(GLOB SRC_FILES CONFIGURE_DEPENDS
  src/*.cpp
  src/*.c
  src/ap/*.cpp
  src/ap/*.c
  src/bsp/*.c
  src/bsp/rtos/*.c  
  src/bsp/device/*.c


  # LittleFS
  src/lib/littlefs/*.c

  # FATFS
  src/lib/fatfs/*.c

  # LwIP
  src/lib/lwip/app/*.c
  src/lib/lwip/target/*.c

  # LAN8742
  src/lib/ST/BSP/*.c

  # FreeRTOS
  src/lib/FreeRTOS/Source/*.c  
  src/lib/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/*.c
  src/lib/FreeRTOS/Source/CMSIS_RTOS/*.c  
  src/lib/FreeRTOS/Source/portable/MemMang/heap_4.c  
)

# 하위폴더에 있는 파일까지 포함한다.
#
file(GLOB_RECURSE SRC_FILES_RECURSE CONFIGURE_DEPENDS
  src/ap/*.cpp
  src/ap/*.c
  src/bsp/*.s
  src/common/*.c
  src/hw/*.c
  src/lib/ST/STM32H5xx_HAL_Driver/Src/*.c

  src/lib/wifi/WiFiEspAT/*.cpp
  src/lib/wifi/Arduino_JSON/src/*.c
  src/lib/wifi/ArduinoJson/src/*.cpp
  src/lib/wifi/PubSubClient/src/*.cpp
  src/lib/lwip/src/*.c
  src/lib/lwip/system/OS/*.c
  )

# 특정 폴더를 빌드에서 제외한다.
#
set(EXCLUDE_PATHS 
)

foreach(TMP_PATH ${SRC_FILES_RECURSE}) 
  foreach(EXCLUDE_PATH ${EXCLUDE_PATHS}) 
    string(FIND ${TMP_PATH} ${EXCLUDE_PATH} RESULT) 
    if(NOT ${RESULT} EQUAL -1) 
      list(REMOVE_ITEM SRC_FILES_RECURSE ${TMP_PATH}) 
    endif() 
  endforeach(EXCLUDE_PATH) 
endforeach(TMP_PATH)


add_executable(${EXECUTABLE} 
  ${SRC_FILES}
  ${SRC_FILES_RECURSE}
  ${QMK_SRC_FILES}
)



target_include_directories(${EXECUTABLE} PRIVATE 
  src  
  src/ap
  src/ap/modules
  src/ap/modules/common
  src/bsp
  src/bsp/device
  src/bsp/rtos  
  src/common 
  src/common/core
  src/common/hw/include 
  src/hw
  src/hw/driver
  src/lib

  src/lib/ST/CMSIS/Include
  src/lib/ST/CMSIS/Device/ST/STM32H5xx/Include
  src/lib/ST/STM32H5xx_HAL_Driver/Inc

  # FreeRTOS
  src/lib/FreeRTOS/Source/CMSIS_RTOS
  src/lib/FreeRTOS/Source/include
  src/lib/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure

  # WiFiEspAT
  src/lib/wifi/WiFiEspAT/src/core
  src/lib/wifi/WiFiEspAT/src  

  # ArduinoJSON
  src/lib/wifi/ArduinoJson/src

  # PubSubClient
  src/lib/wifi/PubSubClient/src

  # FATFS
  src/lib/fatfs

  # LwIP
  src/lib/lwip/target
  src/lib/lwip/src/include
  src/lib/lwip/system

  # LAN8742
  src/lib/ST/BSP
)

target_compile_definitions(${EXECUTABLE} PRIVATE
  -DSTM32H563xx
  -DUSE_HAL_DRIVER  
  -DLFS_THREADSAFE
  )

target_compile_options(${EXECUTABLE} PRIVATE
  -mcpu=cortex-m33
  -mthumb
  -mfpu=fpv4-sp-d16
  -mfloat-abi=hard

  -fdata-sections
  -ffunction-sections
  
  -fstack-usage 
  -Wmissing-field-initializers
  -Wuninitialized 
  # -Wenum-conversion

  -Wall
  -g3
  -Og
  )

target_link_options(${EXECUTABLE} PRIVATE
  -T../src/bsp/ldscript/STM32H563xx_FLASH.ld

  -mcpu=cortex-m33
  -mthumb
  -mfpu=fpv4-sp-d16
  -mfloat-abi=hard
  -specs=nano.specs
  -lc
  -lm
  -lstdc++
  -lsupc++
  -fno-rtti
  
  -Wl,-Map=${PRJ_NAME}.map,--cref
  -Wl,--gc-sections
  -Xlinker -print-memory-usage -Xlinker
  )

set (CMAKE_C_FLAGS "-Wenum-conversion")
set (CMAKE_CXX_FLAGS "-fno-rtti")

message(${CMAKE_CXX_COMPILER_VERSION})
if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11)
  string(APPEND CMAKE_EXE_LINKER_FLAGS "-Wl,--no-warn-rwx-segments")
endif()

add_custom_command(TARGET ${EXECUTABLE} 
  POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} ARGS -O binary ${EXECUTABLE} ${PROJECT_NAME}.bin
  COMMENT "Invoking: Make Bin -> ${KEYBOARD_PATH}"
  )  
  

