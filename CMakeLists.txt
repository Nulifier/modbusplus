cmake_minimum_required(VERSION 3.22)

project(modbusplus VERSION 1.0.0
                       DESCRIPTION "Lua library for interacting with Modbus over TCP and RS485"
					   LANGUAGES CXX)

# These options deal with some variants of the libmodbus library
option(MODBUSPLUS_COMPAT_HWSW_FLOWCONTROL "Adds a hardware and software flow control to the RTU constructor" OFF)
option(MODBUSPLUS_COMPAT_READ_REG_8BIT "modbus_read_registers and modbus_read_input_registers take uint8_t*" OFF)
option(MODBUSPLUS_COMPAT_WRITE_BITS_16BIT "modbus_write_bits takes uint16_t*" OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE STREQUAL Release)
	set(MODBUSPLUS_STACK_CHECK OFF)
else()
	set(MODBUSPLUS_STACK_CHECK ON)
endif()

# Only do these if this is the main project
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	# Use folders in IDEs
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)

	include(CTest)

	# find_package(Doxygen)
	# if (DOXYGEN_FOUND)
	# 	add_subdirectory(docs)
	# else()
	# 	message(STATUS "Doxygen not found, documentation target will not be available")
	# endif()
endif()

find_package(Lua 5.1 REQUIRED)
find_package(libmodbus 3.1 REQUIRED)

set(INCLUDES
	inc/lua-modbusplus.h
	src/lua-modbusplus-private.hpp
	src/modbus-device.hpp
	src/value-utils.hpp
	src/modbus-device-ctx.hpp
	src/mapping-registry.hpp
	src/mapping.hpp
	src/nlohmann/json.hpp
)

set(SOURCES
	src/lua-modbusplus.cpp
	src/modbus-device.cpp
	src/value-utils.cpp
	src/modbus-device-ctx.cpp
	src/mapping-registry.cpp
	src/mapping.cpp
)

configure_file(
	src/modbusplus-config.hpp.in
	${CMAKE_CURRENT_BINARY_DIR}/inc/modbusplus-config.hpp
)

add_library(modbusplus SHARED
	${INCLUDES}
	${SOURCES}
)

target_compile_features(modbusplus PRIVATE cxx_std_17)
set_target_properties(modbusplus PROPERTIES
	CXX_EXTENSIONS OFF
	PREFIX ""  # No 'lib' prefix on Unix-like systems
)
	
# Generate export header
include(GenerateExportHeader)
generate_export_header(modbusplus
	BASE_NAME lua_modbusplus
	EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/inc/lua-modbusplus-export.h
)

# Enable Link Time Optimization if supported
include(CheckIPOSupported)
check_ipo_supported(RESULT result)
if(result)
	set_target_properties(modbusplus PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

target_include_directories(modbusplus PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}/inc
	${CMAKE_CURRENT_SOURCE_DIR}/inc
	${CMAKE_CURRENT_SOURCE_DIR}/src

	${LUA_INCLUDE_DIR}
)

target_link_libraries(modbusplus PRIVATE
	${LUA_LIBRARIES}
	libmodbus
)

install(TARGETS modbusplus
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib/static
)

if ((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING) AND BUILD_TESTING)
	enable_testing()
	add_subdirectory(tests)
endif()
