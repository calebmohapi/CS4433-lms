#include "services.h"
#include <stdexcept>
#include <algorithm>
#include <cmath>

// ---------- Auth ----------
std::unique_ptr<User> AuthService::login(const std::string& username, const std::string& password) {
    auto row = users_.findByUsername(username);
    if (!row) return nullptr;
    if (!row->active) return nullptr;
    if (!util::verifyPassword(password, row->passwordHash)) return nullptr;
    return UserFactory::create(*row);
}

void AuthService::changePassword(int userId, const std::string& oldPw, const std::string& newPw) {
    auto row = users_.findById(userId);
    if (!row) throw std::runtime_error("User not found");
    if (!util::verifyPassword(oldPw, row->passwordHash))
        throw std::runtime_error("Current password is incorrect");
    if (newPw.size() < 6) throw std::runtime_error("New password too short (min 6)");
    row->passwordHash = util::hashPassword(newPw);
    users_.update(*row);
}

// ---------- Admin ----------
int AdminService::createUser(const std::string& uname, const std::string& password,
                             Role role, int deptId) {
    if (users_.findByUsername(uname))
        throw std::runtime_error("Username already exists");
    if (password.size() < 6)
        throw std::runtime_error("Password too short (min 6)");
    auto hash = util::hashPassword(password);
    return users_.create(uname, hash, role, deptId, true);
}
void AdminService::updateUser(int id, const std::string& uname, Role role,
                              int deptId, bool active) {
    auto u = users_.findById(id);
    if (!u) throw std::runtime_error("User not found");
    if (u->username != uname) {
        auto existing = users_.findByUsername(uname);
        if (existing && existing->id != id)
            throw std::runtime_error("Username taken");
    }
    u->username = uname; u->role = role;
    u->departmentId = deptId; u->active = active;
    users_.update(*u);
}
void AdminService::deactivateUser(int id) {
    auto u = users_.findById(id);
    if (!u) throw std::runtime_error("User not found");
    u->active = false;
    users_.update(*u);
}
int AdminService::createDepartment(const std::string& name) {
    return depts_.create(name);
}
int AdminService::createCourse(const std::string& code, const std::string& title, int deptId) {
    return courses_.create(code, title, deptId, -1);
}
void AdminService::assignLecturer(int courseId, int lecturerId) {
    auto u = users_.findById(lecturerId);
    if (!u || u->role != Role::LECTURER)
        throw std::runtime_error("User is not a lecturer");
    courses_.assignLecturer(courseId, lecturerId);
}
void AdminService::enrolStudent(int studentId, int courseId) {
    auto u = users_.findById(studentId);
    if (!u || u->role != Role::STUDENT)
        throw std::runtime_error("User is not a student");
    enrol_.enrol(studentId, courseId);
}

// ---------- Lecturer ----------
int LecturerService::createAssignment(int lecturerId, int courseId,
                                      const std::string& title,
                                      const std::string& deadlineIso, double maxMark) {
    auto course = courses_.findById(courseId);
    if (!course) throw std::runtime_error("Course not found");
    if (course->lecturerId != lecturerId)
        throw std::runtime_error("You do not teach this course");
    if (maxMark <= 0) throw std::runtime_error("Max mark must be > 0");
    return assigns_.create(courseId, title, deadlineIso, maxMark);
}

std::vector<AssignmentRow> LecturerService::listAssignments(int lecturerId) {
    std::vector<AssignmentRow> all;
    for (auto& c : courses_.listByLecturer(lecturerId)) {
        auto a = assigns_.listForCourse(c.id);
        all.insert(all.end(), a.begin(), a.end());
    }
    return all;
}

void LecturerService::grantExtension(int lecturerId, int assignmentId, int studentId,
                                     const std::string& newDeadline, const std::string& reason) {
    if (reason.empty()) throw std::runtime_error("Reason is required");
    auto a = assigns_.findById(assignmentId);
    if (!a) throw std::runtime_error("Assignment not found");
    auto c = courses_.findById(a->courseId);
    if (!c || c->lecturerId != lecturerId)
        throw std::runtime_error("You do not teach this course");
    if (util::parseIsoToEpoch(newDeadline) <= util::parseIsoToEpoch(a->deadline))
        throw std::runtime_error("Extension must be after original deadline");
    exts_.upsert(assignmentId, studentId, newDeadline, reason, lecturerId);
}

