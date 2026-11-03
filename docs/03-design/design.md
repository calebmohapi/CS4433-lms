# 1. Architecture

## 1.1 Architectural style and justification

This system shall adopt a layered architecture style which follows: Presentation ⇒ Application ⇒ Domain ⇒ Data Access ⇒ SQLite storage.

### Why Layered style?

- Due to separated concerns of the system such as console menus, role dependency, business rules such as penalties and moderation, a layered style keeps each concern in its own layer and lets a layer only depend to the one beneath it.
- The domain layer which deals with penalty calculation, penalty checks, and state transitions can be unit-tested with GoogleTest without a database or console attached.
- It allows role separation where the application layer enforce one service per actor group like StudentService, LecturerService, AdminService, etc and the presentation layer only decides what to show.

## 1.2 Component diagram

*refer to [View Image](diagrams/component-diagram.png)

## 1.3 Alternative considered and rejected

Model-View Controller is a suitable alternative but it is rejected because it is well suited to interfaces where the view must update reactively as the model changes such graphical user interfaces. This system does not require GUI, the console only redraws when the user makes a choice.

# 2. Class Diagram

## 2.1 Diagram

refer to [View Image](diagrams/class-diagram.png)

## 2.2 Design patterns and why each was chosen

- **Factory** - `UserFactory`. User is abstract with four concrete subclasses. Wherever a user is created (admin account creation, login after reading a row back from the database), the caller needs a User without knowing which subclass to instantiate until it reads the role column.
- **Strategy** - `IPenaltyPolicy` / `TenPercentPerDayPolicy`. The late-penalty rule is likely to be renegotiated in a way of a future change request and might want a different formula. Submission depends only on the `IPenaltyPolicy` interface, so a new rule is a new class implementing `compute(maxMark, daysLate)`, with no change to Submission itself.
- **Observer** - `ISubmissionObserver` / `AuditLogger`. The system requires that every mark-affecting action be audited, and the system must not be able to "forget" to audit one because a future feature added a new way to change a mark. Submission notifies registered observers whenever a mark changes. AuditLogger is the only observer and writes the audit_log row. Adding a future observer such as a notification service requires no change to Submission, only a new class implementing `ISubmissionObserver`.
- **Repository / DAO** - Repository interfaces plus `SqliteRepository` implementations. Every use case reads or writes SQLite, but the domain and application layers must not depend on SQL or the SQLite C API directly. Each aggregate root (User, Submission, etc.) gets one repository interface.

## 2.3 Why this design?

- **Single responsibility:** Submission manages lifecycle state only and does not compute penalties or write audit logs.
- **Open/Closed:** new penalty rules or new observers extend the system without modifying unrelated source such as Submission.
- **Interface Segregation:** repository interfaces are scoped to one aggregate so a class depending on one class methods is not forced to depend on unrelated class methods.
- **Dependency Inversion:** Every layer depends on abstractions (interfaces) in the layer below not on concrete implementations.

# 3. Behavioural Models

## 3.1 State machine diagram: Submission life cycle

refer to [View Image](diagrams/State-machine-diagram.png)

## 3.2 Activity Diagram

refer to [View Image](diagrams/Activity-diagram.png)

## 3.3 Sequence diagram: Submit Assignment

refer to [View Image](diagrams/Sequence-diagram-Submit-Assignment.png)

## 3.4 Sequence diagram: Release and Moderate Marks

refer to [View Image](diagrams/Sequence-diagram-Moderate-and-Release-marks.png)

# 4. Database Design

## 4.1 Entity Relationship Diagram

refer to [View Image](diagrams/erd.png)

## 4.2 Data Dictionary

