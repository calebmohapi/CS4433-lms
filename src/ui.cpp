#include "ui.h"
#include <iostream>
#include <string>
#include <limits>

static std::string prompt(const std::string& label) {
    std::cout << label;
    std::string s;
    std::getline(std::cin, s);
    return s;
}
static int promptInt(const std::string& label) {
    while (true) {
        std::string s = prompt(label);
        try { return std::stoi(s); }
        catch (...) { std::cout << "  Invalid number, try again.\n"; }
    }
}
static double promptDouble(const std::string& label) {
    while (true) {
        std::string s = prompt(label);
        try { return std::stod(s); }
        catch (...) { std::cout << "  Invalid number, try again.\n"; }
    }
}

ConsoleUI::ConsoleUI(AuthService& auth, AdminService& admin,
                     LecturerService& lecturer, StudentService& student,
                     ModerationService& mod, UserRepository& users,
                     DepartmentRepository& depts, CourseRepository& courses,
                     AssignmentRepository& assigns, SubmissionRepository& subs,
                     ExtensionRepository& exts, AuditRepository& audits)
    : auth_(auth), admin_(admin), lecturer_(lecturer), student_(student),
      mod_(mod), users_(users), depts_(depts), courses_(courses),
      assigns_(assigns), subs_(subs), exts_(exts), audits_(audits) {}

void ConsoleUI::run() {
    std::cout << "=====================================================\n";
    std::cout << "  Learning Management System  (LMS)\n";
    std::cout << "=====================================================\n";
    while (true) {
        std::string uname = prompt("Username (blank to exit): ");
        if (uname.empty()) break;
        std::string pw = prompt("Password: ");
        try {
            auto user = auth_.login(uname, pw);
            if (!user) { std::cout << "  Login failed.\n"; continue; }
            std::cout << "  Logged in as " << user->username
                      << " [" << user->roleName() << "]\n";
            switch (user->role) {
                case Role::ADMIN:    adminMenu(*user);    break;
                case Role::LECTURER: lecturerMenu(*user); break;
                case Role::STUDENT:  studentMenu(*user);  break;
                case Role::HOD:      hodMenu(*user);      break;
            }
        } catch (const std::exception& e) {
            std::cout << "  Error: " << e.what() << "\n";
        }
    }
    std::cout << "Goodbye.\n";
}

void ConsoleUI::changePasswordMenu(User& u) {
    try {
        std::string oldp = prompt("  Current password: ");
        std::string newp = prompt("  New password (min 6): ");
        auth_.changePassword(u.id, oldp, newp);
        std::cout << "  Password changed.\n";
    } catch (const std::exception& e) {
        std::cout << "  Error: " << e.what() << "\n";
    }
}

// ---------------- ADMIN ----------------
void ConsoleUI::adminMenu(User& u) {
    while (true) {
        std::cout << "\n--- Administrator Menu ---\n"
                  << "1. List users\n"
                  << "2. Create user\n"
                  << "3. Deactivate user\n"
                  << "4. List departments\n"
                  << "5. Create department\n"
                  << "6. List courses\n"
                  << "7. Create course\n"
                  << "8. Assign lecturer to course\n"
                  << "9. Enrol student in course\n"
                  << "10. Change password\n"
                  << "0. Logout\n";
        int c = promptInt("Choice: ");
        try {
            if (c == 0) return;
            else if (c == 1) {
                for (auto& r : admin_.listUsers())
                    std::cout << "  #" << r.id << " " << r.username
                              << " [" << roleToString(r.role) << "]"
                              << " dept=" << r.departmentId
                              << (r.active ? "" : " (inactive)") << "\n";
            } else if (c == 2) {
                std::string uname = prompt("  Username: ");
                std::string pw = prompt("  Password: ");
                std::cout << "  Role (1=Admin, 2=Lecturer, 3=HoD, 4=Student): ";
                int r = promptInt("  > ");
                Role role = Role::STUDENT;
                if (r == 1) role = Role::ADMIN;
                else if (r == 2) role = Role::LECTURER;
                else if (r == 3) role = Role::HOD;
                int deptId = -1;
                if (role != Role::ADMIN && role != Role::STUDENT) {
                    deptId = promptInt("  Department id: ");
                } else if (role == Role::STUDENT) {
                    std::string s = prompt("  Department id (blank for none): ");
                    if (!s.empty()) deptId = std::stoi(s);
                }
                int newId = admin_.createUser(uname, pw, role, deptId);
                std::cout << "  Created user #" << newId << "\n";
            } else if (c == 3) {
                int id = promptInt("  User id: ");
                admin_.deactivateUser(id);
                std::cout << "  Deactivated.\n";
            } else if (c == 4) {
                for (auto& d : admin_.listDepartments())
                    std::cout << "  #" << d.id << " " << d.name << "\n";
            } else if (c == 5) {
                std::string name = prompt("  Department name: ");
                int id = admin_.createDepartment(name);
                std::cout << "  Created department #" << id << "\n";
            } else if (c == 6) {
                for (auto& co : admin_.listCourses())
                    std::cout << "  #" << co.id << " " << co.code << " - "
                              << co.title << " (dept " << co.departmentId
                              << ", lecturer " << co.lecturerId << ")\n";
            } else if (c == 7) {
                std::string code = prompt("  Course code: ");
                std::string title = prompt("  Course title: ");
                int dept = promptInt("  Department id: ");
                int id = admin_.createCourse(code, title, dept);
                std::cout << "  Created course #" << id << "\n";
            } else if (c == 8) {
                int cid = promptInt("  Course id: ");
                int lid = promptInt("  Lecturer user id: ");
                admin_.assignLecturer(cid, lid);
                std::cout << "  Assigned.\n";
            } else if (c == 9) {
                int sid = promptInt("  Student user id: ");
                int cid = promptInt("  Course id: ");
                admin_.enrolStudent(sid, cid);
                std::cout << "  Enrolled.\n";
            } else if (c == 10) {
                changePasswordMenu(u);
            } else {
                std::cout << "  Unknown choice.\n";
            }
        } catch (const std::exception& e) {
            std::cout << "  Error: " << e.what() << "\n";
        }
    }
}