double LecturerService::markSubmission(int lecturerId, int submissionId,
                                       double mark, const std::string& feedback) {
    auto sub = subs_.findById(submissionId);
    if (!sub) throw std::runtime_error("Submission not found");
    auto a = assigns_.findById(sub->assignmentId);
    if (!a) throw std::runtime_error("Assignment not found");
    auto c = courses_.findById(a->courseId);
    if (!c || c->lecturerId != lecturerId)
        throw std::runtime_error("You do not teach this course");
    if (sub->state != SubmissionState::SUBMITTED &&
        sub->state != SubmissionState::LATE &&
        sub->state != SubmissionState::RETURNED)
        throw std::runtime_error("Submission is not markable (state=" + stateToString(sub->state) + ")");
    if (mark < 0 || mark > a->maxMark)
        throw std::runtime_error("Mark must be in [0, " + std::to_string((int)a->maxMark) + "]");

    TenPercentPerDayPolicy policy;
    double penalty = policy.compute(a->maxMark, sub->daysLate);
    double finalMark = std::max(0.0, mark - penalty);
    double oldMark = sub->hasMark ? sub->mark : 0.0;

    sub->mark = finalMark;
    sub->hasMark = true;
    sub->feedback = feedback;
    sub->state = SubmissionState::MARKED;
    subs_.update(*sub);

    AuditLogger logger(audits_);
    sub->clearObservers();
    sub->registerObserver(&logger);
    sub->recordMarkChange(lecturerId, oldMark, finalMark, "Marked by lecturer");
    return finalMark;
}

// ---------- Student ----------
void StudentService::submit(int studentId, int assignmentId, const std::string& content) {
    auto a = assigns_.findById(assignmentId);
    if (!a) throw std::runtime_error("Assignment not found");
    if (!enrol_.isEnrolled(studentId, a->courseId))
        throw std::runtime_error("You are not enrolled in this course");

    std::string deadline = a->deadline;
    auto ext = exts_.find(assignmentId, studentId);
    if (ext) deadline = ext->newDeadline;

    int64_t now = util::nowEpoch();
    int64_t dl = util::parseIsoToEpoch(deadline);

    int daysLate = 0;
    bool tooLate = false;
    if (now > dl) {
        double elapsedH = double(now - dl) / 3600.0;
        if (elapsedH > 72.0) { tooLate = true; daysLate = 4; }
        else {
            daysLate = (int)std::ceil(elapsedH / 24.0);
            if (daysLate < 1) daysLate = 1;
        }
    }

    auto existing = subs_.findByStudentAndAssignment(studentId, assignmentId);
    if (existing) {
        if (existing->state == SubmissionState::SUBMITTED && now <= dl) {
            existing->content = content;
            existing->submittedAt = util::nowIso();
            subs_.update(*existing);
            return;
        }
        if (existing->state == SubmissionState::LATE)
            throw std::runtime_error("Cannot resubmit: submission is already late");
        throw std::runtime_error("Cannot resubmit: submission already in state "
                                 + stateToString(existing->state));
    }

    Submission s;
    s.assignmentId = assignmentId;
    s.studentId = studentId;
    s.state = (now > dl) ? SubmissionState::LATE : SubmissionState::SUBMITTED;
    s.submittedAt = util::nowIso();
    s.daysLate = tooLate ? 3 : daysLate;
    s.content = content;
    s.feedback = "";
    if (tooLate) { s.hasMark = true; s.mark = 0; }
    subs_.create(s);
}

std::vector<Submission> StudentService::myReleasedSubmissions(int studentId) {
    std::vector<Submission> out;
    for (auto& s : subs_.listByStudent(studentId))
        if (s.state == SubmissionState::RELEASED) out.push_back(s);
    return out;
}

