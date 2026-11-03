#pragma once
#include "data.h"
#include "util.h"
#include <memory>

class AuditLogger : public ISubmissionObserver
{
public:
    explicit AuditLogger(AuditRepository &repo) : repo_(repo) {}
    void onMarkChanged(int submissionId, int changedBy, double oldValue,
                       double newValue, const std::string &reason) override
    {
        repo_.insert(submissionId, changedBy, util::nowIso(), oldValue, newValue, reason);
    }

private:
    AuditRepository &repo_;
};

class AuthService
{
public:
    explicit AuthService(UserRepository &users) : users_(users) {}
    std::unique_ptr<User> login(const std::string &username, const std::string &password);
    void changePassword(int userId, const std::string &oldPw, const std::string &newPw);

private:
    UserRepository &users_;
};

class AdminService
{
public:
    AdminService(UserRepository &users, DepartmentRepository &depts,
                 CourseRepository &courses, EnrolmentRepository &enrol)
        : users_(users), depts_(depts), courses_(courses), enrol_(enrol) {}
    int createUser(const std::string &uname, const std::string &password,
                   Role role, int deptId);
    void updateUser(int id, const std::string &uname, Role role, int deptId, bool active);
    void deactivateUser(int id);
    int createDepartment(const std::string &name);
    int createCourse(const std::string &code, const std::string &title, int deptId);
    void assignLecturer(int courseId, int lecturerId);
    void enrolStudent(int studentId, int courseId);
    std::vector<UserRecord> listUsers() { return users_.listAll(); }
    std::vector<DepartmentRow> listDepartments() { return depts_.listAll(); }
    std::vector<CourseRow> listCourses() { return courses_.listAll(); }

private:
    UserRepository &users_;
    DepartmentRepository &depts_;
    CourseRepository &courses_;
    EnrolmentRepository &enrol_;
};

class LecturerService
{
public:
    LecturerService(AssignmentRepository &assigns, SubmissionRepository &subs,
                    ExtensionRepository &exts, CourseRepository &courses,
                    UserRepository &users, AuditRepository &audits)
        : assigns_(assigns), subs_(subs), exts_(exts), courses_(courses),
          users_(users), audits_(audits) {}
    int createAssignment(int lecturerId, int courseId, const std::string &title,
                         const std::string &deadlineIso, double maxMark);
    std::vector<CourseRow> listMyCourses(int lecturerId) { return courses_.listByLecturer(lecturerId); }
    std::vector<AssignmentRow> listAssignments(int lecturerId);
    std::vector<AssignmentRow> listAssignmentsForCourse(int courseId)
    {
        return assigns_.listForCourse(courseId);
    }
    void grantExtension(int lecturerId, int assignmentId, int studentId,
                        const std::string &newDeadline, const std::string &reason);
    double markSubmission(int lecturerId, int submissionId, double mark,
                          const std::string &feedback);
    std::vector<Submission> listSubmissions(int assignmentId)
    {
        return subs_.listByAssignment(assignmentId);
    }

private:
    AssignmentRepository &assigns_;
    SubmissionRepository &subs_;
    ExtensionRepository &exts_;
    CourseRepository &courses_;
    UserRepository &users_;
    AuditRepository &audits_;
};

class StudentService
{
public:
    StudentService(SubmissionRepository &subs, AssignmentRepository &assigns,
                   ExtensionRepository &exts, EnrolmentRepository &enrol,
                   CourseRepository &courses)
        : subs_(subs), assigns_(assigns), exts_(exts), enrol_(enrol), courses_(courses) {}
    void submit(int studentId, int assignmentId, const std::string &content);
    std::vector<CourseRow> myCourses(int studentId) { return enrol_.listCoursesForStudent(studentId); }
    std::vector<AssignmentRow> myAssignments(int studentId, int courseId)
    {
        if (!enrol_.isEnrolled(studentId, courseId))
            return {};
        return assigns_.listForCourse(courseId);
    }
    std::optional<Submission> mySubmission(int studentId, int assignmentId)
    {
        return subs_.findByStudentAndAssignment(studentId, assignmentId);
    }
    std::vector<Submission> myReleasedSubmissions(int studentId);

private:
    SubmissionRepository &subs_;
    AssignmentRepository &assigns_;
    ExtensionRepository &exts_;
    EnrolmentRepository &enrol_;
    CourseRepository &courses_;
};

class ModerationService
{
public:
    ModerationService(SubmissionRepository &subs, AssignmentRepository &assigns,
                      CourseRepository &courses, UserRepository &users,
                      AuditRepository &audits)
        : subs_(subs), assigns_(assigns), courses_(courses), users_(users), audits_(audits) {}
    std::vector<Submission> listMarkedInDept(int hodId);
    std::vector<Submission> listModeratedInDept(int hodId);
    void approveMark(int hodId, int submissionId);
    void adjustMark(int hodId, int submissionId, double newMark, const std::string &reason);
    void returnToLecturer(int hodId, int submissionId, const std::string &comment);
    void releaseMark(int hodId, int submissionId);

private:
    bool hodCanAccess(int hodId, int submissionId);
    SubmissionRepository &subs_;
    AssignmentRepository &assigns_;
    CourseRepository &courses_;
    UserRepository &users_;
    AuditRepository &audits_;
};