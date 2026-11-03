#include "data.h"
#include <cstring>

static void throwSqlite(sqlite3* db, const std::string& ctx) {
    throw std::runtime_error(ctx + ": " + sqlite3_errmsg(db));
}

Database::Database(const std::string& path) {
    int rc = sqlite3_open(path.c_str(), &db_);
    if (rc != SQLITE_OK) throw std::runtime_error("Cannot open db: " + path);
    exec("PRAGMA foreign_keys = ON;");
}

Database::~Database() { if (db_) sqlite3_close(db_); }

void Database::exec(const std::string& sql) {
    char* err = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err ? err : "unknown";
        sqlite3_free(err);
        throw std::runtime_error("SQL error: " + msg + "\nSQL: " + sql);
    }
}

void Database::initializeSchema() {
    exec(R"SQL(
CREATE TABLE IF NOT EXISTS department(
  id INTEGER PRIMARY KEY,
  name TEXT UNIQUE NOT NULL
);
CREATE TABLE IF NOT EXISTS user(
  id INTEGER PRIMARY KEY,
  username TEXT UNIQUE NOT NULL,
  pw_hash TEXT NOT NULL,
  role TEXT NOT NULL CHECK (role IN ('ADMIN','LECTURER','HOD','STUDENT')),
  department_id INTEGER REFERENCES department(id),
  active INTEGER NOT NULL DEFAULT 1
);
CREATE TABLE IF NOT EXISTS course(
  id INTEGER PRIMARY KEY,
  code TEXT UNIQUE NOT NULL,
  title TEXT NOT NULL,
  department_id INTEGER NOT NULL REFERENCES department(id),
  lecturer_id INTEGER REFERENCES user(id)
);
CREATE TABLE IF NOT EXISTS enrolment(
  student_id INTEGER NOT NULL REFERENCES user(id),
  course_id INTEGER NOT NULL REFERENCES course(id),
  PRIMARY KEY (student_id, course_id)
);
CREATE TABLE IF NOT EXISTS assignment(
  id INTEGER PRIMARY KEY,
  course_id INTEGER NOT NULL REFERENCES course(id),
  title TEXT NOT NULL,
  deadline TEXT NOT NULL,
  max_mark REAL NOT NULL CHECK (max_mark > 0)
);
CREATE TABLE IF NOT EXISTS extension(
  id INTEGER PRIMARY KEY,
  assignment_id INTEGER NOT NULL REFERENCES assignment(id),
  student_id INTEGER NOT NULL REFERENCES user(id),
  new_deadline TEXT NOT NULL,
  reason TEXT NOT NULL,
  granted_by INTEGER NOT NULL REFERENCES user(id),
  UNIQUE (assignment_id, student_id)
);
CREATE TABLE IF NOT EXISTS submission(
  id INTEGER PRIMARY KEY,
  assignment_id INTEGER NOT NULL REFERENCES assignment(id),
  student_id INTEGER NOT NULL REFERENCES user(id),
  state TEXT NOT NULL,
  submitted_at TEXT NOT NULL,
  days_late INTEGER NOT NULL DEFAULT 0,
  content TEXT NOT NULL,
  mark REAL,
  feedback TEXT,
  UNIQUE (assignment_id, student_id)
);
CREATE TABLE IF NOT EXISTS audit_log(
  id INTEGER PRIMARY KEY,
  submission_id INTEGER NOT NULL REFERENCES submission(id),
  changed_by INTEGER NOT NULL REFERENCES user(id),
  changed_at TEXT NOT NULL,
  old_value REAL,
  new_value REAL NOT NULL,
  reason TEXT NOT NULL
);
CREATE INDEX IF NOT EXISTS ix_sub_student ON submission(student_id);
CREATE INDEX IF NOT EXISTS ix_sub_assign ON submission(assignment_id);
CREATE INDEX IF NOT EXISTS ix_enrol_course ON enrolment(course_id);
CREATE INDEX IF NOT EXISTS ix_course_dept ON course(department_id);
CREATE INDEX IF NOT EXISTS ix_audit_sub ON audit_log(submission_id);
)SQL");
}

Statement::Statement(sqlite3* db, const std::string& sql) {
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt_, nullptr) != SQLITE_OK)
        throwSqlite(db, "prepare");
}
Statement::~Statement() { if (stmt_) sqlite3_finalize(stmt_); }