// ---------------- LECTURER ----------------
void ConsoleUI::lecturerMenu(User& u) {
    while (true) {
        std::cout << "\n--- Lecturer Menu ---\n"
                  << "1. My courses\n"
                  << "2. List assignments\n"
                  << "3. Create assignment\n"
                  << "4. List submissions for an assignment\n"
                  << "5. Mark submission\n"
                  << "6. Grant extension\n"
                  << "7. Change password\n"
                  << "0. Logout\n";
        int c = promptInt("Choice: ");
        try {
            if (c == 0) return;
            else if (c == 1) {
                for (auto& co : lecturer_.listMyCourses(u.id))
                    std::cout << "  #" << co.id << " " << co.code << " - " << co.title << "\n";
            } else if (c == 2) {
                for (auto& a : lecturer_.listAssignments(u.id))
                    std::cout << "  #" << a.id << " [course " << a.courseId << "] "
                              << a.title << " (max " << a.maxMark
                              << ", deadline " << a.deadline << ")\n";
            } else if (c == 3) {
                int cid = promptInt("  Course id: ");
                std::string title = prompt("  Title: ");
                std::string deadline = prompt("  Deadline (YYYY-MM-DD HH:MM:SS): ");
                double mm = promptDouble("  Max mark: ");
                int id = lecturer_.createAssignment(u.id, cid, title, deadline, mm);
                std::cout << "  Created assignment #" << id << "\n";
            } else if (c == 4) {
                int aid = promptInt("  Assignment id: ");
                auto list = lecturer_.listSubmissions(aid);
                if (list.empty()) std::cout << "  (none)\n";
                for (auto& s : list) {
                    std::cout << "  sub#" << s.id << " student " << s.studentId
                              << " state=" << stateToString(s.state)
                              << " daysLate=" << s.daysLate;
                    if (s.hasMark) std::cout << " mark=" << s.mark;
                    std::cout << "\n";
                }
            } else if (c == 5) {
                int sid = promptInt("  Submission id: ");
                double m = promptDouble("  Mark: ");
                std::string fb = prompt("  Feedback: ");
                double finalMark = lecturer_.markSubmission(u.id, sid, m, fb);
                std::cout << "  Marked. Final (after penalty) = " << finalMark << "\n";
            } else if (c == 6) {
                int aid = promptInt("  Assignment id: ");
                int sid = promptInt("  Student user id: ");
                std::string nd = prompt("  New deadline (YYYY-MM-DD HH:MM:SS): ");
                std::string reason = prompt("  Reason: ");
                lecturer_.grantExtension(u.id, aid, sid, nd, reason);
                std::cout << "  Extension granted.\n";
            } else if (c == 7) {
                changePasswordMenu(u);
            } else std::cout << "  Unknown choice.\n";
        } catch (const std::exception& e) {
            std::cout << "  Error: " << e.what() << "\n";
        }
    }
}

