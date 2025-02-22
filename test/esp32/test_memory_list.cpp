#include "test_memory_list.h"

#include <Tester.h>
#include <WiFi.h>
#include "MemoryList.h"

const int buttonPin = 13;
volatile bool buttonPressed = false;

// Interrupt Service Routine (ISR)
void IRAM_ATTR buttonInterruptHandler() {
    buttonPressed = true;
}

const String test_file = "/test_file.txt";
MemoryList memoryList = MemoryList(test_file);


// Helper function to check and report test result
bool checkTest(bool condition, const String& testName, const String& expected = "", const String& got = "") {
    if(!condition) {
        if(expected.length() > 0) {
            DEBUG_PRINT("TEST_FAIL", testName + " - Expected: " + expected + ", got: " + got);
        } else {
            DEBUG_PRINT("TEST_FAIL", testName);
        }
    } else {
        DEBUG_PRINT("test_pass", testName);
    }
    return condition;
}


// Helper function to compare two JsonDocuments
bool compareJsonDocuments(const JsonDocument& doc1, const JsonDocument& doc2) {
    String str1, str2;
    serializeJson(doc1, str1);
    serializeJson(doc2, str2);
    return str1 == str2;
}



bool checkTestResults(bool* test_stats, int test_size) {
    int pass_count = 0;
    for(int i = 0; i < test_size; i++) {
        if(test_stats[i]) pass_count++;
    }
    if(pass_count == test_size) {
        DEBUG_PRINT("all pass");
        return true;
    }
    DEBUG_PRINT("TEST FAIL", "passed: " + String(pass_count) + "/" + String(test_size));
    return false;
}



bool testClear() {
    MemoryList testList("/test_clear.txt");
    
    const int test_size = 2;
    bool test_stats[test_size] = {false};
    int test_index = 0;

    // Setup test data
    JsonDocument doc;
    doc["test"] = "test_data";
    testList.push(doc.as<JsonObjectConst>());

    // Test 1: Clear list
    testList.clear();
    test_stats[test_index++] = (testList.size() == 0);

    // Test 2: Verify file exists but is empty
    test_stats[test_index++] = (SD.exists("/test_clear.txt") && 
                               SD.open("/test_clear.txt", FILE_READ).size() == 0);

    return checkTestResults(test_stats, test_size);
}



bool testPush() {
    MemoryList testList("/test_push.txt");
    testList.clear();  // Start fresh
    
    const int test_size = 3;
    bool test_stats[test_size] = {false};
    int test_index = 0;

    // Test 1: Push single element
    JsonDocument doc1;
    doc1["test"] = "first";
    test_stats[test_index++] = testList.push(doc1.as<JsonObjectConst>());
    
    // Test 2: Verify size after push
    test_stats[test_index++] = (testList.size() == 1);
    
    // Test 3: Verify content
    String expected;
    serializeJson(doc1, expected);
    test_stats[test_index++] = (testList.get(0) == expected);

    return checkTestResults(test_stats, test_size);
}



bool testGet() {
    MemoryList testList("/test_get.txt");
    testList.clear();
    
    const int test_size = 4;
    bool test_stats[test_size] = {false};
    int test_index = 0;

    // Setup test data
    JsonDocument docs[3];
    for(int i = 0; i < 3; i++) {
        docs[i]["test"] = "item" + String(i);
        testList.push(docs[i].as<JsonObjectConst>());
    }

    // Test 1: Get middle element
    String expected;
    serializeJson(docs[1], expected);
    test_stats[test_index++] = (testList.get(1) == expected);

    // Test 2: Get out of bounds
    test_stats[test_index++] = (testList.get(testList.size()) == "");

    // Test 3: Get after remove
    testList.remove(1);
    serializeJson(docs[2], expected);
    test_stats[test_index++] = (testList.get(1) == expected);

    // Test 4: Get from empty list
    testList.clear();
    test_stats[test_index++] = (testList.get(0) == "");

    return checkTestResults(test_stats, test_size);
}



