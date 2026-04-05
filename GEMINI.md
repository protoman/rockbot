# Project: C++ SDL based game

## General Instructions
*   When generating new code, the target language is **C++11** .
*   Prioritize using modern C++ features and standard library components (e.g., `std::vector`, `std::string`, `std::unique_ptr`).
*   Avoid raw pointers and manual memory management where possible.
*   Ensure all new functions and classes have Doxygen-style comments.
*   The code should compile with `g++` and use `CMake` for the build system.
*   Do not remove 'std::' from the namespace prefix com types.
*   Always build the project to check it is working before and after starting doing changes.
*   Always ask before removing existing functions.
*   This is a multi platform project, so avoid using methods that only work on one operating system or architecture.
*   Do not change the size of arrays used as file.
*   Replace instances of non-safe methods like sprintf, vsprintf, snprintf, vsstrcpy, strcat, strncpy, strncat, strtok,alloca and realpath for memory safe alternatives.
*   When converting sprintf to snprintf, check that the parameters are correct. You might need to add size_t __maxlen.

## Coding Style
*   Use tab for indentation.
*   Class names should use `PascalCase`.
*   Function and variable names should use `snake_case`.

## Project Architecture
*   It uses SDL library for graphics, sound, time and input.


## Example C++ Code Snippet (for context)
```cpp
// include/utils.h
#pragma once

#include <string>

/**
 * @brief Converts a string to uppercase.
 * @param input The input string.
 * @return The uppercase string.
 */
std::string to_uppercase(const std::string& input);