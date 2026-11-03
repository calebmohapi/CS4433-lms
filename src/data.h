#pragma once
#include "domain.h"
#include <sqlite3.h>
#include <string>
#include <vector>
#include <optional>
#include <stdexcept>

struct DepartmentRow { int id = 0; std::string name; };
struct CourseRow { int id = 0; std::string code, title; int departmentId = 0; int lecturerId = -1; };
struct AssignmentRow { int id = 0; int courseId = 0; std::string title, deadline; double maxMark = 0; };
struct ExtensionRow { int id = 0; int assignmentId = 0; int studentId = 0;
                      std::string newDeadline, reason; int grantedBy = 0; };
struct AuditRow { int id = 0; int submissionId = 0; int changedBy = 0; std::string changedAt;
                  double oldValue = 0; double newValue = 0; std::string reason; };

class Database {
public:
    explicit Database(const std::string& path);
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    void exec(const std::string& sql);
    sqlite3* raw() { return db_; }
    void initializeSchema();

private:
    sqlite3* db_ = nullptr;
};

class Statement {
public:
    Statement(sqlite3* db, const std::string& sql);
    ~Statement();
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;

    void bind(int idx, int v);
    void bind(int idx, double v);
    void bind(int idx, const std::string& v);
    void bindNull(int idx);
    bool step();
    int colInt(int i) const;
    double colDouble(int i) const;
    std::string colText(int i) const;
    bool colIsNull(int i) const;

private:
    sqlite3_stmt* stmt_ = nullptr;
};

class UserRepository {
public:
    explicit UserRepository(Database& db) : db_(db) {}
    std::optional<UserRecord> findByUsername(const std::string& uname);
    std::optional<UserRecord> findById(int id);
    int create(const std::string& uname, const std::string& hash, Role role,
               int deptId, bool active);
    void update(const UserRecord& u);
    std::vector<UserRecord> listAll();
    std::vector<UserRecord> listByDepartment(int deptId);
    std::vector<UserRecord> listByRole(Role r);
private:
    Database& db_;
    UserRecord rowToRecord(Statement& s);
};

class DepartmentRepository {
public:
    explicit DepartmentRepository(Database& db) : db_(db) {}
    int create(const std::string& name);
    std::optional<DepartmentRow> findById(int id);
    std::vector<DepartmentRow> listAll();
private:
    Database& db_;
};

class CourseRepository {
public:
    explicit CourseRepository(Database& db) : db_(db) {}
    int create(const std::string& code, const std::string& title, int deptId, int lecturerId);
    std::optional<CourseRow> findById(int id);
    std::vector<CourseRow> listAll();
    std::vector<CourseRow> listByDepartment(int deptId);
    std::vector<CourseRow> listByLecturer(int lecturerId);
    void assignLecturer(int courseId, int lecturerId);
private:
    Database& db_;
};

class EnrolmentRepository {
public:
    explicit EnrolmentRepository(Database& db) : db_(db) {}
    void enrol(int studentId, int courseId);
    bool isEnrolled(int studentId, int courseId);
    std::vector<CourseRow> listCoursesForStudent(int studentId);
private:
    Database& db_;
};

class AssignmentRepository {
public:
    explicit AssignmentRepository(Database& db) : db_(db) {}
    int create(int courseId, const std::string& title, const std::string& deadline,
               double maxMark);
    std::optional<AssignmentRow> findById(int id);
    std::vector<AssignmentRow> listForCourse(int courseId);
private:
    Database& db_;
};

class ExtensionRepository {
public:
    explicit ExtensionRepository(Database& db) : db_(db) {}
    void upsert(int assignmentId, int studentId, const std::string& newDeadline,
                const std::string& reason, int grantedBy);
    std::optional<ExtensionRow> find(int assignmentId, int studentId);
private:
    Database& db_;
};

class SubmissionRepository {
public:
    explicit SubmissionRepository(Database& db) : db_(db) {}
    int create(const Submission& s);
    void update(const Submission& s);
    std::optional<Submission> findById(int id);
    std::optional<Submission> findByStudentAndAssignment(int studentId, int assignmentId);
    std::vector<Submission> listByAssignment(int assignmentId);
    std::vector<Submission> listByStudent(int studentId);
private:
    Database& db_;
    Submission rowToSubmission(Statement& st);
};

class AuditRepository {
public:
    explicit AuditRepository(Database& db) : db_(db) {}
    void insert(int submissionId, int changedBy, const std::string& changedAt,
                double oldValue, double newValue, const std::string& reason);
    std::vector<AuditRow> listBySubmission(int submissionId);
private:
    Database& db_;
};