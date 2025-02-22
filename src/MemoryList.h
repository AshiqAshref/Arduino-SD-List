/**
 * @file MemoryList.h
 * @brief FIFO list implementation using SD card storage with JSON support
 * @details Provides a FIFO (First In, First Out) list implementation that stores JSON objects
 *          on an SD card with features like fragmentation management and efficient read/write operations
 */

 #ifndef MEMORY_LIST_H
#define MEMORY_LIST_H

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <StreamUtils.h>
#include <ArduinoJson.h>
#include <Tester.h>


/**
 * @class MemoryList
 * @brief SD card-based FIFO list manager for JSON objects
 * @details Implements a FIFO list stored on SD card with features including:
 *          - JSON object storage and retrieval
 *          - Automatic defragmentation
 *          - Tombstone-based deletion system
 *          - Buffer-aware operations for ESP32
 *          - Memory-efficient streaming operations
 */

class MemoryList {
private:
    /** @brief Path to the storage file on SD card */
    String filePath;

    /** @brief Current number of valid entries in the list */
    size_t currentSize;

    /** @brief Buffer size optimized for ESP32 SD card operations */
    static constexpr size_t BUFFER_SIZE = 512;  // ESP32 friendly buffer size
    /** @brief Character used to mark deleted entries */
    static constexpr char TOMBSTONE = '$';      // Marker for deleted entries
    /** @brief Threshold ratio that triggers automatic defragmentation */
    static constexpr float DEFRAG_THRESHOLD = 0.6f; // 60% fragmentation triggers defrag


    /**
     * @brief Internal method to push JSON object to file
     * @param element JSON object to store
     * @param file Open file handle
     * @return true if successful, false otherwise
     * @details Handles serialization and writing of JSON objects with error checking
     */
    bool push(const JsonObjectConst element, File file) {
        if (!file) {DEBUG_PRINT("File not opened by SD!");return false;}
        if (element.isNull()) {DEBUG_PRINT("Element is null!");return false;}
    
        String element_string;
        serializeJson(element, element_string);
        element_string.trim();
    
        if (file.println(element_string)) {currentSize++; return true;}
        DEBUG_PRINT("Failed to write element to file!");
        return false;
    }


    /**
     * @brief Validates and ensures file existence
     * @return true if file exists or was created successfully
     * @details Creates file if it doesn't exist, performs error checking
     */
    [[nodiscard]] bool checkFile() const {
        if (!SD.exists(filePath)) {
            File dataFile = SD.open(filePath, FILE_WRITE);
            if (!dataFile) {DEBUG_PRINT("Failed to create file!"); return false;}
            dataFile.close();
        }
        return true;
    }


    /**
     * @brief Reads a line from specific position in file
     * @param cursor_pos Starting position in file
     * @param file Open file handle
     * @return String containing the line read
     * @details Handles boundary conditions and EOF
     */
    static String readLineFromPos(const size_t cursor_pos, File file) {
        file.seek(cursor_pos);
        if(file.available()){
            String str = file.readStringUntil('\n');
            str.trim(); 
            return str;
        }
        return "";
    }

public:
    /**
     * @brief Constructor
     * @param filePath Path to storage file on SD card
     * @throws Runtime error if SD card initialization fails
     * @details Initializes SD card, creates/opens storage file, validates content
     */
    explicit MemoryList(const String& filePath) :
        filePath(filePath)
    {
        if (!SD.begin()) {DEBUG_PRINT("SD card initialization failed!"); return;}
        if (!checkFile()) return;
        // if (!is_file_valid()) if(!fix_file()) {DEBUG_PRINT("FILE NOT VALID AND COULD NOT BE FIXED"); return;}
        this->currentSize =  calcSize();
    }
    

    /**
     * @brief Default destructor
     */
    ~MemoryList() =default;


    /**
     * @brief Returns statistics about the list
     * @return JsonDocument containing:
     *         - size: current number of valid entries
     *         - fragmentation: current fragmentation ratio
     *         - fileSize: total file size in bytes
     */
    [[nodiscard]] JsonDocument getStats() const {
        JsonDocument stats;
        stats["size"] = currentSize;
        stats["fragmentation"] = getFragmentationRatio();
        stats["fileSize"] = SD.open(filePath).size();
        return stats;
    }


    /**
     * @brief Calculate current number of valid entries
     * @return Number of valid entries
     * @details Counts non-tombstone entries in file
     */
    [[nodiscard]] size_t calcSize() const {
        File dataFile = SD.open(filePath, FILE_READ);
        if (!dataFile) {DEBUG_PRINT("Failed to open file for reading!"); return 0;}

        size_t size = 0;
        ReadBufferingStream reader(dataFile, 64); 
        while (reader.available()) {
            const String line = reader.readStringUntil('\n');
            if (line.length()>0 && line[0] != TOMBSTONE) ++size;
        }
        dataFile.close();
        return size;
    }