void Statement::bind(int idx, int v) { sqlite3_bind_int(stmt_, idx, v); }
void Statement::bind(int idx, double v) { sqlite3_bind_double(stmt_, idx, v); }
void Statement::bind(int idx, const std::string& v) {
    sqlite3_bind_text(stmt_, idx, v.c_str(), (int)v.size(), SQLITE_TRANSIENT);
}
void Statement::bindNull(int idx) { sqlite3_bind_null(stmt_, idx); }
bool Statement::step() {
    int rc = sqlite3_step(stmt_);
    if (rc == SQLITE_ROW) return true;
    if (rc == SQLITE_DONE) return false;
    throw std::runtime_error(std::string("sqlite step error: ") + sqlite3_errstr(rc));
}
int Statement::colInt(int i) const { return sqlite3_column_int(stmt_, i); }
double Statement::colDouble(int i) const { return sqlite3_column_double(stmt_, i); }
std::string Statement::colText(int i) const {
    auto p = reinterpret_cast<const char*>(sqlite3_column_text(stmt_, i));
    return p ? std::string(p) : std::string();
}
bool Statement::colIsNull(int i) const {
    return sqlite3_column_type(stmt_, i) == SQLITE_NULL;
}

// ---------- UserRepository ----------
UserRecord UserRepository::rowToRecord(Statement& s) {
    UserRecord r;
    r.id = s.colInt(0);
    r.username = s.colText(1);
    r.passwordHash = s.colText(2);
    r.role = roleFromString(s.colText(3));
    r.departmentId = s.colIsNull(4) ? -1 : s.colInt(4);
    r.active = s.colInt(5) != 0;
    return r;
}

std::optional<UserRecord> UserRepository::findByUsername(const std::string& uname) {
    Statement s(db_.raw(), "SELECT id,username,pw_hash,role,department_id,active FROM user WHERE username=?");
    s.bind(1, uname);
    if (s.step()) return rowToRecord(s);
    return std::nullopt;
}

std::optional<UserRecord> UserRepository::findById(int id) {
    Statement s(db_.raw(), "SELECT id,username,pw_hash,role,department_id,active FROM user WHERE id=?");
    s.bind(1, id);
    if (s.step()) return rowToRecord(s);
    return std::nullopt;
}

int UserRepository::create(const std::string& uname, const std::string& hash, Role role,
                           int deptId, bool active) {
    Statement s(db_.raw(), "INSERT INTO user(username,pw_hash,role,department_id,active) VALUES(?,?,?,?,?)");
    s.bind(1, uname);
    s.bind(2, hash);
    s.bind(3, roleToString(role));
    if (deptId >= 0) s.bind(4, deptId); else s.bindNull(4);
    s.bind(5, active ? 1 : 0);
    s.step();
    return (int)sqlite3_last_insert_rowid(db_.raw());
}

void UserRepository::update(const UserRecord& u) {
    Statement s(db_.raw(), "UPDATE user SET username=?,role=?,department_id=?,active=? WHERE id=?");
    s.bind(1, u.username);
    s.bind(2, roleToString(u.role));
    if (u.departmentId >= 0) s.bind(3, u.departmentId); else s.bindNull(3);
    s.bind(4, u.active ? 1 : 0);
    s.bind(5, u.id);
    s.step();
}

std::vector<UserRecord> UserRepository::listAll() {
    Statement s(db_.raw(), "SELECT id,username,pw_hash,role,department_id,active FROM user ORDER BY id");
    std::vector<UserRecord> out;
    while (s.step()) out.push_back(rowToRecord(s));
    return out;
}

std::vector<UserRecord> UserRepository::listByDepartment(int deptId) {
    Statement s(db_.raw(), "SELECT id,username,pw_hash,role,department_id,active FROM user WHERE department_id=? ORDER BY id");
    s.bind(1, deptId);
    std::vector<UserRecord> out;
    while (s.step()) out.push_back(rowToRecord(s));
    return out;
}

std::vector<UserRecord> UserRepository::listByRole(Role r) {
    Statement s(db_.raw(), "SELECT id,username,pw_hash,role,department_id,active FROM user WHERE role=? ORDER BY id");
    s.bind(1, roleToString(r));
    std::vector<UserRecord> out;
    while (s.step()) out.push_back(rowToRecord(s));
    return out;
}

