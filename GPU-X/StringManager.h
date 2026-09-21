#pragma once
#include <stdint.h>


class StringManager {
public:
    StringManager() = delete;
    ~StringManager() = delete;

    static void ReadStrings(uint32_t count);
    static const wchar_t* GetStringByID(int id);
    static void Release();

private:
    static wchar_t* _strings;
    static int* _arrayOfOffsets;
};
