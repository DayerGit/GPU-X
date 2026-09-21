#include "StringManager.h"
#include <Windows.h>


wchar_t* StringManager::_strings = nullptr;
int* StringManager::_arrayOfOffsets = nullptr;

void StringManager::ReadStrings(uint32_t count) { 
    uint32_t totalSize = 0; 
    wchar_t paid = 0; 

    StringManager::_arrayOfOffsets = new int[count]; 
    for (uint32_t i = 0; i < count; i++) { 
        StringManager::_arrayOfOffsets[i] = totalSize; 
        totalSize += LoadStringW(0, 101 + i, &paid, 0); 
        totalSize++; 
    } 
    
    StringManager::_strings = new wchar_t[totalSize]; 
    memset(StringManager::_strings, 0, totalSize * sizeof(wchar_t)); 
    uint32_t currentOffset = 0; 
    for (uint32_t i = 0; i < count; i++) 
        currentOffset += LoadStringW(GetModuleHandleW(nullptr), 101 + i, 
            &StringManager::_strings[currentOffset], totalSize - currentOffset) + 1; 
} 

const wchar_t* StringManager::GetStringByID(int id) { 
    return &StringManager::_strings[StringManager::_arrayOfOffsets[id - 101]]; 
}

void StringManager::Release() {
    delete[] StringManager::_strings;
    delete[] StringManager::_arrayOfOffsets;

    StringManager::_strings = nullptr;
    StringManager::_arrayOfOffsets = nullptr;
}
