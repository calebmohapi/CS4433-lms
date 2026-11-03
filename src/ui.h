#pragma once
#include "services.h"

class ConsoleUI {
public:
    ConsoleUI(AuthService& auth, AdminService& admin,
              LecturerService& lecturer, StudentService& student,
              ModerationService& mod, UserRepository& users,
              DepartmentRepository& depts, CourseRepository& courses,
              AssignmentRepository& assigns, SubmissionRepository& subs,
              ExtensionRepository& exts, AuditRepository& audits);
    void run();

private:
    void adminMenu(User& u);
    void lecturerMenu(User& u);
    void studentMenu(User& u);
    void hodMenu(User& u);
    void changePasswordMenu(User& u);

    AuthService& auth_;
    AdminService& admin_;
    LecturerService& lecturer_;
    StudentService& student_;
    ModerationService& mod_;
    UserRepository& users_;
    DepartmentRepository& depts_;
    CourseRepository& courses_;
    AssignmentRepository& assigns_;
    SubmissionRepository& subs_;
    ExtensionRepository& exts_;
    AuditRepository& audits_;
};