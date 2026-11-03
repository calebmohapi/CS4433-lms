#pragma once
#include <string>
#include <cstdint>

namespace util {

std::string sha256(const std::string& input);
std::string hashPassword(const std::string& password);
bool verifyPassword(const std::string& password, const std::string& stored);

std::string nowIso();
int64_t parseIsoToEpoch(const std::string& iso);
std::string epochToIso(int64_t epoch);
std::string addHours(const std::string& iso, int hours);
int64_t nowEpoch();

}