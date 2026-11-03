# Requirements Engineering

# 1. Stakeholder Analysis and Conflict Resolution

## 1.1 Stakeholders and Interests

| Stakeholder | Principal | Type | Managerial Interest |
| --- | --- | --- | --- |
| Principal | Managerial | | Simplicity for non-IT staff and academic integrity of released marks |
| Lecturer | System's user | | Fast release of marks to students and flexibility to give individual extensions |
| Head of Department | System's user | | Oversight and consistency of marks across the department with fairness of penalties |
| Student rep | System's user | | Chance to correct mistakes and resubmit within the deadline. Feedback must be written and legitimate late submissions must be accepted. |
| IT Officer | Technical user | | Reliable system offline with password security and smooth responsiveness |
| System Administrator | System's user | | Manages accounts, departments and courses all on their own accord without external input |
| Students | System's user | | Privacy for their submissions and marks |

## 1.2 Conflicts and Ambiguities, with Resolution

| ID | Conflicting statements | Resolution |
| --- | --- | --- |
| C1 | Principal: "Students must not see any mark until it has been checked by the Head of Department"<br>Lecturer: "I want students to get their marks as soon as I have marked the work" | Lecturer marking is provisional and only the HoD's release makes a mark visible to the student. |
| C2 | Lecturer want to be able to extend deadlines but the HoD's 3-Days/10% per-day suggests a fixed deadline policy. | Extensions are allowed but changes only that specific student deadline and then the late submission policy starts after the extended deadline, not the original one. |
| C3 | HoD wants zero mark after 3 days of late submission but the student rep says late work should not simply be discarded. | Submission is always be accepted but scores zero after day 3 and the lecturer may still write feedback and keep the submission on record. |
| C4 | The principal says the system must be simple but also the system requires the need for audit, role-based access and other Hod and IT Officer features | Simplicity is delivered at the user-interface. The system's audit, roles and other engines run in the backend invisible to the user. |
| C5 | HoD needs to see every mark before release but it is never stated how changing a mark is handled once submitted by lecturer. | Every mark change must write an audit entry stating who, when, old value, new value and reason for change. |

# 2. Software Requirements Specification

## 2.1 Functional Requirements

| ID | Requirement | Priority | Verification |
| --- | --- | --- | --- |
| FR1 | The system shall authenticate a user by username and password. | Must | Login with credentials valid/invalid |
| FR2 | The system shall restrict every function to the roles permitted for it and rejects any attempt by unpermitted role. | Must | Attempt each function as each role. |
| FR3 | The administrator shall be able to create, edit, and deactivate user accounts, departments and courses. | Must | CRUD test per entry |
| FR4 | The administrator must be able to assign a lecturer to a course and enrol a student in a course. | Must | Assignment/Enrolment test |
| FR5 | The lecturer shall be able to create an assignment for a course they teach, with a title, deadline and maximum marks. | Must | Create assignment, check persisted fields |
| FR6 | The Lecturer shall be able to grant an individual student an extension, with a reason, for a specific assignment. | Must | Grant extension, check deadline shift and audit |
| FR7 | The student shall be able to view the list of courses they are enrolled in and the assignments for each. | Must | View as a seeded student |
| FR8 | The student shall be able to submit work for an assignment and resubmit before the deadline, with the latest version replacing the previous one. | Must | Submit, resubmit, check latest wins |
| FR9 | The system shall accept the submission made after the deadline, record it as late and compute a penalty of 10% of the maximum mark per stated 24-hour period up to 72-hours, after which the submission receives zero. | Must | Boundary tests at deadline ±0s +24h, +48h, +72h, +72h+1s |
| FR10 | The lecturer shall be able to record a mark and written feedback for a submission. | Must | Mark entry, reject out of range marks |
| FR11 | The head of Department shall be able to view every mark for every course in their own department and shall not be able to view marks for other departments. | Must | View as HoD of Department A and attempt Department B |
| FR12 | The Head of Department shall be able to approve a mark unchanged or adjust it before release. | Must | Approve path and adjust path |
| FR13 | The Head of Department shall be able to release approved marks after which the mark and feedback becomes visible to the student. | Must | Release then view as student |
| FR14 | The system shall record every mark creation and change in an audit log containing who made it, when, the old value, the new value, and the reason. | Must | Query audit log after every mark- affecting action |
| FR15 | The system shall enforce a submission life cycle (Submitted → Marked → Moderated → Released) and reject any action that attempts an invalid transition. | Must | Attempt each invalid transition. |

## 2.2 Non-functional Requirements

| ID | Requirement | Priority | Verification |
| --- | --- | --- | --- |
| NFR1 | Login shall complete within 2 seconds with all 800 students stored in the database. | Must | Timed test against an 800-row seed. |
| NFR2 | The system shall be fully usable offline without any internet connection. | Must | Run full workflow with internet disabled |
| NFR3 | No password shall be stored or logged plaintext anywhere in the system. | Must | Inspect database and log files after use |
| NFR4 | Invalid input shall never crash the system but rather rejected with a message and continue the session. | Must | Negative-input test with zero crashes |
| NFR5 | Automated tests shall achieve at least 70%-line coverage of the business logic layers. | Must | Coverage report |
| NFR6 | All data stored shall remain available even after system or machine restart and a backup of the system shall restore correctly within 5 minutes | Must | Kill process mid-session, restart and verify data. |

# 3. Use Case Model

