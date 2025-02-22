/**
 * @file Tester.h
 * @brief Provides a class for testing and debugging.
 * @version 1.2
 */
#ifndef TESTER_H
#define TESTER_H

#include <Arduino.h>

class Tester{
private:
    static void printChar(const char c, const uint8_t repeat){
        for (int i = 0; i < repeat; i++) Serial.print(c);
    }
    static void printHeading(const String& heading){
        printChar('_', 20);
        Serial.print(" ");
        Serial.print(heading);
        Serial.print(" ");
        printChar('_', 20);
        Serial.println(); 
    }


    static void pre_test(const String& testName){
        for(uint8_t i=0;i<4;i++){Serial.println();}
        printCenter("Running : "+testName);
        Serial.println();
    }
    static void post_test(const String& testName){
        Serial.println();
        printCenter(("Run: "+testName+ " Done"));
        for(uint8_t i=0;i<4;i++){Serial.println();}
    }

public:
    static void run_test(void(*testFunct)(void),const char* testName,const char* label=""){
        // run_test(testFunct, (String(testName)+" : "+String(label)).c_str());
        // const String test_name_ = testName;
        const String test_name_ = strlen(label)? String(testName)+" : "+String(label): String(testName);

        pre_test(test_name_);
        // Memory_View::printBasicMemoryInfo();
        testFunct();
        // Memory_View::printBasicMemoryInfo();
        post_test(test_name_);
    }
    
    static void run_test(bool(*testFunct)(void),const char* testName,const char* label=""){
        // run_test(testFunct, (String(testName)+" : "+String(label)).c_str());
        // const String test_name_ = testName;
        const String test_name_ = strlen(label)? String(testName)+" : "+String(label): String(testName);

        pre_test(test_name_);
        // Memory_View::printBasicMemoryInfo();
        testFunct();
        // Memory_View::printBasicMemoryInfo();
        post_test(test_name_);
    }

    

    static void printEnvDetails(){
        Serial.println();
        Serial.print("pretty_funct: ");
        Serial.println(__PRETTY_FUNCTION__);
        Serial.print("cpp_version: ");
        Serial.println(__cplusplus);
        Serial.print("compiler_version: ");
        Serial.println(__VERSION__);
        Serial.print("GNUG_version: ");
        Serial.println(__GNUG__); 
        Serial.print("GNUC_version: ");
        Serial.println(__GNUC__);
    }
    
    static void printCenter(const String &a, char padding='=', uint16_t max_width = 80) { //TODO
        uint16_t row=0;
        if (a.length()>max_width) return;
        if(a.length()<max_width){
            row = max_width - a.length();
            row = row/2;
        }else if(a.length()==max_width) {
            row=0;
        }
        Serial.println();
        for (uint16_t i = 0; i < max_width; i++) Serial.print(padding);   
        Serial.println();

        for (uint16_t i = 0; i < row; i++)Serial.print(padding);
        Serial.print(a);
        for (uint16_t i = 0; i < max_width-(row+a.length()); i++)Serial.print(padding);
        Serial.println();
        for (uint16_t i = 0; i < max_width; i++) Serial.print(padding);   
        Serial.println();
    }
   
    template <typename T>
    static void debug_print(const T& message, const String& label ="", uint16_t line_no=0, const char* func = "")  {
        Serial.println();
        if(strlen(func)){
            Serial.print(func);
            Serial.print(":");
        }
        line_no? Serial.print(static_cast<String>(line_no)+":"): Serial.print(" ");
        if(label.length()){
            Serial.print(label);
            Serial.print(":");
        }
        if (std::is_same<T, bool>::value){
            Serial.println(message? " true": " false");
        }else
            Serial.println(message);
        // Serial.flush();
    }


    /**
     * @brief Prints JSON document to Serial with optional metadata
     * @param doc JSON document to print
     * @param a Optional label/message (default: empty string)
     * @param line_no Optional line number (default: 0)
     * @param func Optional function name (default: empty string)
     * 
     * Utility function for debug printing of JSON documents with metadata
     */
    static void printJson(const JsonVariantConst doc, const String& label ="", const uint16_t line_no=0, const char* func = "")  {
        if(!doc.size()) {
            debug_print("!!_!!_!!_JSON_EMPTY_!!_!!_!!", label, line_no, func);
            return;
        }
        Serial.println();
        if(strlen(func)){
            Serial.print(func);
            Serial.print(":");
        }
        line_no? Serial.print(static_cast<String>(line_no)+":"): Serial.print(" ");
        if(label.length()){
            Serial.print(label);
            Serial.print(":");
        }
        serializeJson(doc, Serial);
        Serial.println();
        Serial.flush();
    }

};
    


// #define TEST(func) Tester::run_test(func, #func)
#define GET_PRINT_MACRO(_1,_2,NAME,...) NAME

#define TEST(...)  GET_PRINT_MACRO(__VA_ARGS__, TEST_WITH_LABEL, TEST_WITHOUT_LABEL)(__VA_ARGS__)
#define TEST_WITHOUT_LABEL(func) Tester::run_test(func, #func)
#define TEST_WITH_LABEL(func, label) Tester::run_test(func, #func, label)

#define DEBUG_PRINT(...) GET_PRINT_MACRO(__VA_ARGS__, PRINT_WITH_LABEL, PRINT_NO_LABEL)(__VA_ARGS__)
#define PRINT_WITH_LABEL(message, label) Tester::debug_print(message, label, __LINE__,CLASS_NAME)
#define PRINT_NO_LABEL(message) Tester::debug_print(message, "", __LINE__, CLASS_NAME)

#define PRINT_JSON(...) GET_PRINT_MACRO(__VA_ARGS__, PRINT_JSON_WITH_LABEL, PRINT_JSON_NO_LABEL)(__VA_ARGS__)
#define PRINT_JSON_WITH_LABEL(doc, label) Tester::printJson(doc, label, __LINE__,CLASS_NAME)
#define PRINT_JSON_NO_LABEL(doc) Tester::printJson(doc, "", __LINE__, CLASS_NAME)


#define CLASS_NAME GET_CLASS_NAME(__FILE__)
#define GET_CLASS_NAME(relative_path) []() -> const char* { \
    const char* f = relative_path; \
    const char* lastSlash = strrchr(f, '/'); \
    if (!lastSlash) lastSlash = strrchr(f, '\\'); \
    const char* fileName = lastSlash ? lastSlash + 1 : f; \
    const char* dot = strrchr(fileName, '.'); \
    static char result[256]; \
    size_t len = dot ? dot - fileName : strlen(fileName); \
    strncpy(result, fileName, len); \
    result[len] = '\0'; \
    return result; \
}()

#endif // TESTER_H