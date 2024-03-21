#pragma once
#include <iostream>
#include <string>
#include <ctime>

#define NORMAL
#define WARNING
#define FATAL
#define DEBUG

#define Log(level, desc) log(#level, __FILE__, __LINE__, desc)

void log(std::string level, std::string file, int line, std::string desc)
{
    std::cout << "[" << level << "]" << "[" << time(nullptr) << "]" << "[" << file << ": " << line << "]" << ": [" << desc << "]" << std::endl;
}