// ---------- DepartmentRepository ----------
int DepartmentRepository::create(const std::string& name) {
    Statement s(db_.raw(), "INSERT INTO department(name) VALUES(?)");
    s.bind(1, name); s.step();
    return (int)sqlite3_last_insert_rowid(db_.raw());
}
std::optional<DepartmentRow> DepartmentRepository::findById(int id) {
    Statement s(db_.raw(), "SELECT id,name FROM department WHERE id=?");
    s.bind(1, id);
    if (s.step()) return DepartmentRow{s.colInt(0), s.colText(1)};
    return std::nullopt;
}
std::vector<DepartmentRow> DepartmentRepository::listAll() {
    Statement s(db_.raw(), "SELECT id,name FROM department ORDER BY id");
    std::vector<DepartmentRow> out;
    while (s.step()) out.push_back({s.colInt(0), s.colText(1)});
    return out;
}

// ---------- CourseRepository ----------
int CourseRepository::create(const std::string& code, const std::string& title,
                             int deptId, int lecturerId) {
    Statement s(db_.raw(), "INSERT INTO course(code,title,department_id,lecturer_id) VALUES(?,?,?,?)");
    s.bind(1, code); s.bind(2, title); s.bind(3, deptId);
    if (lecturerId >= 0) s.bind(4, lecturerId); else s.bindNull(4);
    s.step();
    return (int)sqlite3_last_insert_rowid(db_.raw());
}
std::optional<CourseRow> CourseRepository::findById(int id) {
    Statement s(db_.raw(), "SELECT id,code,title,department_id,lecturer_id FROM course WHERE id=?");
    s.bind(1, id);
    if (s.step()) {
        CourseRow r; r.id=s.colInt(0); r.code=s.colText(1); r.title=s.colText(2);
        r.departmentId=s.colInt(3);
        r.lecturerId = s.colIsNull(4) ? -1 : s.colInt(4);
        return r;
    }
    return std::nullopt;
}
std::vector<CourseRow> CourseRepository::listAll() {
    Statement s(db_.raw(), "SELECT id,code,title,department_id,lecturer_id FROM course ORDER BY id");
    std::vector<CourseRow> out;
    while (s.step()) {
        CourseRow r; r.id=s.colInt(0); r.code=s.colText(1); r.title=s.colText(2);
        r.departmentId=s.colInt(3);
        r.lecturerId = s.colIsNull(4) ? -1 : s.colInt(4);
        out.push_back(r);
    }
    return out;
}
std::vector<CourseRow> CourseRepository::listByDepartment(int deptId) {
    Statement s(db_.raw(), "SELECT id,code,title,department_id,lecturer_id FROM course WHERE department_id=? ORDER BY id");
    s.bind(1, deptId);
    std::vector<CourseRow> out;
    while (s.step()) {
        CourseRow r; r.id=s.colInt(0); r.code=s.colText(1); r.title=s.colText(2);
        r.departmentId=s.colInt(3);
        r.lecturerId = s.colIsNull(4) ? -1 : s.colInt(4);
        out.push_back(r);
    }
    return out;
}
std::vector<CourseRow> CourseRepository::listByLecturer(int lecturerId) {
    Statement s(db_.raw(), "SELECT id,code,title,department_id,lecturer_id FROM course WHERE lecturer_id=? ORDER BY id");
    s.bind(1, lecturerId);
    std::vector<CourseRow> out;
    while (s.step()) {
        CourseRow r; r.id=s.colInt(0); r.code=s.colText(1); r.title=s.colText(2);
        r.departmentId=s.colInt(3);
        r.lecturerId = s.colIsNull(4) ? -1 : s.colInt(4);
        out.push_back(r);
    }
    return out;
}
void CourseRepository::assignLecturer(int courseId, int lecturerId) {
    Statement s(db_.raw(), "UPDATE course SET lecturer_id=? WHERE id=?");
    s.bind(1, lecturerId); s.bind(2, courseId); s.step();
}