bool testGetFirst() {
    MemoryList testList("/test_get_first.txt");
    testList.clear();

    const int test_size = 3;
    bool test_stats[test_size] = {false};
    int test_index = 0;

    // Setup test data
    JsonDocument docs[3];
    for(int i = 0; i < 3; i++) {
        docs[i]["test"] = "item" + String(i);
        testList.push(docs[i].as<JsonObjectConst>());
    }

    // Test 1: Get first 2 elements
    JsonDocument firstTwo = testList.getFirst(2);
    test_stats[test_index++] = (firstTwo.size() == 2);

    // Test 2: Verify content of first element
    String expected;
    serializeJson(docs[0], expected);
    test_stats[test_index++] = (firstTwo[0].as<String>() == expected);

    // Test 3: Request more elements than exist
    JsonDocument allElements = testList.getFirst(5);
    test_stats[test_index++] = (allElements.size() == 3);

    return checkTestResults(test_stats, test_size);
}



bool testRemoveFirst() {
    MemoryList testList("/test_remove_first.txt");
    testList.clear();

    const int test_size = 3;
    bool test_stats[test_size] = {false};
    int test_index = 0;

    // Setup test data
    for(int i = 0; i < 5; i++) {
        JsonDocument doc;
        doc["test"] = "item" + String(i);
        testList.push(doc.as<JsonObjectConst>());
    }

    // Test 1: Remove first 2 elements
    test_stats[test_index++] = (testList.removeFirst(2) == 2);

    // Test 2: Verify size after remove
    test_stats[test_index++] = (testList.size() == 3);

    // Test 3: Verify remaining elements
    JsonDocument expectedDoc;
    expectedDoc["test"] = "item2";
    String expected;
    serializeJson(expectedDoc, expected);
    test_stats[test_index++] = (testList.get(0) == expected);

    return checkTestResults(test_stats, test_size);
}



bool testRemove() {
    MemoryList testList("/test_remove.txt");
    testList.clear();

    const int test_size = 3;
    bool test_stats[test_size] = {false};
    int test_index = 0;

    // Setup test data
    JsonDocument doc1, doc2, doc3;
    doc1["test"] = "first";
    doc2["test"] = "second";
    doc3["test"] = "third";
    
    testList.push(doc1.as<JsonObjectConst>());
    testList.push(doc2.as<JsonObjectConst>());
    testList.push(doc3.as<JsonObjectConst>());

    // Test 1: Remove middle element
    String expected;
    serializeJson(doc2, expected);
    String removed = testList.remove(1);
    test_stats[test_index++] = (removed == expected);

    // Test 2: Verify size after remove
    test_stats[test_index++] = (testList.size() == 2);

    // Test 3: Verify remaining elements
    String firstExpected, lastExpected;
    serializeJson(doc1, firstExpected);
    serializeJson(doc3, lastExpected);
    test_stats[test_index++] = (testList.get(0) == firstExpected && 
                               testList.get(1) == lastExpected);

    return checkTestResults(test_stats, test_size);
}



bool testTombstoning() {
    MemoryList testList("/test_tombstone.txt");
    testList.clear();
    
    const int test_size = 3;
    bool test_stats[test_size] = {false};
    int test_index = 0;

    // Setup test data
    for(int i = 1; i <= 5; i++) {
        JsonDocument doc;
        doc["test"] = "item" + String(i);
        testList.push(doc.as<JsonObjectConst>());
    }
    DEBUG_PRINT("Before tombstone");
    testList.print_all();

    // Test 1: Verify tombstone marking
    testList.remove(2);  // Remove middle element
    DEBUG_PRINT("After tombstone");
    testList.print_all();
    File dataFile = SD.open("/test_tombstone.txt", FILE_READ);
    if (dataFile) {
        dataFile.seek(0);
        ReadBufferingStream reader(dataFile, 64);
        int count = 0;
        while(reader.available() && count < 3) {
            String line = reader.readStringUntil('\n');
            if(count == 2) {
                test_stats[test_index++] = (line[0] == '$');
                break;
            }
            count++;
        }
        dataFile.close();
    }

    // Test 2: Verify data integrity after tombstoning
    JsonDocument expectedDoc;
    expectedDoc["test"] = "item3";
    String expected;
    serializeJson(expectedDoc, expected);
    test_stats[test_index++] = (testList.get(2) == expected);

    // Test 3: Verify size management
    test_stats[test_index++] = (testList.size() == 4);

    return checkTestResults(test_stats, test_size);
}



