# libmodbus++

## Modbus Mapping
When defining a device context, you specify a mapping (typically loaded from
JSON) that defines the registers and bits that can be used for that device.

```json
{
	"REGISTER_NAME": {
		"addr": 1234,
		"format": "u16",
		"len": 123,
		"type": "input",
		"scale": 10
	}
}
```

| Field    | Optional? | Description                                               |
| -------- | --------- | --------------------------------------------------------- |
| `addr`   | No        | Specifies the address of the register                     |
| `format` | No        | Specifies the format of the data                          |
| `len`    | By Format | How many registers for variable length types              |
| `type`   | Yes       | `"input"` or `"hold"`, defauts to `"hold"`                |
| `scale`  | Yes       | The stored value is divided by this before being returned |
| `trim`   | Yes       | Trims whitespace on strings, defaults to true             |

### `format`
| Format | Length | Description                |
| ------ | ------ | -------------------------- |
| bit    | 1      | Single bit, coil or input  |
| u16    | 1      | Unsigned 16-bit integer    |
| i16    | 1      | Signed 16-bit integer      |
| u32    | 2      | Unsigned 32-bit integer    |
| i32    | 2      | Signed 32-bit integer      |
| u64    | 4      | Unsigned 64-bit integer    |
| i64    | 4      | Signed 64-bit integer      |
| f32    | 2      | Single precision float     |
| f64    | 4      | Double precision float     |
| str    | varies | Alias for `str_ab`         |
| str_ab | varies | String, high then low byte |
| str_a  | varies | String, high byte only     |
| str_b  | varies | String, low byte only      |
| str_ba | varies | String, low then high byte |

#### Note for `str`
Strings can be handled in a variety of ways in modbus. The most typical is to
store the first character in the high byte and the second character in the low
byte. Then these 16-bit registers are repeated for a set number of characters.
