cmake_minimum_required(VERSION 3.18)

project(FanPiPWMld)
if(EXISTS "/proc/cpuinfo")
    file(READ "/proc/cpuinfo" CPUINFO)
    string(FIND "${CPUINFO}" "Raspberry Pi" IS_RPI_POS)
    if (IS_RPI_POS GREATER -1)
        set(PLATFORM_BOARD "RASPBERRYPI")
    endif()
endif()

if(NOT DEFINED PLATFORM_BOARD)
    if (EXISTS "/etc/orangepi-release")
        file(READ "/etc/orangepi-release" ORANGE_RELEASE)
        string(FIND "${ORANGE_RELEASE}" "orangepi" IS_OPI_POS)
        if (IS_OPI_POS GREATER -1)
            set(PLATFORM_BOARD "ORANGEPI")
        endif()
    endif()
endif()

if(NOT DEFINED PLATFORM_BOARD)
    set(PLATFORM_BOARD "UNKNOWN")
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config/platform_info.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/platform_info.h
)

set(SOURCE_FanPiPWMld src/FanPiPWMld.cpp)
set(SOURCE_FanMon src/FanMon.cpp)

set(SOURCE_CFGINI src/cfgini.cpp)
set(SOURCE_LOG src/Log.cpp)
set(SOURCE_SHMMEM src/shmMem.cpp)
set(SOURCE_FANPWM src/fanPWM.cpp)
set(SOURCE_FANTEMP src/fanTemp.cpp)
set(SOURCE_FANSTRINGS src/fanStrings.cpp)
set(SOURCE_FANKBD src/fanKbd.cpp)

include_directories(include)

add_library(cfgini STATIC ${SOURCE_CFGINI})
add_library(log STATIC ${SOURCE_LOG})
add_library(shmMem STATIC ${SOURCE_SHMMEM})
add_library(fanPWM STATIC ${SOURCE_FANPWM})
target_include_directories(fanPWM PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
add_library(fanTemp STATIC ${SOURCE_FANTEMP})
add_library(fanStrings STATIC ${SOURCE_FANSTRINGS})
add_library(fanKbd STATIC ${SOURCE_FANKBD})

add_executable(fanpwmd ${SOURCE_FanPiPWMld})
target_include_directories(fanpwmd PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_compile_options(fanpwmd PRIVATE -Wall -O2 -Ofast)
target_link_libraries(fanpwmd cfgini log shmMem fanPWM fanTemp wiringPi rt)

add_executable(fanmon ${SOURCE_FanMon})
target_include_directories(fanmon PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_compile_options(fanmon PRIVATE -Wall -O2 -Ofast)
target_link_libraries(fanmon cfgini shmMem fanStrings fanKbd wiringPi pthread rt)

install(TARGETS fanpwmd DESTINATION bin)
install(TARGETS fanmon DESTINATION bin)

install(FILES ${CMAKE_SOURCE_DIR}/config/fanpwmd.ini.default DESTINATION /usr/local/etc)

configure_file(${CMAKE_SOURCE_DIR}/service/fan_pwm.service.in ${CMAKE_BINARY_DIR}/fan_pwm.service @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/fan_pwm.service DESTINATION /etc/systemd/system)