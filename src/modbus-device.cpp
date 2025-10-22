#include "modbus-device.hpp"
#include <modbus/modbus.h>
#include <stdexcept>
#include <value-utils.hpp>

ModbusDevice::ModbusDevice(modbus_t* ctx) : m_ctx(ctx) {
	if (m_ctx == nullptr) {
		throw std::runtime_error("Failed to create Modbus context");
	}
}

ModbusDevice::~ModbusDevice() {
	if (m_ctx) {
		modbus_free(m_ctx);
		m_ctx = nullptr;
	}
}

void ModbusDevice::connect() {
	if (modbus_connect(m_ctx) == -1) {
		throw std::runtime_error(modbus_strerror(errno));
	}
	m_connected = true;
}

void ModbusDevice::close() noexcept {
	m_connected = false;
	if (m_ctx) {
		modbus_close(m_ctx);
	}
}

unsigned int ModbusDevice::flush() {
	int rc = modbus_flush(m_ctx);
	if (rc == -1) {
		throw std::runtime_error(modbus_strerror(errno));
	}
	return static_cast<unsigned int>(rc);
}

void ModbusDevice::setSlave(int slave) {
	if (modbus_set_slave(m_ctx, slave) == -1) {
		throw std::runtime_error(modbus_strerror(errno));
	}
}

unsigned int ModbusDevice::readBits(int addr, int nb, uint8_t* dest) {
	int rc = modbus_read_bits(m_ctx, addr, nb, dest);
	if (rc == -1) {
		throw std::runtime_error(modbus_strerror(errno));
	}
	return static_cast<unsigned int>(rc);
}

unsigned int ModbusDevice::readInputBits(int addr, int nb, uint8_t* dest) {
	int rc = modbus_read_input_bits(m_ctx, addr, nb, dest);
	if (rc == -1) {
		throw std::runtime_error(modbus_strerror(errno));
	}
	return static_cast<unsigned int>(rc);
}

unsigned int ModbusDevice::readRegisters(int addr, int nb, uint16_t* dest) {
#ifndef MODBUSPLUS_COMPAT_READ_REG_8BIT
	int rc = modbus_read_registers(m_ctx, addr, nb, dest);
#else
	int rc = modbus_read_registers(m_ctx, addr, nb,
								   reinterpret_cast<uint8_t*>(dest));

	// Swap bytes for each register to convert to big-endian
	for (int i = 0; i < nb; ++i) {
		dest[i] = (dest[i] >> 8) | (dest[i] << 8);
	}
#endif
	if (rc == -1) {
		throw std::runtime_error(modbus_strerror(errno));
	}
	return static_cast<unsigned int>(rc);
}

unsigned int ModbusDevice::readInputRegisters(int addr,
											  int nb,
											  uint16_t* dest) {
#ifndef MODBUSPLUS_COMPAT_READ_REG_8BIT
	int rc = modbus_read_input_registers(m_ctx, addr, nb, dest);
#else
	int rc = modbus_read_input_registers(m_ctx, addr, nb,
										 reinterpret_cast<uint8_t*>(dest));

	// Swap bytes for each register to convert to big-endian
	for (int i = 0; i < nb; ++i) {
		dest[i] = (dest[i] >> 8) | (dest[i] << 8);
	}
#endif
	if (rc == -1) {
		throw std::runtime_error(modbus_strerror(errno));
	}
	return static_cast<unsigned int>(rc);
}

unsigned int ModbusDevice::writeBit(int addr, uint8_t value) {
	int rc = modbus_write_bit(m_ctx, addr, value);
	if (rc == -1) {
		throw std::runtime_error(modbus_strerror(errno));
	}
	return static_cast<unsigned int>(rc);
}

unsigned int ModbusDevice::writeBits(int addr, int nb, const uint8_t* src) {
#ifndef MODBUSPLUS_COMPAT_WRITE_BITS_16BIT
	int rc = modbus_write_bits(m_ctx, addr, nb, src);
#else
	auto vec = value_utils::pack_coils_to_u16(src, nb);
	int rc = modbus_write_bits(m_ctx, addr, nb, vec.data());
#endif
	if (rc == -1) {
		throw std::runtime_error(modbus_strerror(errno));
	}
	return static_cast<unsigned int>(rc);
}

unsigned int ModbusDevice::writeRegister(int addr, uint16_t value) {
	int rc = modbus_write_register(m_ctx, addr, value);
	if (rc == -1) {
		throw std::runtime_error(modbus_strerror(errno));
	}
	return static_cast<unsigned int>(rc);
}

unsigned int ModbusDevice::writeRegisters(int addr,
										  int nb,
										  const uint16_t* src) {
	int rc = modbus_write_registers(m_ctx, addr, nb, src);
	if (rc == -1) {
		throw std::runtime_error(modbus_strerror(errno));
	}
	return static_cast<unsigned int>(rc);
}

#ifndef MODBUSPLUS_COMPAT_HWSW_FLOWCONTROL
ModbusDeviceRtu::ModbusDeviceRtu(const char* device,
								 int baud,
								 ModbusDeviceRtu::Parity parity,
								 ModbusDeviceRtu::DataBits data_bits,
								 ModbusDeviceRtu::StopBits stop_bits)
	: ModbusDevice(modbus_new_rtu(device,
								  baud,
								  static_cast<char>(parity),
								  static_cast<int>(data_bits),
								  static_cast<int>(stop_bits))) {
	modbus_rtu_set_serial_mode(m_ctx, MODBUS_RTU_RS485);
}
#else
ModbusDeviceRtu::ModbusDeviceRtu(const char* device,
								 int baud,
								 ModbusDeviceRtu::Parity parity,
								 ModbusDeviceRtu::DataBits data_bits,
								 ModbusDeviceRtu::StopBits stop_bits,
								 ModbusDeviceRtu::FlowControl flow_control)
	: ModbusDevice(modbus_new_rtu(device,
								  baud,
								  static_cast<char>(parity),
								  static_cast<int>(data_bits),
								  static_cast<int>(stop_bits),
								  static_cast<int>(flow_control))) {
	modbus_rtu_set_serial_mode(m_ctx, MODBUS_RTU_RS485);
}
#endif

ModbusDeviceTcp::ModbusDeviceTcp(const char* ip, int port)
	: ModbusDevice(modbus_new_tcp(ip, port)) {}