// ---------- EnrolmentRepository ----------
void EnrolmentRepository::enrol(int studentId, int courseId) {
    Statement s(db_.raw(), "INSERT OR IGNORE INTO enrolment(student_id,course_id) VALUES(?,?)");
    s.bind(1, studentId); s.bind(2, courseId); s.step();
}
bool EnrolmentRepository::isEnrolled(int studentId, int courseId) {
    Statement s(db_.raw(), "SELECT 1 FROM enrolment WHERE student_id=? AND course_id=?");
    s.bind(1, studentId); s.bind(2, courseId);
    return s.step();
}
std::vector<CourseRow> EnrolmentRepository::listCoursesForStudent(int studentId) {
    Statement s(db_.raw(),
      "SELECT c.id,c.code,c.title,c.department_id,c.lecturer_id "
      "FROM course c JOIN enrolment e ON e.course_id=c.id WHERE e.student_id=? ORDER BY c.id");
    s.bind(1, studentId);
    std::vector<CourseRow> out;
    while (s.step()) {
        CourseRow r; r.id=s.colInt(0); r.code=s.colText(1); r.title=s.colText(2);
        r.departmentId=s.colInt(3);
        r.lecturerId = s.colIsNull(4) ? -1 : s.colInt(4);
        out.push_back(r);
    }
    return out;
}

// ---------- AssignmentRepository ----------
int AssignmentRepository::create(int courseId, const std::string& title,
                                 const std::string& deadline, double maxMark) {
    Statement s(db_.raw(),
      "INSERT INTO assignment(course_id,title,deadline,max_mark) VALUES(?,?,?,?)");
    s.bind(1, courseId); s.bind(2, title); s.bind(3, deadline); s.bind(4, maxMark);
    s.step();
    return (int)sqlite3_last_insert_rowid(db_.raw());
}
std::optional<AssignmentRow> AssignmentRepository::findById(int id) {
    Statement s(db_.raw(), "SELECT id,course_id,title,deadline,max_mark FROM assignment WHERE id=?");
    s.bind(1, id);
    if (s.step()) return AssignmentRow{s.colInt(0),s.colInt(1),s.colText(2),s.colText(3),s.colDouble(4)};
    return std::nullopt;
}
std::vector<AssignmentRow> AssignmentRepository::listForCourse(int courseId) {
    Statement s(db_.raw(),
      "SELECT id,course_id,title,deadline,max_mark FROM assignment WHERE course_id=? ORDER BY id");
    s.bind(1, courseId);
    std::vector<AssignmentRow> out;
    while (s.step())
        out.push_back({s.colInt(0),s.colInt(1),s.colText(2),s.colText(3),s.colDouble(4)});
    return out;
}

// ---------- ExtensionRepository ----------
void ExtensionRepository::upsert(int assignmentId, int studentId,
                                 const std::string& newDeadline,
                                 const std::string& reason, int grantedBy) {
    Statement s(db_.raw(),
      "INSERT INTO extension(assignment_id,student_id,new_deadline,reason,granted_by) "
      "VALUES(?,?,?,?,?) "
      "ON CONFLICT(assignment_id,student_id) DO UPDATE SET "
      "new_deadline=excluded.new_deadline, reason=excluded.reason, "
      "granted_by=excluded.granted_by");
    s.bind(1, assignmentId); s.bind(2, studentId); s.bind(3, newDeadline);
    s.bind(4, reason); s.bind(5, grantedBy);
    s.step();
}
std::optional<ExtensionRow> ExtensionRepository::find(int assignmentId, int studentId) {
    Statement s(db_.raw(),
      "SELECT id,assignment_id,student_id,new_deadline,reason,granted_by "
      "FROM extension WHERE assignment_id=? AND student_id=?");
    s.bind(1, assignmentId); s.bind(2, studentId);
    if (s.step())
        return ExtensionRow{s.colInt(0),s.colInt(1),s.colInt(2),s.colText(3),s.colText(4),s.colInt(5)};
    return std::nullopt;
}

// ---------- SubmissionRepository ----------
static Submission rowToSub(Statement& st) {
    Submission s;
    s.id = st.colInt(0);
    s.assignmentId = st.colInt(1);
    s.studentId = st.colInt(2);
    s.state = stateFromString(st.colText(3));
    s.submittedAt = st.colText(4);
    s.daysLate = st.colInt(5);
    s.content = st.colText(6);
    s.hasMark = !st.colIsNull(7);
    s.mark = s.hasMark ? st.colDouble(7) : 0.0;
    s.feedback = st.colText(8);
    return s;
}

