#include "domain.h"
#include <stdexcept>
#include <algorithm>

std::unique_ptr<User> UserFactory::create(const UserRecord& rec) {
    std::unique_ptr<User> u;
    switch (rec.role) {
        case Role::ADMIN:    u = std::make_unique<Administrator>(); break;
        case Role::LECTURER: u = std::make_unique<LecturerUser>(); break;
        case Role::HOD:      u = std::make_unique<HeadOfDepartmentUser>(); break;
        case Role::STUDENT:  u = std::make_unique<StudentUser>(); break;
    }
    u->id = rec.id;
    u->username = rec.username;
    u->passwordHash = rec.passwordHash;
    u->role = rec.role;
    u->departmentId = rec.departmentId;
    u->active = rec.active;
    return u;
}

double TenPercentPerDayPolicy::compute(double maxMark, int daysLate) const {
    if (daysLate < 0) return 0.0;
    if (daysLate > 3) return maxMark; // 100% penalty (>72h late)
    return maxMark * 0.10 * daysLate;
}

bool Submission::canTransitionTo(SubmissionState to) const {
    switch (state) {
        case SubmissionState::SUBMITTED:
            return to == SubmissionState::SUBMITTED ||
                   to == SubmissionState::LATE ||
                   to == SubmissionState::MARKED;
        case SubmissionState::LATE:
            return to == SubmissionState::MARKED;
        case SubmissionState::MARKED:
            return to == SubmissionState::MODERATED ||
                   to == SubmissionState::RETURNED;
        case SubmissionState::RETURNED:
            return to == SubmissionState::MARKED;
        case SubmissionState::MODERATED:
            return to == SubmissionState::RELEASED;
        case SubmissionState::RELEASED:
            return false;
    }
    return false;
}

bool Submission::transitionTo(SubmissionState to) {
    if (!canTransitionTo(to)) return false;
    state = to;
    return true;
}

void Submission::recordMarkChange(int changedBy, double oldValue, double newValue,
                                   const std::string& reason) {
    for (auto* obs : observers_) {
        obs->onMarkChanged(id, changedBy, oldValue, newValue, reason);
    }
}