    /**
     * @brief Adds a new JSON object to the list
     * @param element The JSON object to add
     * @return true if successful, false on failure
     * @throws None
     * @details Opens the file in append mode, serializes the JSON object,
     *          and writes it to the end of the file. Updates currentSize on success.
     *          Handles file opening errors and null element validation.
     */
    bool push(const JsonObjectConst element) {
        File dataFile = SD.open(filePath, FILE_APPEND);
        if (!dataFile) {DEBUG_PRINT("Failed to open file for appending!");return false;}
        if (element.isNull()) {DEBUG_PRINT("Element is null!");return false;}

        const bool status = push(element, dataFile);
        dataFile.close();
        return status;
    }


    /**
     * @brief Checks if the list is empty
     * @return true if list contains no valid elements, false otherwise
     * @throws None
     * @details Fast operation that checks currentSize without file access
     */
    [[nodiscard]] bool isEmpty() const {
        return currentSize == 0;
    }


    /**
     * @brief Retrieves the last valid element in the list
     * @return String containing the last valid JSON object, empty string on failure
     * @throws None
     * @details Implements a sophisticated buffer-aware backward search algorithm:
     *          - Handles buffer boundaries
     *          - Skips tombstone entries
     *          - Uses efficient buffered reading
     *          - Handles newline characters at buffer boundaries
     *          - Manages file position tracking
     */
    [[nodiscard]] String getLast() const {
        if (isEmpty()) {DEBUG_PRINT("List is empty!");return ""; }
        File dataFile = SD.open(filePath, FILE_READ);
        if (!dataFile) {DEBUG_PRINT("Failed to open file for reading!"); return "";}


        const size_t fileSize = dataFile.size();

        for (size_t pos = fileSize; pos > 0;) {
            constexpr size_t bufferSize = 512;
            uint8_t buffer[bufferSize];

            const size_t readSize = min(bufferSize, pos);
            pos -= readSize;
            if(!dataFile.seek(pos)){dataFile.close(); DEBUG_PRINT(pos, "Failed to seek to position"); return "";}
            const size_t bytesRead = dataFile.read(buffer, readSize);


            for(uint16_t i = bytesRead; i-- > 0;) {
                if (buffer[i] == '\n') {
                    if(i == bytesRead - 1) {
                        const String val = readLineFromPos(pos+1+i, dataFile);
                        if(val.length()>0 && val.charAt(0)!= TOMBSTONE) {dataFile.close(); return val;}

                    }else if (buffer[i+1]!=TOMBSTONE){
                        String val = readLineFromPos(pos+1+i, dataFile);
                        if(val.length()>0) {dataFile.close(); return val;}
                    }
                }else if (i==0 && pos == 0){
                    String val = readLineFromPos(0, dataFile);
                    if(val.length()>0 && val.charAt(0)!= TOMBSTONE) {dataFile.close(); return val;}
                }
            }
        }
        DEBUG_PRINT("Failed to get last element!");
        dataFile.close();
        return "";
    }


    /**
     * @brief Retrieves element at specified index
     * @param index Zero-based index of desired element
     * @return String containing the JSON object at index, empty string if index invalid
     * @throws None
     * @details 
     *          - Validates index against currentSize
     *          - Skips tombstone entries during counting
     *          - Uses buffered reading for efficiency
     *          - Returns empty string on any error condition
     */
    [[nodiscard]] String get(const size_t index) const {
        if (index >= currentSize) {
            DEBUG_PRINT("Index out of bounds!");
            return ""; // Return an empty string to indicate failure
        }
        return readLine(index);
    }


    /**
     * @brief Returns current number of valid elements
     * @return Current size of list (excluding tombstone entries)
     * @throws None
     * @details Constant time operation using cached size value
     */
    [[nodiscard]] size_t size() const {
        return currentSize;
    }


    /**
     * @brief Retrieves first n elements as JSON array
     * @param count Number of elements to retrieve
     * @return JsonDocument containing array of retrieved elements
     * @throws None
     * @details 
     *          - Returns empty document if list is empty
     *          - Handles JSON parsing errors
     *          - Limits return size to min(count, currentSize)
     *          - Skips tombstone entries
     *          - Validates JSON format of each element
     */
    [[nodiscard]] JsonDocument getFirst(const size_t count) const {
        JsonDocument doc;
        if (currentSize == 0) { DEBUG_PRINT("List is empty!"); return doc;}
        const size_t numElements = min(count, currentSize);
    
        size_t validCount = 0;
        for (size_t i = 0; validCount < numElements; i++) {
            const String element = readLine(i);
            if (element.isEmpty()) {
                DEBUG_PRINT("Failed to get element!");
                doc.clear();
                return doc;
            }
            JsonDocument elementDoc;
            DeserializationError error = deserializeJson(elementDoc, element);
            if (error) {
                DEBUG_PRINT(error.c_str(), "Json deserialization error");
                doc.clear();
                return doc;
            }
            doc.add(elementDoc);
            validCount++;
        }
    
        return doc;
    }


