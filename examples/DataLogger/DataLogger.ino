#include <Arduino.h>
#include <MemoryList.h>
#include <ArduinoJson.h>

// Simulate sensor data
struct SensorData {
    float temperature;
    float humidity;
    uint32_t timestamp;
};

MemoryList dataLogger("/sensor_log.txt");
const uint32_t LOG_INTERVAL = 5000;  // Log every 5 seconds
uint32_t lastLog = 0;
int logCount = 0;
const int MAX_LOGS = 100;  // Maximum number of logs to keep

// Simulate sensor reading
SensorData readSensor() {
    return {
        .temperature = 20.0 + random(-50, 50) / 10.0f,
        .humidity = 50.0 + random(-100, 100) / 10.0f,
        .timestamp = millis()
    };
}

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(100);
    
    if (!SD.begin()) {
        Serial.println("SD Card initialization failed!");
        return;
    }

    dataLogger.clear();
    Serial.println("Starting Data Logger Example...");
    
    // Print initial statistics
    JsonDocument stats = dataLogger.getStats();
    Serial.println("Initial Status:");
    Serial.printf("Storage Size: %d bytes\n", stats["fileSize"].as<int>());
}

void loop() {
    if (millis() - lastLog >= LOG_INTERVAL) {
        // Read sensor
        SensorData data = readSensor();
        
        // Create JSON document
        JsonDocument doc;
        doc["temp"] = data.temperature;
        doc["hum"] = data.humidity;
        doc["time"] = data.timestamp;
        
        // Log data
        if (dataLogger.push(doc.as<JsonObjectConst>())) {
            Serial.printf("Log #%d: Temp=%.1f°C, Humidity=%.1f%%\n", 
                ++logCount, data.temperature, data.humidity);
        }
        
        // Maintain fixed size (FIFO)
        if (dataLogger.size() > MAX_LOGS) {
            dataLogger.remove(0);  // Remove oldest entry
            Serial.println("Removed oldest entry to maintain size limit");
        }
        
        // Print statistics every 10 logs
        if (logCount % 10 == 0) {
            JsonDocument stats = dataLogger.getStats();
            Serial.println("\nStorage Statistics:");
            Serial.printf("Entries: %d\n", stats["size"].as<int>());
            Serial.printf("File Size: %d bytes\n", stats["fileSize"].as<int>());
            Serial.printf("Fragmentation: %.2f%%\n", 
                stats["fragmentation"].as<float>() * 100);
        }
        
        lastLog = millis();
    }
}