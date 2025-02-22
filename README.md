# MemoryList

A lightweight, efficient FIFO list implementation for storing JSON objects on SD card, optimized for ESP32 and similar microcontrollers.

## Features

- Efficient buffered reading and writing
- Tombstone-based deletion for fast remove operations
- Automatic defragmentation
- Thread-safe file operations
- Comprehensive error handling
- Extensive test coverage

## Installation

1. Clone this repository into your Arduino/libraries folder:
```bash
cd ~/Arduino/libraries
git clone https://github.com/yourusername/MemoryList.git
```

2. Include in your project:
```cpp
#include <MemoryList.h>
```

## Usage

```cpp
// Create or open a list
MemoryList list("/data.txt");

// Push data
JsonDocument doc;
doc["id"] = 123;
doc["timestamp"] = 1645567890;
list.push(doc.as<JsonObjectConst>());

// Get last entry
String lastEntry = list.getLast();

// Remove entry
list.remove(0);

// Get statistics
JsonDocument stats = list.getStats();
Serial.println("Size: " + String(stats["size"].as<size_t>()));
```

## Performance Characteristics

- Push: O(1) - Constant time append
- Get: O(n) - Linear scan
- getLast: O(b) - Where b is number of buffers from end
- Remove: O(1) - Uses tombstoning
- Defragment: O(n) - Full file rewrite

## Memory Usage

- Static buffer: 512 bytes
- Stack usage: ~1KB
- Heap usage: Minimal, mainly for String operations

## Limitations

- Maximum entry size: ~512 bytes (single buffer)
- Recommended for small to medium datasets (<100MB)
- Requires SD card support

### Using Wokwi Simulator

The project includes Wokwi simulation support with preconfigured `diagram.json` and `wokwi.toml`.

1. Install Wokwi for VS Code
2. Open the project in VS Code
3. Press F1 to open commands and select ">Wokwi: Start Simulator"

### Test Environments
Each environment is configured for different testing scenarios:
```bash
# Unity framework tests
pio test --without-uploading --without-testing --no-reset -e test -v

# ESP32 hardware tests
pio test --without-uploading --without-testing --no-reset -e esp32dev -v

# UPESY WROOM specific tests
pio test --without-uploading --without-testing --no-reset -e upesy_wroom -v

# Example sketches validation
pio test --without-uploading --without-testing --no-reset -e examples -v
```

### Dependencies

- ArduinoJson (^7.1)
- StreamUtils
- Unity (for tests)

## Testing

### Using Wokwi Simulator
```bash
# Build test firmware
pio run -e test

# The .pio/build/test/firmware.elf file will be used by Wokwi
```

1. Install Wokwi for VS Code
2. Open the project in VS Code
3. Press F1 and select "Wokwi: Start Simulator"
4. The simulator will automatically load the test firmware

### Project Structure
```
Arduino-SD-List/
├── src/             # Source files
├── test/            # Test files
│   ├── unity/       # Unity framework tests
│   └── esp32/       # ESP32 specific tests
├── wokwi.toml       # Wokwi configuration
└── diagram.json     # Wokwi hardware configuration
```


## Contributing

Contributions are welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.