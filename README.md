# Learning Management System (LMS)

C++17 / SQLite console application — CS4433 Final Project.

## Features

- Role-based access for Administrator, Lecturer, Head of Department, Student
- Submission lifecycle: Submitted → Late → Marked → Moderated → Released
- Late penalty: 10% per day for 3 days, zero after 72 hours
- Individual and class-wide extensions
- Full audit log of every mark change (who, when, old, new, reason)
- Offline-first, single-file SQLite storage

## Build

### Prerequisites

- C++17 compiler (GCC ≥ 9, Clang ≥ 7, MSVC ≥ 2019)
- CMake ≥ 3.15
- SQLite3 development libraries

On Windows:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-sqlite3
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
cmake --build build -j
./build/lms

Default accounts (seeded on first run)

User Password Role
admin admin123 Administrator
hod hod123 Head of Department (CS)
lect1 lect123 Lecturer (CS)
lect2 lect123 Lecturer (Math)
stud1 stud123 Student (CS)
stud2 stud123 Student (CS)

Project structure

lms/
├── CMakeLists.txt
├── src/
│   ├── types.h              Enums + string conversions
│   ├── util.h/.cpp          Hashing, ISO-8601 time helpers
│   ├── domain.h/.cpp        User hierarchy, Submission, IPenaltyPolicy
│   ├── data.h/.cpp          SQLite wrapper + repositories
│   ├── services.h/.cpp      Application services (Auth, Admin, Lecturer, Student, Moderation)
│   ├── ui.h/.cpp            Console UI
│   └── main.cpp             Entry point + seed
└── tests/test_main.cpp      GoogleTest-style unit

Design patterns used

Factory — UserFactory creates the right User subclass from a role string

Strategy — IPenaltyPolicy / TenPercentPerDayPolicy

Observer — ISubmissionObserver / AuditLogger

Repository — one interface per aggregate, SQLite implementations behind

**`LICENSE`**

Copyright (c) 2026 Hlalefang Mohapi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
