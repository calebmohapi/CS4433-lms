# Moderated Learning Management System (LMS)

**Version:** 1.1 · **Course:** CS4433 Final Project · **License:** MIT

A console-based, offline-first Learning Management System for a secondary school. Handles users, courses, assignments, submissions, marking, moderation, and release, with a complete audit trail of every mark change.

## Status

![CI](https://github.com/YOUR_USERNAME/lms-cs4433/actions/workflows/ci.yml/badge.svg)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)](https://en.cppreference.com/w/cpp/17)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

## Features

- **Role-based access** — Administrator, Lecturer, Head of Department, Student
- **Submission lifecycle** — Submitted → Late → Marked → Moderated → Released (FR15)
- **Late penalty** — 10% per day up to 3 days, zero after 72 hours (FR9)
- **Extensions** — individual (FR6) and class-wide (FR16, CR1), both with mandatory reason
- **Audit log** — every mark change recorded: who, when, old, new, reason (FR14)
- **Offline-first** — single-file SQLite database, no network (NFR2)
- **Secure** — salted iterated SHA-256; parameterised queries only (NFR3)
- **Robust** — invalid input rejected, no crashes (NFR4)
- **Tested** — ≥ 70% line coverage of business-logic layers (NFR5)
- **Durable** — data survives process kill and restart (NFR6)

## Requirements

### Runtime

- Windows 10/11 (x64) or any modern Linux (glibc ≥ 2.28)
- 50 MB disk space, no network connection required

### Build tools

- C++17 compiler: GCC ≥ 9 / Clang ≥ 7 / MSVC ≥ 2019
- CMake ≥ 3.15
- SQLite3 development headers and libraries

## Building

### Ubuntu

Open the MSYS2 UCRT64 terminal

```bash
# One-time dependency install
pacman -S --needed \
  mingw-w64-ucrt-x86_64-gcc \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-make \
  mingw-w64-ucrt-x86_64-sqlite3

# Build
cd /c/Users/YOU/Documents/lms
rm -rf build && mkdir build && cd build

cmake .. -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTS=ON \
  -DSQLite3_INCLUDE_DIR=/ucrt64/include \
  -DSQLite3_LIBRARY=/ucrt64/lib/libsqlite3.dll.a

cmake --build . -j"$(nproc)"

#Run
cd build
./lms
