#pragma once
#include <string>

enum class Role { ADMIN, LECTURER, HOD, STUDENT };
enum class SubmissionState { SUBMITTED, LATE, MARKED, RETURNED, MODERATED, RELEASED };

inline std::string roleToString(Role r) {
    switch (r) {
        case Role::ADMIN: return "ADMIN";
        case Role::LECTURER: return "LECTURER";
        case Role::HOD: return "HOD";
        case Role::STUDENT: return "STUDENT";
    }
    return "ADMIN";
}
inline Role roleFromString(const std::string& s) {
    if (s == "LECTURER") return Role::LECTURER;
    if (s == "HOD") return Role::HOD;
    if (s == "STUDENT") return Role::STUDENT;
    return Role::ADMIN;
}
inline std::string stateToString(SubmissionState s) {
    switch (s) {
        case SubmissionState::SUBMITTED: return "Submitted";
        case SubmissionState::LATE: return "Late";
        case SubmissionState::MARKED: return "Marked";
        case SubmissionState::RETURNED: return "Returned";
        case SubmissionState::MODERATED: return "Moderated";
        case SubmissionState::RELEASED: return "Released";
    }
    return "Submitted";
}
inline SubmissionState stateFromString(const std::string& s) {
    if (s == "Late") return SubmissionState::LATE;
    if (s == "Marked") return SubmissionState::MARKED;
    if (s == "Returned") return SubmissionState::RETURNED;
    if (s == "Moderated") return SubmissionState::MODERATED;
    if (s == "Released") return SubmissionState::RELEASED;
    return SubmissionState::SUBMITTED;
}