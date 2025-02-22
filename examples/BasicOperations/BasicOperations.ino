#include <Arduino.h>
#include <MemoryList.h>
#include <ArduinoJson.h>

MemoryList list("/data.txt");

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(100);
    
    if (!SD.begin()) {
        Serial.println("SD Card initialization failed!");
        return;
    }

    // Clear any existing data
    list.clear();
    Serial.println("Starting Basic Operations Example...");

    // 1. Push some data
    JsonDocument doc;
    doc["id"] = 1;
    doc["data"] = "test entry";
    Serial.println("\nPushing first entry...");
    if (list.push(doc.as<JsonObjectConst>())) {
        Serial.println("Push successful!");
    }

    // 2. Get size
    Serial.printf("Current size: %d\n", list.size());

    // 3. Get first element
    Serial.println("\nFirst element: " + list.get(0));

    // 4. Add more elements
    doc["id"] = 2;
    doc["data"] = "second entry";
    list.push(doc.as<JsonObjectConst>());

    // 5. Get last element
    Serial.println("\nLast element: " + list.getLast());

    // 6. Remove first element
    Serial.println("\nRemoving first element...");
    list.remove(0);
    Serial.printf("Size after remove: %d\n", list.size());

    // 7. Get statistics
    JsonDocument stats = list.getStats();
    Serial.println("\nList Statistics:");
    Serial.printf("Size: %d\n", stats["size"].as<int>());
    Serial.printf("File Size: %d bytes\n", stats["fileSize"].as<int>());
    Serial.printf("Fragmentation: %.2f%%\n", stats["fragmentation"].as<float>() * 100);
}

void loop() {
    delay(1000);
}