// ---------------- STUDENT ----------------
void ConsoleUI::studentMenu(User& u) {
    while (true) {
        std::cout << "\n--- Student Menu ---\n"
                  << "1. My courses\n"
                  << "2. Assignments in a course\n"
                  << "3. Submit / resubmit\n"
                  << "4. View submission for an assignment\n"
                  << "5. View released marks\n"
                  << "6. Change password\n"
                  << "0. Logout\n";
        int c = promptInt("Choice: ");
        try {
            if (c == 0) return;
            else if (c == 1) {
                for (auto& co : student_.myCourses(u.id))
                    std::cout << "  #" << co.id << " " << co.code << " - " << co.title << "\n";
            } else if (c == 2) {
                int cid = promptInt("  Course id: ");
                for (auto& a : student_.myAssignments(u.id, cid))
                    std::cout << "  #" << a.id << " " << a.title
                              << " deadline=" << a.deadline << " max=" << a.maxMark << "\n";
            } else if (c == 3) {
                int aid = promptInt("  Assignment id: ");
                std::string content = prompt("  Content: ");
                student_.submit(u.id, aid, content);
                std::cout << "  Submitted.\n";
            } else if (c == 4) {
                int aid = promptInt("  Assignment id: ");
                auto s = student_.mySubmission(u.id, aid);
                if (!s) std::cout << "  No submission.\n";
                else {
                    std::cout << "  State: " << stateToString(s->state)
                              << "\n  Submitted at: " << s->submittedAt
                              << "\n  Content: " << s->content << "\n";
                    if (s->state == SubmissionState::RELEASED) {
                        std::cout << "  Mark: " << s->mark
                                  << "\n  Feedback: " << s->feedback << "\n";
                    }
                }
            } else if (c == 5) {
                for (auto& s : student_.myReleasedSubmissions(u.id))
                    std::cout << "  sub#" << s.id << " assignment " << s.assignmentId
                              << " mark=" << s.mark << " feedback=" << s.feedback << "\n";
            } else if (c == 6) {
                changePasswordMenu(u);
            } else std::cout << "  Unknown choice.\n";
        } catch (const std::exception& e) {
            std::cout << "  Error: " << e.what() << "\n";
        }
    }
}

// ---------------- HOD ----------------
void ConsoleUI::hodMenu(User& u) {
    while (true) {
        std::cout << "\n--- Head of Department Menu ---\n"
                  << "1. List MARKED submissions in my dept\n"
                  << "2. Approve mark (unchanged)\n"
                  << "3. Adjust mark\n"
                  << "4. Return to lecturer\n"
                  << "5. List MODERATED submissions\n"
                  << "6. Release mark\n"
                  << "7. View audit log for submission\n"
                  << "8. Change password\n"
                  << "0. Logout\n";
        int c = promptInt("Choice: ");
        try {
            if (c == 0) return;
            else if (c == 1) {
                for (auto& s : mod_.listMarkedInDept(u.id))
                    std::cout << "  sub#" << s.id << " assignment " << s.assignmentId
                              << " student " << s.studentId
                              << " mark=" << s.mark << "\n";
            } else if (c == 2) {
                int sid = promptInt("  Submission id: ");
                mod_.approveMark(u.id, sid);
                std::cout << "  Approved (state=Moderated).\n";
            } else if (c == 3) {
                int sid = promptInt("  Submission id: ");
                double m = promptDouble("  New mark: ");
                std::string r = prompt("  Reason: ");
                mod_.adjustMark(u.id, sid, m, r);
                std::cout << "  Adjusted.\n";
            } else if (c == 4) {
                int sid = promptInt("  Submission id: ");
                std::string cm = prompt("  Comment: ");
                mod_.returnToLecturer(u.id, sid, cm);
                std::cout << "  Returned to lecturer.\n";
            } else if (c == 5) {
                for (auto& s : mod_.listModeratedInDept(u.id))
                    std::cout << "  sub#" << s.id << " mark=" << s.mark << "\n";
            } else if (c == 6) {
                int sid = promptInt("  Submission id: ");
                mod_.releaseMark(u.id, sid);
                std::cout << "  Released (visible to student).\n";
            } else if (c == 7) {
                int sid = promptInt("  Submission id: ");
                for (auto& a : audits_.listBySubmission(sid))
                    std::cout << "  [" << a.changedAt << "] by #" << a.changedBy
                              << " old=" << a.oldValue << " new=" << a.newValue
                              << " reason=" << a.reason << "\n";
            } else if (c == 8) {
                changePasswordMenu(u);
            } else std::cout << "  Unknown choice.\n";
        } catch (const std::exception& e) {
            std::cout << "  Error: " << e.what() << "\n";
        }
    }
}