// ---------- Moderation ----------
bool ModerationService::hodCanAccess(int hodId, int submissionId) {
    auto sub = subs_.findById(submissionId);
    if (!sub) return false;
    auto a = assigns_.findById(sub->assignmentId);
    if (!a) return false;
    auto c = courses_.findById(a->courseId);
    if (!c) return false;
    auto h = users_.findById(hodId);
    if (!h) return false;
    return h->departmentId == c->departmentId;
}

std::vector<Submission> ModerationService::listMarkedInDept(int hodId) {
    auto h = users_.findById(hodId);
    if (!h) throw std::runtime_error("HoD not found");
    std::vector<Submission> out;
    for (auto& c : courses_.listByDepartment(h->departmentId)) {
        for (auto& a : assigns_.listForCourse(c.id)) {
            for (auto& s : subs_.listByAssignment(a.id))
                if (s.state == SubmissionState::MARKED) out.push_back(s);
        }
    }
    return out;
}

std::vector<Submission> ModerationService::listModeratedInDept(int hodId) {
    auto h = users_.findById(hodId);
    if (!h) throw std::runtime_error("HoD not found");
    std::vector<Submission> out;
    for (auto& c : courses_.listByDepartment(h->departmentId)) {
        for (auto& a : assigns_.listForCourse(c.id)) {
            for (auto& s : subs_.listByAssignment(a.id))
                if (s.state == SubmissionState::MODERATED) out.push_back(s);
        }
    }
    return out;
}

void ModerationService::approveMark(int hodId, int submissionId) {
    if (!hodCanAccess(hodId, submissionId))
        throw std::runtime_error("Access denied");
    auto sub = subs_.findById(submissionId);
    if (!sub || sub->state != SubmissionState::MARKED)
        throw std::runtime_error("Submission not in MARKED state");
    sub->state = SubmissionState::MODERATED;
    subs_.update(*sub);
}

void ModerationService::adjustMark(int hodId, int submissionId,
                                   double newMark, const std::string& reason) {
    if (!hodCanAccess(hodId, submissionId))
        throw std::runtime_error("Access denied");
    if (reason.empty()) throw std::runtime_error("Reason is required");
    auto sub = subs_.findById(submissionId);
    if (!sub) throw std::runtime_error("Submission not found");
    if (sub->state != SubmissionState::MARKED)
        throw std::runtime_error("Submission not in MARKED state");
    auto a = assigns_.findById(sub->assignmentId);
    if (!a) throw std::runtime_error("Assignment not found");
    if (newMark < 0 || newMark > a->maxMark)
        throw std::runtime_error("Mark out of range");
    double oldMark = sub->hasMark ? sub->mark : 0.0;
    sub->mark = newMark;
    sub->hasMark = true;
    sub->state = SubmissionState::MODERATED;
    subs_.update(*sub);

    AuditLogger logger(audits_);
    sub->clearObservers();
    sub->registerObserver(&logger);
    sub->recordMarkChange(hodId, oldMark, newMark, "Moderated: " + reason);
}

void ModerationService::returnToLecturer(int hodId, int submissionId,
                                         const std::string& comment) {
    if (!hodCanAccess(hodId, submissionId))
        throw std::runtime_error("Access denied");
    auto sub = subs_.findById(submissionId);
    if (!sub || sub->state != SubmissionState::MARKED)
        throw std::runtime_error("Submission not in MARKED state");
    sub->state = SubmissionState::RETURNED;
    if (!comment.empty())
        sub->feedback = sub->feedback + "\n[HoD]: " + comment;
    subs_.update(*sub);
}

void ModerationService::releaseMark(int hodId, int submissionId) {
    if (!hodCanAccess(hodId, submissionId))
        throw std::runtime_error("Access denied");
    auto sub = subs_.findById(submissionId);
    if (!sub) throw std::runtime_error("Submission not found");
    if (sub->state != SubmissionState::MODERATED)
        throw std::runtime_error("Submission not in MODERATED state");
    sub->state = SubmissionState::RELEASED;
    subs_.update(*sub);
}