| Table | Column | Type | Constraint | Meaning |
| --- | --- | --- | --- | --- |
| **department** | id | INTEGER | PK | Department identifier |
| | name | TEXT | UNIQUE NOT NULL | Department name |
| **user** | id | INTEGER | PK | User identifier |
| | username | TEXT | UNIQUE NOT NULL | Login name |
| | pw_hash | TEXT | NOT NULL | Argon2id salted hash (salt embedded in the encoded string — see Week 4) |
| | role | TEXT | CHECK IN (ADMIN, LECTURER, HOD, STUDENT) | Discriminator for single-table inheritance (§4.4) |
| | department_id | INTEGER | FK → department.id, NULL for students before enrolment | Owning department (staff) |
| | active | INTEGER | NOT NULL DEFAULT 1 | Soft-deactivation flag (FR3) |
| **course** | id | INTEGER | PK | Course identifier |
| | code | TEXT | UNIQUE NOT NULL | Course code |
| | title | TEXT | NOT NULL | Course title |
| | department_id | INTEGER | FK → department.id NOT NULL | Owning department |
| | lecturer_id | INTEGER | FK → user.id | Assigned lecturer (FR4), nullable until assigned |
| **enrolment** | student_id | INTEGER | FK → user.id, PK (composite) | Resolves |
| | course_id | INTEGER | FK → course.id, PK | Resolves |
| **extension** | id | INTEGER | PK | Extension identifier |
| | assignment_id | INTEGER | FK → assignment.id | Linked assignment |
| | student_id | INTEGER | FK → user.id | Linked student |
| | new_deadline | TEXT | NOT NULL | Extended deadline |
| | reason | TEXT | NOT NULL | Reason for extension |
| | granted_by | INTEGER | FK → user.id | Lecturer who granted |
| **submission** | id | INTEGER | PK | Submission identifier |
| | assignment_id | INTEGER | FK → assignment.id | Linked assignment |
| | student_id | INTEGER | FK → user.id | Linked student |
| | state | TEXT | NOT NULL | Lifecycle state |
| | submitted_at | TEXT | NOT NULL | Timestamp |
| | days_late | INTEGER | NOT NULL DEFAULT 0 | Days late |
| | content | TEXT | NOT NULL | Submission content |
| | mark | REAL | Nullable | Awarded mark |
| | feedback | TEXT | Nullable | Written feedback |
| **audit_log** | id | INTEGER | PK | Audit identifier |
| | submission_id | INTEGER | FK → submission.id | Affected submission |
| | changed_by | INTEGER | FK → user.id | Actor |
| | changed_at | TEXT | NOT NULL | Timestamp |
| | old_value | REAL | Nullable | Previous mark |
| | new_value | REAL | NOT NULL | New mark |
| | reason | TEXT | NOT NULL | Reason for change (FR14) |

## 4.3 SQL Schema

```sql
CREATE TABLE department (
    id INTEGER PRIMARY KEY,
    name TEXT UNIQUE NOT NULL
);

CREATE TABLE user (
    id INTEGER PRIMARY KEY,
    username TEXT UNIQUE NOT NULL,
    pw_hash TEXT NOT NULL,
    role TEXT NOT NULL CHECK (role IN ('ADMIN','LECTURER','HOD','STUDENT')),
    department_id INTEGER REFERENCES department(id),
    active INTEGER NOT NULL DEFAULT 1
);

CREATE TABLE course (
    id INTEGER PRIMARY KEY,
    code TEXT UNIQUE NOT NULL,
    title TEXT NOT NULL,
    department_id INTEGER NOT NULL REFERENCES department(id),
    lecturer_id INTEGER REFERENCES user(id)
);

CREATE TABLE enrolment (
    student_id INTEGER NOT NULL REFERENCES user(id),
    course_id INTEGER NOT NULL REFERENCES course(id),
    PRIMARY KEY (student_id, course_id)
);

CREATE TABLE assignment (
    id INTEGER PRIMARY KEY,
    course_id INTEGER NOT NULL REFERENCES course(id),
    title TEXT NOT NULL,
    deadline TEXT NOT NULL,
    max_mark REAL NOT NULL CHECK (max_mark > 0)
);

CREATE TABLE extension (
    id INTEGER PRIMARY KEY,
    assignment_id INTEGER NOT NULL REFERENCES assignment(id),
    student_id INTEGER NOT NULL REFERENCES user(id),
    new_deadline TEXT NOT NULL,
    reason TEXT NOT NULL,
    granted_by INTEGER NOT NULL REFERENCES user(id),
    UNIQUE (assignment_id, student_id)
);

CREATE TABLE submission (
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

CREATE TABLE audit_log (
    id INTEGER PRIMARY KEY,
    submission_id INTEGER NOT NULL REFERENCES submission(id),
    changed_by INTEGER NOT NULL REFERENCES user(id),
    changed_at TEXT NOT NULL,
    old_value REAL,
    new_value REAL NOT NULL,
    reason TEXT NOT NULL
);

CREATE INDEX ix_sub_student ON submission(student_id);
