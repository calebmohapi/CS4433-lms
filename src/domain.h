#pragma once
#include "types.h"
#include <string>
#include <vector>
#include <memory>

struct UserRecord {
    int id = 0;
    std::string username;
    std::string passwordHash;
    Role role = Role::STUDENT;
    int departmentId = -1;
    bool active = true;
};

class User {
public:
    int id = 0;
    std::string username;
    std::string passwordHash;
    Role role = Role::STUDENT;
    int departmentId = -1;
    bool active = true;
    virtual ~User() = default;
    virtual std::string roleName() const = 0;
};

class Administrator : public User { public: std::string roleName() const override { return "ADMIN"; } };
class LecturerUser : public User { public: std::string roleName() const override { return "LECTURER"; } };
class HeadOfDepartmentUser : public User { public: std::string roleName() const override { return "HOD"; } };
class StudentUser : public User { public: std::string roleName() const override { return "STUDENT"; } };

class UserFactory {
public:
    static std::unique_ptr<User> create(const UserRecord& rec);
};

class IPenaltyPolicy {
public:
    virtual ~IPenaltyPolicy() = default;
    virtual double compute(double maxMark, int daysLate) const = 0;
};

class TenPercentPerDayPolicy : public IPenaltyPolicy {
public:
    double compute(double maxMark, int daysLate) const override;
};

class ISubmissionObserver {
public:
    virtual ~ISubmissionObserver() = default;
    virtual void onMarkChanged(int submissionId, int changedBy,
                               double oldValue, double newValue,
                               const std::string& reason) = 0;
};

struct Submission {
    int id = 0;
    int assignmentId = 0;
    int studentId = 0;
    SubmissionState state = SubmissionState::SUBMITTED;
    std::string submittedAt;
    int daysLate = 0;
    std::string content;
    double mark = 0;
    bool hasMark = false;
    std::string feedback;

    void registerObserver(ISubmissionObserver* obs) { observers_.push_back(obs); }
    void clearObservers() { observers_.clear(); }

    bool canTransitionTo(SubmissionState to) const;
    bool transitionTo(SubmissionState to);
    void recordMarkChange(int changedBy, double oldValue, double newValue,
                          const std::string& reason);

private:
    std::vector<ISubmissionObserver*> observers_;
};