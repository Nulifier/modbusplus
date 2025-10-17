local modbus = require("modbusplus")

local device = modbus.newRtu({
	device = "/dev/ttyUSB0",
	baud = 9600,
	parity = "N",
	data_bits = 8,
	stop_bits = 1
})

-- Raw device operations
device:raw_connect()
device:raw_set_slave(1)
local coils = device:raw_read_bits(0, 10)

-- Context operations

local contextConfig = {
	slave = 1,
	mapping = {
		PHASE = {
			addr = 0,
			len = 1,
			format = "u16",
			type = "inReg"
		},
		FLAME_INTENSITY = {
			addr = 1,
			len = 1,
			format = "u16",
			type = "inReg",
			scale = 0.1
		},
	}
}

local context = device:newContext(contextConfig)

local phase = context:read("PHASE")