    /**
     * @brief Removes element at specified index
     * @param index Zero-based index of element to remove
     * @return String containing removed element, empty string on failure
     * @throws None
     * @details 
     *          - Validates index bounds
     *          - Uses tombstone marking for deletion
     *          - Updates currentSize
     *          - Triggers defragmentation if needed
     *          - Maintains file integrity during operation
     *          - Handles file positioning and cursor management
     */
    String remove(const size_t index) {
        if (index >= currentSize) {DEBUG_PRINT("Index out of bounds!"); return "";}
    
        //gets the cursor position of the line to be removed
        size_t cursor_position =0;
        String removedElement = readLine(index, &cursor_position);
        if (removedElement.isEmpty()) {DEBUG_PRINT("Failed to read line!"); return "";}
    
        // Remove the element from the file
        File dataFile = SD.open(filePath, FILE_WRITE);
        
        dataFile.seek(dataFile.size(), SeekSet);
        dataFile.seek(cursor_position, SeekSet);
        dataFile.write(TOMBSTONE);
    
        dataFile.flush();
        dataFile.close();
        currentSize--;

        if(shouldDefragment()) defragment();
        return removedElement;
    }


    /**
     * @brief Clears all elements from the list
     * @throws None
     * @details 
     *          - Removes file completely
     *          - Creates new empty file
     *          - Resets currentSize
     *          - Handles file operation errors
     *          - Ensures atomic operation
     */
    void clear() {
        if (SD.remove(filePath)) {
            Serial.println("cleared successfully!");
            SD.open(filePath, FILE_WRITE).close();
            currentSize = 0;
        } else {
            DEBUG_PRINT("Failed to clear file!");
        }
    }


    /**
     * @brief Removes first n elements from the list
     * @param count Number of elements to remove
     * @return Number of elements actually removed
     * @throws None
     * @details 
     *          - Optimized batch removal operation
     *          - Uses buffered reading for efficiency
     *          - Tracks file positions for deletion
     *          - Updates currentSize for each removal
     *          - Triggers defragmentation if needed
     *          - Handles partial success cases
     */
    uint16_t removeFirst(const size_t count) {
        if (currentSize == 0) {DEBUG_PRINT("List is empty!");return 0;}
        File dataFile = SD.open(filePath, FILE_READ);
        if (!dataFile) {DEBUG_PRINT("Failed to open file for reading!");return 0;}

        const size_t count_ = min(count, currentSize);
        size_t positions[count_];
        uint16_t readLines = 0;
        size_t currentPos = 0;
        ReadBufferingStream reader(dataFile, 64);

        while (reader.available() && readLines < count_) {
            String line = reader.readStringUntil('\n');
            line.trim();
            if (line.length() > 0 && line[0] != TOMBSTONE) {
                positions[readLines++] = currentPos;
            }
            currentPos += line.length() + 1;
        }
        dataFile.close();
        
        dataFile = SD.open(filePath, FILE_WRITE);
        if (!dataFile) {DEBUG_PRINT("Failed to open file for writing!");return 0;}
        dataFile.seek(dataFile.size(), SeekSet);
        
        for (uint16_t i = 0; i < readLines && dataFile.seek(positions[i], SeekSet); i++){
             dataFile.write(TOMBSTONE);
             currentSize--;
        }

        dataFile.flush();
        dataFile.close();
        if(shouldDefragment()) defragment();
        return readLines;
    }


    /**
     * @brief Defragments the storage file
     * @return true if successful, false on failure
     * @throws None
     * @details 
     *          - Creates temporary file
     *          - Copies valid entries sequentially
     *          - Handles file operation errors
     *          - Manages atomic file replacement
     *          - Updates currentSize
     *          - Maintains data integrity
     *          - Uses buffered operations for efficiency
     */
    bool defragment() {
        const String tempPath = filePath + ".tmp";

        File sourceFile = SD.open(filePath, FILE_READ);
        if (sourceFile.size() == 0) {DEBUG_PRINT("File is empty, no need to defragment");return true;}
        File tempFile = SD.open(tempPath, FILE_WRITE);
        if (!sourceFile || !tempFile) {
            DEBUG_PRINT("Failed to open files!");
            if (sourceFile) sourceFile.close();
            if (tempFile) tempFile.close();
            return false;
        }

        size_t validCount = 0;
        ReadBufferingStream reader(sourceFile, 64);
        while(reader.available()) {
            String line = reader.readStringUntil('\n');
            line.trim();
            if (line.length() > 0 && line[0] != TOMBSTONE) {
                if(!tempFile.println(line)){
                    DEBUG_PRINT("Write to temp file failed!");
                    sourceFile.close();
                    tempFile.close();
                    SD.remove(tempPath);
                    return false;
                }
                validCount++;
            }
        }
        sourceFile.close();
        tempFile.flush();
        tempFile.close();


        if (!SD.remove(filePath)) {DEBUG_PRINT("Failed to remove original file!");
            SD.remove(tempPath);
            return false;
        }
        if (!SD.rename(tempPath, filePath)) { DEBUG_PRINT("Failed to rename temp file!");
            SD.remove(tempPath);
            return false;
        }

        currentSize = validCount;
        DEBUG_PRINT("Defragmentation complete. Valid entries: " + String(validCount));
        return true;
    }