bool testDefragmentation() {
    MemoryList testList("/test_defrag.txt");
    testList.clear();
    
    const int test_size = 4;
    bool test_stats[test_size] = {false};
    int test_index = 0;

    // Setup test data with deletions
    for(int i = 0; i < 6; i++) {
        JsonDocument doc;
        doc["test"] = "item" + String(i);
        testList.push(doc.as<JsonObjectConst>());
    }
    testList.remove(1);
    testList.remove(3);
    DEBUG_PRINT("Before defrag");
    testList.print_all();

    // Test 1: Check fragmentation ratio before defrag
    const float fragRatio = testList.getFragmentationRatio();
    test_stats[test_index++] = (fragRatio > 0.0f);
    if(!test_stats[test_index - 1]) {DEBUG_PRINT("Fragmentation ratio fail: " ,"Expected:>0 result: "+ String(fragRatio));}

    // Test 2: Perform defragmentation
    test_stats[test_index++] = testList.defragment();
    if(!test_stats[test_index - 1]) {DEBUG_PRINT("Defragmentation fail");}

    DEBUG_PRINT("After defrag");
    testList.print_all();


    // Test 3: Verify data integrity after defrag
    JsonDocument expectedDoc;
    expectedDoc["test"] = "item4";
    String expected = expectedDoc.as<String>();
    test_stats[test_index++] = (testList.get(2) == expected);

    // Test 4: Verify fragmentation ratio after defrag
    test_stats[test_index++] = (testList.getFragmentationRatio() < fragRatio);

    return checkTestResults(test_stats, test_size);
}



bool testEmptyOperations() {
    MemoryList testList("/test_empty_ops.txt");
    testList.clear();
    
    const int test_size = 5;
    bool test_stats[test_size] = {false};
    int test_index = 0;

    // Test 1: Size of empty list
    test_stats[test_index++] = (testList.size() == 0);

    // Test 2: Remove from empty list
    test_stats[test_index++] = (testList.remove(0) == "");

    // Test 3: RemoveFirst from empty list
    test_stats[test_index++] = (testList.removeFirst(1) == 0);

    // Test 4: GetFirst from empty list
    JsonDocument emptyDoc = testList.getFirst(1);
    test_stats[test_index++] = (emptyDoc.size() == 0);

    // Test 5: Defragment empty list
    test_stats[test_index++] = testList.defragment();

    return checkTestResults(test_stats, test_size);
}







// Independent test cases for getLast()
bool testGetLast_EmptyList() {
    MemoryList testList("/test_empty.txt");
    return checkTest(testList.getLast().isEmpty(), "Empty list should return empty string");
}



bool testGetLast_SingleElement() {
    MemoryList testList("/test_single.txt");
    JsonDocument doc;
    doc["test"] = "single";
    testList.push(doc.as<JsonObjectConst>());
    String expected;
    serializeJson(doc, expected);
    return checkTest(testList.getLast() == expected, "Single element", expected, testList.getLast());
}



bool testGetLast_MultipleElements() {
    MemoryList testList("/test_multiple.txt");
    JsonDocument doc1, doc2, doc3;
    doc1["test"] = "first";
    doc2["test"] = "second";
    doc3["test"] = "last";
    
    testList.push(doc1.as<JsonObjectConst>());
    testList.push(doc2.as<JsonObjectConst>());
    testList.push(doc3.as<JsonObjectConst>());
    
    String expected;
    serializeJson(doc3, expected);
    return checkTest(testList.getLast() == expected, "Multiple elements", expected, testList.getLast());
}



