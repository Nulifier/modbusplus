find_path(LIBMODBUS_INCLUDE_DIR
	NAMES
		modbus/modbus.h
	PATHS
		/usr/local/include
		/usr/include
)

find_library(LIBMODBUS_LIBRARY
	NAMES
		modbus
	PATHS
		/usr/local/lib
		/usr/lib
)

if(LIBMODBUS_LIBRARY)
	set(LIBMODBUS_LIBRARIES ${LIBMODBUS_LIBRARY} CACHE STRING "libmodbus library")
endif()

if(LIBMODBUS_INCLUDE_DIR)
	file(STRINGS ${LIBMODBUS_INCLUDE_DIR}/modbus/modbus-version.h modbus_version_str REGEX "#define[ \t]+LIBMODBUS_VERSION_MAJOR")
	string(REGEX MATCH "[0-9]+" modbus_version_major "${modbus_version_str}")
	file(STRINGS ${LIBMODBUS_INCLUDE_DIR}/modbus/modbus-version.h modbus_version_str REGEX "#define[ \t]+LIBMODBUS_VERSION_MINOR")
	string(REGEX MATCH "[0-9]+" modbus_version_minor "${modbus_version_str}")
	file(STRINGS ${LIBMODBUS_INCLUDE_DIR}/modbus/modbus-version.h modbus_version_str REGEX "#define[ \t]+LIBMODBUS_VERSION_MICRO")
	string(REGEX MATCH "[0-9]+" modbus_version_micro "${modbus_version_str}")

	set(LIBMODBUS_VERSION "${modbus_version_major}.${modbus_version_minor}.${modbus_version_micro}")

	unset(modbus_version_str)
	unset(modbus_version_major)
	unset(modbus_version_minor)
	unset(modbus_version_micro)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libmodbus
	REQUIRED_VARS LIBMODBUS_LIBRARIES LIBMODBUS_INCLUDE_DIR
	VERSION_VAR LIBMODBUS_VERSION
	HANDLE_VERSION_RANGE
)

add_library(libmodbus INTERFACE)
target_include_directories(libmodbus INTERFACE ${LIBMODBUS_INCLUDE_DIR})
target_link_libraries(libmodbus INTERFACE ${LIBMODBUS_LIBRARIES})