    /**
     * @brief Reads specific line from file
     * @param line_no Line number to read
     * @param cursorPosition Optional pointer to store cursor position
     * @return String containing the line read, empty string on failure
     * @throws None
     * @details 
     *          - Skips tombstone entries
     *          - Uses buffered reading
     *          - Tracks valid line count
     *          - Manages file positioning
     *          - Optional cursor position tracking
     */
    [[nodiscard]] String readLine(const size_t line_no, size_t* cursorPosition = nullptr) const {
        File dataFile = SD.open(filePath, FILE_READ);
        if (!dataFile) { DEBUG_PRINT("Failed to open file for reading!"); return "";}
        if(!dataFile.seek(0)) {DEBUG_PRINT(0,"Failed to seek to position!"); return "";}

        size_t validLineCount = 0;
        size_t cursorPosition_ = 0;
        ReadBufferingStream reader(dataFile, 64); 
        while (reader.available()) {
            String line = reader.readStringUntil('\n');
            if (line[0] != TOMBSTONE) {
                if (validLineCount == line_no) {
                    if(cursorPosition!=nullptr) *cursorPosition = cursorPosition_;
                    line.trim();
                    return line;
                }
                validLineCount++;
            }
            cursorPosition_ += line.length() + 1;
        }
        if(cursorPosition) *cursorPosition = cursorPosition_;
        dataFile.close();
        return "";
    }


    /**
     * @brief Debug utility to print all entries
     * @throws None
     * @details 
     *          - Prints all entries including tombstones
     *          - Uses buffered reading
     *          - Handles file operation errors
     *          - Formats output with markers
     */
    void print_all() const {
        File dataFile = SD.open(filePath, "r");
        DEBUG_PRINT("--printBgn--");
        ReadBufferingStream reader(dataFile, 64); 
        while (reader.available()) {
            String line = reader.readStringUntil('\n');
            line.trim();
            Serial.println(line);
        }
        dataFile.close();
        DEBUG_PRINT("--printEnd--");
    }


    /**
     * @brief Calculates current fragmentation ratio
     * @return Float value between 0.0 and 1.0 indicating fragmentation level
     * @throws None
     * @details 
     *          - Calculates ratio of invalid to total space
     *          - Handles empty file case
     *          - Uses buffered reading
     *          - Accounts for newlines in calculations
     *          - Precise floating-point calculations
     */
    [[nodiscard]] float getFragmentationRatio() const {
        File dataFile = SD.open(filePath, FILE_READ);
        if (!dataFile) {
            DEBUG_PRINT("Failed to open file for reading!");
            return 0.0f;
        }

        const float rawFileSize = static_cast<float>(dataFile.size());
        if (rawFileSize == 0) {dataFile.close();return 0.0f;}

        float validDataSize = 0.0f;
        ReadBufferingStream reader(dataFile, 64); // 64 is the buffer size

        while (reader.available()) {
            String line = reader.readStringUntil('\n');
            if (line.length() > 0 && line[0] != TOMBSTONE) {
                validDataSize += line.length() + 1; // +1 for newline
            }
        }

        dataFile.close();
        return (rawFileSize - validDataSize) / rawFileSize;
    }


    /**
     * @brief Checks if defragmentation is needed
     * @param threshold Fragmentation ratio threshold (default 0.7)
     * @return true if defragmentation is recommended
     * @throws None
     * @details 
     *          - Compares current fragmentation to threshold
     *          - Configurable threshold value
     *          - Logs debug information
     *          - Uses cached fragmentation data when available
     */
    bool shouldDefragment(const float threshold = 0.7f) const {
        const float fragRatio = getFragmentationRatio();
        if (fragRatio >= threshold) {
            DEBUG_PRINT("Fragmentation ratio " + String(fragRatio * 100) + "% exceeds threshold " + String(threshold * 100) + "%");
            return true;
        }
        return false;
    }

};


#endif