bool testGetLast_AfterRemove() {
    MemoryList testList("/test_after_remove.txt");
    JsonDocument doc1, doc2, doc3;
    doc1["test"] = "first";
    doc2["test"] = "second";
    doc3["test"] = "third";
    
    testList.push(doc1.as<JsonObjectConst>());
    testList.push(doc2.as<JsonObjectConst>());
    testList.push(doc3.as<JsonObjectConst>());

    testList.remove(testList.size() - 1);
    
    String expected = doc2.as<String>();
    String result = testList.getLast();
    return checkTest(result == expected, "After remove", expected, result);
}



bool testGetLast_LargeFile() {
    MemoryList testList("/test_large.txt");
    JsonDocument lastDoc;
    
    for(int i = 0; i < 10; i++) {
        JsonDocument docLarge;
        docLarge["test"] = "item" + String(i);
        testList.push(docLarge.as<JsonObjectConst>());
        if(i == 9) lastDoc = docLarge;
    }
    
    String expected;
    serializeJson(lastDoc, expected);
    return checkTest(testList.getLast() == expected, "Large file", expected, testList.getLast());
}



bool testGetLast_MultipleTombstones() {
    MemoryList testList("/test_tombstones.txt");
    JsonDocument expectedDoc;
    
    for(int i = 0; i < 10; i++) {
        JsonDocument doc;
        doc["test"] = "item" + String(i);
        testList.push(doc.as<JsonObjectConst>());
        if(i == 4) expectedDoc = doc;
    }
    
    // Remove last 5 elements
    for(int i = 0; i < 5; i++) {
        testList.remove(testList.size() - 1);
    }
    
    String expected;
    serializeJson(expectedDoc, expected);
    return checkTest(testList.getLast() == expected, "Multiple tombstones", expected, testList.getLast());
}



bool testGetLast_BufferBoundaryNewline() {
    MemoryList testList("/test_buffer_boundary.txt");
    String longString(510, 'a');
    longString += "\n";
    
    JsonDocument doc1, doc2;
    doc1["test"] = longString;
    doc2["test"] = "final";
    
    testList.push(doc1.as<JsonObjectConst>());
    testList.push(doc2.as<JsonObjectConst>());
    
    String expected;
    serializeJson(doc2, expected);
    return checkTest(testList.getLast() == expected, "Buffer boundary newline", expected, testList.getLast());
}



bool testGetLast_SplitAcrossBuffers() {
    MemoryList testList("/test_split_buffers.txt");
    String longString(511, 'a');
    
    JsonDocument doc1, doc2;
    doc1["test"] = longString;
    doc2["test"] = "split";
    
    testList.push(doc1.as<JsonObjectConst>());
    testList.push(doc2.as<JsonObjectConst>());
    
    String expected;
    serializeJson(doc2, expected);
    return checkTest(testList.getLast() == expected, "Split across buffers", expected, testList.getLast());
}



bool testGetLast_MultipleNewlinesAtBoundary() {
    MemoryList testList("/test_multiple_newlines.txt");
    String longString(509, 'a');
    longString += "\n\n";
    
    JsonDocument doc1, doc2;
    doc1["test"] = longString;
    doc2["test"] = "after_multiple_newlines";
    
    testList.push(doc1.as<JsonObjectConst>());
    testList.push(doc2.as<JsonObjectConst>());
    
    String expected;
    serializeJson(doc2, expected);
    return checkTest(testList.getLast() == expected, "Multiple newlines at boundary", expected, testList.getLast());
}