## 3.1 Use case diagram

refer to [View Image](diagrams/Use-case-diagram.png)

## 3.2 Fully dressed use case specifications

### Use-Case Specification 01: Submit Assignment

| Field | Detail |
| --- | --- |
| **Actor** | Student |
| **Preconditions** | 1. Student is logged into the system.<br>2. Student is enrolled in the assignment's course.<br>3. Assignment exists. |
| **Trigger** | Student selects "Submit or resubmit work" for an assignment |
| **Main flow** | 1. System displays the assignment's deadline and maximum mark.<br>2. Student provides the submission content.<br>3. System checks current time against the deadline.<br>4. If on time, system stores the submission in state 'Submitted'.<br>5. System confirms the submission to the student. |
| **Alternative flow A: Resubmission** | At step 3 in main flow, if a submission already exists for this assignment and the student and the deadline has not passed, the system replaces the stored work with the new work, keeps the state 'Submitted' and confirms. |
| **Alternative flow B: Late Submission** | At step 3 in main flow, if the current time is after the deadline but within 72 hours, the system accepts the submission, sets state 'Late', computes the elapsed whole days late and applies late penalty policy. |
| **Alternate flow C: Extension applies** | If the lecturer has granted this student an extension for this assignment, the deadline used in step 3 of main flow and the penalty calculation is the extended deadline, not the original one. |
| **Exception flow D: Too Late** | At step 3, if more than 72 hours have passed since the deadline and no prior on-time submission exists, the system accepts the submission, sets state 'Late' with the mark pre-set to 0 at marking time, and records that the zero resulted from lateness. |
| **Exception flow E: Not enrolled** | If the student is not enrolled in the assignment's course, the system rejects the action with a generic "not available" message (no information disclosure about course existence) and logs the attempt. |
| **Exception flow F: Invalid content** | If the submission content fails validation (empty, exceeds size limit), the system rejects it with a specific message and does not change the stored state. |
| **Postconditions** | A submission exists in state 'Submitted' or 'Late' for this student and assignment; if late, days_late and the computed penalty are stored. |
| **Business Rules** | BR1: 10% of max mark per started 24h period late, capped at 3 days.<br>BR2: latest submission replaces earlier ones before the deadline; exactly one late submission accepted after it.<br>BR3: extension shifts the effective deadline used by this use case. |

### Use-Case Specification 02: Mark Submission

| Field | Detail |
| --- | --- |
| **Actor** | Lecturer |
| **Preconditions** | 1. Lecturer is logged in.<br>2. Lecturer teaches the course.<br>3. Submission exists in state 'Submitted' or 'Late'. |
| **Trigger** | Lecturer selects "Mark submissions" for an assignment |
| **Main flow** | 1. System lists outstanding submissions for the assignment.<br>2. Lecturer selects one.<br>3. System displays the submission content and any late-penalty already computed.<br>4. Lecturer enters a mark and written feedback.<br>5. System validates the mark range, stores mark and feedback, sets state Marked, and writes an audit entry recording the new mark with reason. |
| **Alternative flow A: Late penalty applied** | At step 4, if the submission is late, the system subtracts the stored penalty from the entered mark before saving. Shows the lecturer both the original and penalized marks. |
| **Alternative flow B: Extension** | If, while marking, the lecturer judges the student needed more time, they may instead grant extension and stop marking. |
| **Exception flow C: Mark out of range** | At step 4, if the entered mark is negative or exceeds the assignment's maximum, the system rejects the input with a message and re-prompts. |
| **Exception flow D: Not the course's lecturer** | If the acting user does not teach the course, the system rejects the action and logs the attempt. |
| **Postconditions** | Submission state is 'Marked'. Mark and feedback stored but not visible to the student and one audit entry exists for this mark. |
| **Business rules** | BR4: A 'Marked' submission is never visible to the student until released by the HoD. |

### Use-Case Specification 03: Moderate and Release Marks

| Field | Detail |
| --- | --- |
| **Actor** | Head of Department |
| **Preconditions** | HoD is logged in; HoD's department matches the course's department; submission is in state Marked |
| **Trigger** | HoD selects "Moderate marks" for a department course |
| **Main flow** | 1. System lists all Marked submissions in the HoD's own department.<br>2. HoD selects one and reviews mark and feedback.<br>3. HoD approves the mark unchanged.<br>4. System sets state Moderated.<br>5. HoD (separately, "Release approved marks") selects moderated submissions and releases them.<br>6. System sets state Released; mark and feedback become visible to the student. |
| **Alternative flow A** | At step 3, HoD instead enters a new mark and a reason for the change. System validates the new mark, stores it, sets state "Moderated", and writes an audit entry with old value, new value, reason, moderator identity. |
| **Alternative flow B** | At step 3, HoD may instead return the submission to the lecturer with a comment. Lecturer must re-mark before it can be moderated again. |
| **Exception flow C** | If the selected submission's course is outside the HoD's own department, the system rejects the action and logs the attempt. |
| **Exception flow D** | If HoD attempts to release a submission that is not in state "Moderated", the system rejects the action. |
| **Postconditions** | Submission state is "Moderated" or "Released". If adjusted, an audit entry with a reason exists. Once 'Released', the student can view the mark and feedback. |
| **Business rules** | BR5: Any mark value change by the HoD requires a reason, enforced the same way as the lecturer's initial mark.<br>BR6: Release is the only action that makes a mark and feedback visible to the student. |
