#pragma once

#include <cstdint>
#include "modbusplus-config.hpp"

// Forward declaration of modbus_t
typedef struct _modbus modbus_t;

class ModbusDevice {
   public:
	ModbusDevice() = delete;
	ModbusDevice(const ModbusDevice&) = delete;
	ModbusDevice& operator=(const ModbusDevice&) = delete;
	ModbusDevice(ModbusDevice&&) = delete;
	ModbusDevice& operator=(ModbusDevice&&) = delete;

	virtual ~ModbusDevice();

	void connect();
	void close() noexcept;
	unsigned int flush();

	void setSlave(int slave);

	bool isConnected() const noexcept {
		return m_connected;
	}

	/**
	 * Read bits (coils) from the Modbus device.
	 * @param addr The starting address to read from.
	 * @param nb The number of bits to read.
	 * @param dest Pointer to the destination buffer to store the read bits.
	 * @return The number of bits read.
	 */
	unsigned int readBits(int addr, int nb, uint8_t* dest);

	/**
	 * Read input bits (discrete inputs) from the Modbus device.
	 * @param addr The starting address to read from.
	 * @param nb The number of input bits to read.
	 * @param dest Pointer to the destination buffer to store the read input
	 * bits.
	 * @return The number of input bits read.
	 */
	unsigned int readInputBits(int addr, int nb, uint8_t* dest);

	/**
	 * Read holding registers from the Modbus device.
	 * @param addr The starting address to read from.
	 * @param nb The number of registers to read.
	 * @param dest Pointer to the destination buffer to store the read
	 * registers.
	 * @return The number of registers read.
	 */
	unsigned int readRegisters(int addr, int nb, uint16_t* dest);

	/**
	 * Read input registers from the Modbus device.
	 * @param addr The starting address to read from.
	 * @param nb The number of input registers to read.
	 * @param dest Pointer to the destination buffer to store the read input
	 * registers.
	 * @return The number of input registers read.
	 */
	unsigned int readInputRegisters(int addr, int nb, uint16_t* dest);

	/**
	 * Write a single bit (coil) to the Modbus device.
	 * @param addr The address to write to.
	 * @param value The value to write (true or false).
	 * @return The number of bits written (1 if successful).
	 */
	unsigned int writeBit(int addr, uint8_t value);

	/**
	 * Write multiple bits (coils) to the Modbus device.
	 * @param addr The starting address to write to.
	 * @param nb The number of bits to write.
	 * @param src Pointer to the source buffer containing the bits to write.
	 * @return The number of bits written.
	 */
	unsigned int writeBits(int addr, int nb, const uint8_t* src);

	/**
	 * Write a single holding register to the Modbus device.
	 * @param addr The address to write to.
	 * @param value The value to write.
	 * @return The number of registers written (1 if successful).
	 */
	unsigned int writeRegister(int addr, uint16_t value);

	/**
	 * Write multiple holding registers to the Modbus device.
	 * @param addr The starting address to write to.
	 * @param nb The number of registers to write.
	 * @param src Pointer to the source buffer containing the registers to
	 * write.
	 * @return The number of registers written.
	 */
	unsigned int writeRegisters(int addr, int nb, const uint16_t* src);

   protected:
	ModbusDevice(modbus_t* ctx);

	bool m_connected = false;
	modbus_t* m_ctx;
};

class ModbusDeviceRtu : public ModbusDevice {
   public:
	enum class Parity { None = 'N', Even = 'E', Odd = 'O' };
	enum class DataBits { Five = 5, Six = 6, Seven = 7, Eight = 8 };
	enum class StopBits { One = 1, Two = 2 };

#ifndef MODBUSPLUS_COMPAT_HWSW_FLOWCONTROL
	ModbusDeviceRtu(const char* device,
					int baud,
					Parity parity = Parity::None,
					DataBits data_bits = DataBits::Eight,
					StopBits stop_bits = StopBits::One);
#else
	enum class FlowControl { None = 0, Hardware = 1, Software = 2 };
	ModbusDeviceRtu(const char* device,
					int baud,
					Parity parity = Parity::None,
					DataBits data_bits = DataBits::Eight,
					StopBits stop_bits = StopBits::One,
					FlowControl flow_control = FlowControl::None);
#endif
};

class ModbusDeviceTcp : public ModbusDevice {
   public:
	ModbusDeviceTcp(const char* ip, int port);
};