bool testGetLast_ExactBufferBoundary() {
    MemoryList testList("/test_exact_boundary.txt");
    testList.clear();
    
    // Create a string that ends exactly at buffer boundary (511 chars + newline)
    String boundaryString(511, 'x');
    JsonDocument doc1, doc2;
    doc1["test"] = boundaryString;
    doc2["test"] = "final";
    
    testList.push(doc1.as<JsonObjectConst>());
    testList.push(doc2.as<JsonObjectConst>());
    
    String expected;
    serializeJson(doc2, expected);
    return checkTest(testList.getLast() == expected, "Exact buffer boundary", expected, testList.getLast());
}

bool testGetLast_TombstoneAtBufferBoundary() {
    MemoryList testList("/test_tombstone_boundary.txt");
    testList.clear();
    
    // Create entries where tombstone falls at buffer boundary
    String boundaryString(511, 'x');
    JsonDocument doc1, doc2, doc3;
    doc1["test"] = boundaryString;
    doc2["test"] = "to_be_removed";
    doc3["test"] = "final";
    
    testList.push(doc1.as<JsonObjectConst>());
    testList.push(doc2.as<JsonObjectConst>());
    testList.push(doc3.as<JsonObjectConst>());
    
    // Remove middle entry to create tombstone
    testList.remove(1);
    
    String expected;
    serializeJson(doc3, expected);
    return checkTest(testList.getLast() == expected, "Tombstone at buffer boundary", expected, testList.getLast());
}

void testMemoryList() {
    Serial.println("Starting MemoryList tests...");

    // // Basic operations
    // TEST(testPush, "Test push operation");
    // TEST(testGet, "Test get operation");
    // TEST(testRemove, "Test remove operation");
    // TEST(testRemoveFirst, "Test removeFirst operation");
    // TEST(testClear, "Test clear operation");
    // TEST(testGetFirst, "Test getFirst operation");
    // TEST(testEmptyOperations, "Test operations on empty list");

    // // getLast tests
    // TEST(testGetLast_EmptyList, "Test getLast with empty list");
    // TEST(testGetLast_SingleElement, "Test getLast with single element");
    // TEST(testGetLast_MultipleElements, "Test getLast with multiple elements");
    // TEST(testGetLast_AfterRemove, "Test getLast after remove");
    // TEST(testGetLast_LargeFile, "Test getLast with large file");
    // TEST(testGetLast_MultipleTombstones, "Test getLast with multiple tombstones");
    // TEST(testGetLast_BufferBoundaryNewline, "Test getLast with buffer boundary newline");
    // TEST(testGetLast_SplitAcrossBuffers, "Test getLast split across buffers");
    // TEST(testGetLast_MultipleNewlinesAtBoundary, "Test getLast with multiple newlines at boundary");

    TEST(testGetLast_ExactBufferBoundary, "Test getLast with exact buffer boundary");
    TEST(testGetLast_TombstoneAtBufferBoundary, "Test getLast with tombstone at buffer boundary");
    // // Advanced features
    // TEST(testTombstoning, "Test tombstone functionality");
    // TEST(testDefragmentation, "Test defragmentation process");

    Serial.println("MemoryList tests complete.");
}

void onPress() {
    // Memory::sd_print_all_files(test_file);
}

void setup() {
    setUp();
    TEST(testMemoryList);
    teardown();
}

void loop() {
    if (buttonPressed) {onPress(); buttonPressed = false;}
}

void setUp() {
    DEBUG_PRINT(SD.begin() ? "true" : "false", "init SD");
    initializeEnvirnoment();
    attachInterrupt(digitalPinToInterrupt(buttonPin), buttonInterruptHandler, FALLING); // Attach interrupt
    Tester::printEnvDetails();
    for (uint8_t i = 0; i < 4; i++) {
        Serial.println();
    }
}
void teardown() {
    for (uint8_t i = 0; i < 4; i++) {
        Serial.println();
    }
}

void press_to_start() {
    Serial.println("Press_to_start");
    Serial.println();
    Serial.println();
    while (Serial.available() == 0) {
    }
    while (Serial.available() > 0) {
        Serial.read();
    }
}

void initializeEnvirnoment() {
    Serial.begin(115200);
    Serial.print("\x1b[20h");
}




