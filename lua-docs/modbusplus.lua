--- @meta

--- @class ModbusDevice
local ModbusDevice = {}

--- @alias ModbusDevice.RtuConfig { device: string, baud: integer, parity?: "N" | "E" | "O", data_bits?: 5 | 6 | 7 | 8, stop_bits?: 1 | 2, flowctrl?: "None" | "HW" | "SW" }
--- @alias ModbusDevice.TcpConfig { ip: string, port: integer }

--- Creates a new ModbusDevice object.
--- @param config ModbusDevice.RtuConfig Configuration for the Modbus device.
--- @return ModbusDevice
function ModbusDevice.newRtu(config) end

--- Creates a new ModbusDevice object.
--- @param config ModbusDevice.TcpConfig Configuration for the Modbus device.
--- @return ModbusDevice
function ModbusDevice.newTcp(config) end

--- Connects to the Modbus device.
--- @return nil
function ModbusDevice:raw_connect() end

--- Closes the Modbus device.
--- @return nil
function ModbusDevice:raw_close() end

--- Flushes the Modbus device.
--- @return nil
function ModbusDevice:raw_flush() end

--- Sets the slave ID for the Modbus device.
--- @param slave integer Slave ID to set.
--- @return nil
function ModbusDevice:raw_set_slave(slave) end

--- Reads the status of the num_bits coils starting from addr.
--- @param addr integer Starting address.
--- @param num_bits integer Number of bits to read.
--- @return boolean[] # Array of boolean values representing the state of each coil.
function ModbusDevice:raw_read_bits(addr, num_bits) end

--- Reads the status of the num_bits input bits starting from addr.
--- @param addr integer Starting address.
--- @param num_bits integer Number of bits to read.
--- @return boolean[] # Array of boolean values representing the state of each input bit.
function ModbusDevice:raw_read_input_bits(addr, num_bits) end

--- Reads the values of the num_registers holding registers starting from addr.
--- @param addr integer Starting address.
--- @param num_registers integer Number of registers to read.
--- @return integer[] # Array of integers representing the values of each holding register.
function ModbusDevice:raw_read_registers(addr, num_registers) end

--- Reads the values of the num_registers input registers starting from addr.
--- @param addr integer Starting address.
--- @param num_registers integer Number of registers to read.
--- @return integer[] # Array of integers representing the values of each input register.
function ModbusDevice:raw_read_input_registers(addr, num_registers) end

--- Writes a single coil at addr with the specified value.
--- @param addr integer Address of the coil to write.
--- @param value boolean Value to write (true for ON, false for OFF).
--- @return nil
function ModbusDevice:raw_write_bit(addr, value) end

--- Writes multiple coils starting from addr with the specified values.
--- @param addr integer Starting address.
--- @param values boolean[] Array of boolean values to write (true for ON, false for OFF).
--- @return nil
function ModbusDevice:raw_write_bits(addr, values) end

--- Writes a single holding register at addr with the specified value.
--- @param addr integer Address of the register to write.
--- @param value integer Value to write (0-65535).
--- @return nil
function ModbusDevice:raw_write_register(addr, value) end

--- Writes multiple holding registers starting from addr with the specified values.
--- @param addr integer Starting address.
--- @param values integer[] Array of integer values to write (each 0-65535).
--- @return nil
function ModbusDevice:raw_write_registers(addr, values) end

--- @alias ModbusDevice.Format "bit" | "u16" | "i16" | "u32" | "i32" | "u64" | "i64" | "f32" | "f64"
--- @alias ModbusDevice.Mapping { addr: integer, format: ModbusDevice.Format, type?: "input" | "hold", scale?: number }
--- @alias ModbusDevice.StrMapping { addr: integer, format: "str" | "str_ab" | "str_ba" | "str_a" | "str_b", length: integer, type?: "input" | "hold", trim?: boolean }

--- Creates a new context for high-level operations based on the provided configuration.
--- @param slave integer Slave ID for the context.
--- @param mapping (ModbusDevice.Mapping | ModbusDevice.StrMapping)[] Array of mapping configurations.
--- @return ModbusDeviceContext
function ModbusDevice:newContext(slave, mapping) end

--- @class ModbusDeviceContext
local ModbusDeviceContext = {}

--- Reads the value associated with the given name from the context.
--- @param name string Name of the variable to read.
--- @return any # The value associated with the name, type depends on the mapping configuration.
function ModbusDeviceContext:read(name) end

--- Writes the given data to the variable associated with the given name in the context.
--- @param name string Name of the variable to write to.
--- @param data any Data to write, type depends on the mapping configuration.
--- @return nil
function ModbusDeviceContext:write(name, data) end

return ModbusDevice