int SubmissionRepository::create(const Submission& s) {
    Statement st(db_.raw(),
      "INSERT INTO submission(assignment_id,student_id,state,submitted_at,days_late,content,mark,feedback) "
      "VALUES(?,?,?,?,?,?,?,?)");
    st.bind(1, s.assignmentId); st.bind(2, s.studentId);
    st.bind(3, stateToString(s.state)); st.bind(4, s.submittedAt);
    st.bind(5, s.daysLate); st.bind(6, s.content);
    if (s.hasMark) st.bind(7, s.mark); else st.bindNull(7);
    st.bind(8, s.feedback);
    st.step();
    return (int)sqlite3_last_insert_rowid(db_.raw());
}
void SubmissionRepository::update(const Submission& s) {
    Statement st(db_.raw(),
      "UPDATE submission SET state=?,submitted_at=?,days_late=?,content=?,mark=?,feedback=? WHERE id=?");
    st.bind(1, stateToString(s.state)); st.bind(2, s.submittedAt);
    st.bind(3, s.daysLate); st.bind(4, s.content);
    if (s.hasMark) st.bind(5, s.mark); else st.bindNull(5);
    st.bind(6, s.feedback); st.bind(7, s.id);
    st.step();
}
std::optional<Submission> SubmissionRepository::findById(int id) {
    Statement st(db_.raw(),
      "SELECT id,assignment_id,student_id,state,submitted_at,days_late,content,mark,feedback "
      "FROM submission WHERE id=?");
    st.bind(1, id);
    if (st.step()) return rowToSub(st);
    return std::nullopt;
}
std::optional<Submission> SubmissionRepository::findByStudentAndAssignment(int studentId, int assignmentId) {
    Statement st(db_.raw(),
      "SELECT id,assignment_id,student_id,state,submitted_at,days_late,content,mark,feedback "
      "FROM submission WHERE student_id=? AND assignment_id=?");
    st.bind(1, studentId); st.bind(2, assignmentId);
    if (st.step()) return rowToSub(st);
    return std::nullopt;
}
std::vector<Submission> SubmissionRepository::listByAssignment(int assignmentId) {
    Statement st(db_.raw(),
      "SELECT id,assignment_id,student_id,state,submitted_at,days_late,content,mark,feedback "
      "FROM submission WHERE assignment_id=? ORDER BY id");
    st.bind(1, assignmentId);
    std::vector<Submission> out;
    while (st.step()) out.push_back(rowToSub(st));
    return out;
}
std::vector<Submission> SubmissionRepository::listByStudent(int studentId) {
    Statement st(db_.raw(),
      "SELECT id,assignment_id,student_id,state,submitted_at,days_late,content,mark,feedback "
      "FROM submission WHERE student_id=? ORDER BY id");
    st.bind(1, studentId);
    std::vector<Submission> out;
    while (st.step()) out.push_back(rowToSub(st));
    return out;
}

// ---------- AuditRepository ----------
void AuditRepository::insert(int submissionId, int changedBy,
                             const std::string& changedAt,
                             double oldValue, double newValue,
                             const std::string& reason) {
    Statement s(db_.raw(),
      "INSERT INTO audit_log(submission_id,changed_by,changed_at,old_value,new_value,reason) "
      "VALUES(?,?,?,?,?,?)");
    s.bind(1, submissionId); s.bind(2, changedBy); s.bind(3, changedAt);
    s.bind(4, oldValue); s.bind(5, newValue); s.bind(6, reason);
    s.step();
}
std::vector<AuditRow> AuditRepository::listBySubmission(int submissionId) {
    Statement s(db_.raw(),
      "SELECT id,submission_id,changed_by,changed_at,old_value,new_value,reason "
      "FROM audit_log WHERE submission_id=? ORDER BY id");
    s.bind(1, submissionId);
    std::vector<AuditRow> out;
    while (s.step()) {
        AuditRow r;
        r.id=s.colInt(0); r.submissionId=s.colInt(1); r.changedBy=s.colInt(2);
        r.changedAt=s.colText(3);
        r.oldValue = s.colIsNull(4) ? 0.0 : s.colDouble(4);
        r.newValue = s.colDouble(5); r.reason=s.colText(6);
        out.push_back(r);
    }
    return out;
}