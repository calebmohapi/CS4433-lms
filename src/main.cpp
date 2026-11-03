#include "ui.h"
#include <iostream>
#include <memory>

static void seed(Database &db)
{
    UserRepository users(db);
    if (users.findByUsername("admin"))
        return;

    DepartmentRepository depts(db);
    int cs = depts.create("Computer Science");
    int math = depts.create("Mathematics");

    auto mk = [&](const std::string &u, const std::string &p, Role r, int d)
    {
        return users.create(u, util::hashPassword(p), r, d, true);
    };
    mk("admin", "admin123", Role::ADMIN, -1);
    int hodId = mk("h=od", "hod123", Role::HOD, cs);
    int lect1Id = mk("lect1", "lect123", Role::LECTURER, cs);
    int lect2Id = mk("lect2", "lect123", Role::LECTURER, math);
    int stud1Id = mk("stud1", "stud123", Role::STUDENT, cs);
    int stud2Id = mk("stud2", "stud123", Role::STUDENT, cs);
    (void)hodId;

    CourseRepository courses(db);
    int c1 = courses.create("CS101", "Intro to Programming", cs, lect1Id);
    int c2 = courses.create("CS201", "Data Structures", cs, lect1Id);
    int c3 = courses.create("MA101", "Calculus", math, lect2Id);

    EnrolmentRepository enr(db);
    enr.enrol(stud1Id, c1);
    enr.enrol(stud1Id, c2);
    enr.enrol(stud2Id, c1);
    enr.enrol(stud2Id, c3);

    AssignmentRepository assigns(db);
    // Assignment 1: deadline well in the past (past 3 days -> zero penalty),
    // Assignment 2: deadline in future
    std::string past = util::addHours(util::nowIso(), -24 * 10);
    std::string future = util::addHours(util::nowIso(), 24 * 7);
    int a1 = assigns.create(c1, "Essay 1", past, 100.0);
    (void)a1;
    assigns.create(c2, "Project", future, 100.0);

    std::cout << "[seed] created demo data.\n"
              << "  admin/admin123\n"
              << "  hod/hod123 (CS)\n"
              << "  lect1/lect123, lect2/lect123\n"
              << "  stud1/stud123, stud2/stud123\n";
}

int main()
{
    try
    {
        Database db("lms.db");
        db.initializeSchema();
        seed(db);

        UserRepository users(db);
        DepartmentRepository depts(db);
        CourseRepository courses(db);
        EnrolmentRepository enrol(db);
        AssignmentRepository assigns(db);
        ExtensionRepository exts(db);
        SubmissionRepository subs(db);
        AuditRepository audits(db);

        AuthService auth(users);
        AdminService admin(users, depts, courses, enrol);
        LecturerService lecturer(assigns, subs, exts, courses, users, audits);
        StudentService student(subs, assigns, exts, enrol, courses);
        ModerationService mod(subs, assigns, courses, users, audits);

        ConsoleUI ui(auth, admin, lecturer, student, mod,
                     users, depts, courses, assigns, subs, exts, audits);
        ui.run();
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
}