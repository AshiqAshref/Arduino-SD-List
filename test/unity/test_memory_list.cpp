#include <unity.h>
#include <Arduino.h>
#include "MemoryList.h"
#include <ArduinoJson.h>

MemoryList* testList;

void setUp(void) {
    SD.begin();
    testList = new MemoryList("/test.txt");
    testList->clear();
}



void tearDown(void) {
    delete testList;
}

// Basic Operations Tests
void test_push_should_add_element(void) {
    JsonDocument doc;
    doc["test"] = "first";
    
    TEST_ASSERT_TRUE(testList->push(doc.as<JsonObjectConst>()));
    TEST_ASSERT_EQUAL(1, testList->size());
    
    String expected;
    serializeJson(doc, expected);
    TEST_ASSERT_EQUAL_STRING(expected.c_str(), testList->get(0).c_str());
}

void test_get_should_return_element_at_index(void) {
    JsonDocument docs[3];
    for(int i = 0; i < 3; i++) {
        docs[i]["test"] = "item" + String(i);
        testList->push(docs[i].as<JsonObjectConst>());
    }

    String expected;
    serializeJson(docs[1], expected);
    TEST_ASSERT_EQUAL_STRING(expected.c_str(), testList->get(1).c_str());
}

void test_remove_should_delete_element(void) {
    JsonDocument doc1, doc2, doc3;
    doc1["test"] = "first";
    doc2["test"] = "second";
    doc3["test"] = "third";
    
    testList->push(doc1.as<JsonObjectConst>());
    testList->push(doc2.as<JsonObjectConst>());
    testList->push(doc3.as<JsonObjectConst>());

    String expected;
    serializeJson(doc2, expected);
    TEST_ASSERT_EQUAL_STRING(expected.c_str(), testList->remove(1).c_str());
    TEST_ASSERT_EQUAL(2, testList->size());
}

// getLast Tests
void test_getLast_empty_list_should_return_empty_string(void) {
    TEST_ASSERT_TRUE(testList->getLast().isEmpty());
}

void test_getLast_with_single_element(void) {
    JsonDocument doc;
    doc["test"] = "single";
    testList->push(doc.as<JsonObjectConst>());
    
    String expected;
    serializeJson(doc, expected);
    TEST_ASSERT_EQUAL_STRING(expected.c_str(), testList->getLast().c_str());
}

void test_getLast_with_buffer_boundary(void) {
    String boundaryString(511, 'x');
    JsonDocument doc1, doc2;
    doc1["test"] = boundaryString;
    doc2["test"] = "final";
    
    testList->push(doc1.as<JsonObjectConst>());
    testList->push(doc2.as<JsonObjectConst>());
    
    String expected;
    serializeJson(doc2, expected);
    TEST_ASSERT_EQUAL_STRING(expected.c_str(), testList->getLast().c_str());
}

void test_getLast_with_tombstone_at_boundary(void) {
    String boundaryString(511, 'x');
    JsonDocument doc1, doc2, doc3;
    doc1["test"] = boundaryString;
    doc2["test"] = "to_be_removed";
    doc3["test"] = "final";
    
    testList->push(doc1.as<JsonObjectConst>());
    testList->push(doc2.as<JsonObjectConst>());
    testList->push(doc3.as<JsonObjectConst>());
    
    testList->remove(1);
    
    String expected;
    serializeJson(doc3, expected);
    TEST_ASSERT_EQUAL_STRING(expected.c_str(), testList->getLast().c_str());
}

// Fragmentation Tests
void test_defragmentation_should_reduce_fragmentation(void) {
    for(int i = 0; i < 6; i++) {
        JsonDocument doc;
        doc["test"] = "item" + String(i);
        testList->push(doc.as<JsonObjectConst>());
    }
    testList->remove(1);
    testList->remove(3);
    
    float fragBefore = testList->getFragmentationRatio();
    TEST_ASSERT_TRUE(testList->defragment());
    float fragAfter = testList->getFragmentationRatio();
    
    TEST_ASSERT_TRUE(fragAfter < fragBefore);
}


// Add these test functions:

void test_getLast_split_across_buffers(void) {
    String longString(511, 'a');
    
    JsonDocument doc1, doc2;
    doc1["test"] = longString;
    doc2["test"] = "split";
    
    testList->push(doc1.as<JsonObjectConst>());
    testList->push(doc2.as<JsonObjectConst>());
    
    String expected;
    serializeJson(doc2, expected);
    TEST_ASSERT_EQUAL_STRING(expected.c_str(), testList->getLast().c_str());
}

void test_getLast_multiple_newlines_at_boundary(void) {
    String longString(509, 'a');
    longString += "\n\n";
    
    JsonDocument doc1, doc2;
    doc1["test"] = longString;
    doc2["test"] = "after_multiple_newlines";
    
    testList->push(doc1.as<JsonObjectConst>());
    testList->push(doc2.as<JsonObjectConst>());
    
    String expected;
    serializeJson(doc2, expected);
    TEST_ASSERT_EQUAL_STRING(expected.c_str(), testList->getLast().c_str());
}

void test_getLast_multiple_tombstones(void) {
    JsonDocument expectedDoc;
    
    for(int i = 0; i < 10; i++) {
        JsonDocument doc;
        doc["test"] = "item" + String(i);
        testList->push(doc.as<JsonObjectConst>());
        if(i == 4) expectedDoc = doc;
    }
    
    // Remove last 5 elements
    for(int i = 0; i < 5; i++) {
        testList->remove(testList->size() - 1);
    }
    
    String expected;
    serializeJson(expectedDoc, expected);
    TEST_ASSERT_EQUAL_STRING(expected.c_str(), testList->getLast().c_str());
}

void test_large_file_operations(void) {
    JsonDocument lastDoc;
    
    for(int i = 0; i < 10; i++) {
        JsonDocument doc;
        doc["test"] = "item" + String(i);
        testList->push(doc.as<JsonObjectConst>());
        if(i == 9) lastDoc = doc;
    }
    
    String expected;
    serializeJson(lastDoc, expected);
    TEST_ASSERT_EQUAL_STRING(expected.c_str(), testList->getLast().c_str());
    TEST_ASSERT_EQUAL(10, testList->size());
}

void RUN_UNITY_TESTS() {
    UNITY_BEGIN();
    
    // Basic Operations
    RUN_TEST(test_push_should_add_element);
    RUN_TEST(test_get_should_return_element_at_index);
    RUN_TEST(test_remove_should_delete_element);
    
    // getLast Tests
    RUN_TEST(test_getLast_empty_list_should_return_empty_string);
    RUN_TEST(test_getLast_with_single_element);
    RUN_TEST(test_getLast_with_buffer_boundary);
    RUN_TEST(test_getLast_with_tombstone_at_boundary);
    
    // Fragmentation Tests
    RUN_TEST(test_defragmentation_should_reduce_fragmentation);

    RUN_TEST(test_getLast_split_across_buffers);
    RUN_TEST(test_getLast_multiple_newlines_at_boundary);
    RUN_TEST(test_getLast_multiple_tombstones);
    RUN_TEST(test_large_file_operations);
    
    UNITY_END();
}

void setup() {
    delay(2000);
    RUN_UNITY_TESTS();
}

void loop() {}