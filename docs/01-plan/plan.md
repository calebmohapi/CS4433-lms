# Project Management Plan

# Feasibility Study

## Technical Feasibility

> Can C++17 with SQLite database, CMake and the rest of the tools deliver the require functionality on the school's existing hardware?

- C++17, SQLite and CMake do not have high hardware demands and have no licensing costs.
- SQLite is well-suited to old computers, unreliable internet and to offline operation. It is a single file serverless database.
- C++17 has a mature library therefore security concerns will be easily solvable.

> Yes, the system is technically feasible.

## Operational Feasibility

> Will all the users in their different roles be able to use the system day to day?

- A simple numbered menu driven user console is required to filter different types of group users. GUI frameworks could have steep learning curves.
- The user type sees only the menu items relevant to it to achieve role-based access.

> The system is operationally feasible as a simple-console.

## Economic Feasibility

> Can all the tools required to build the system be bought within the available financial budget?

- All tools are open-source or free for this scale of use.
- It requires about 200 hours of human [effort].

> The system is economically feasible at zero direct cost.

## Legal feasibility

> Can the system operate in regard to legal laws and follow ethical principles, data and privacy principles?

- The system schema stores only what is functionally needed.
- Access is role-restricted at the service layer.
- Every mark change is independently audited with tags of who, when, old value, new value and reason tags.
- No third-party data processors such as cloud services, payments, videos are involved in the system to reduce legal exposure.

> The system is legally feasible when implemented as specified.

## Schedule Feasibility

> Can the 7 weeks provided cover the full Waterfall Life Cycle and a change request cycle?

- The system testing shall start tests as soon as the first runnable code is produced not wait for the whole thing code to complete.
- The system design shall be strictly done in the 2 weeks allocated, to avoid overelaboration of designs such as UML.

> It is feasible but tight in terms of schedule.

## Recommendation

> The system is feasible across all five dimensions under stated constraints. The project scope shall be fixed after Requirements Engineering and not re-negotiated except through formal change-request process.

# Work Breakdown Structure

## Work Breakdown Structure Table (WBS Table)

| ID | Task | Phase | Duration | Dependency | Deliverable |
| --- | --- | --- | --- | --- | --- |
| 1.1 | Start Project | Planning | 1-Day | None | Repo Initialized |
| 1.2 | Feasibility Study | Planning | 1 Day | Task 1.1 | Feasibility study section |
| 1.3 | Work Breakdown and schedule | Planning | 0.5 Day | Task 1.2 | WBS and Gantt Chart |
| 1.4 | Risk Register | Planning | 0.5 Day | Task 1.2 | Risk Register |
| 1.5 | Review of Phase 1 | Planning | 1 Day | Task 1.1-1.4 | Phase 1 complete |
| 2.1 | Stakeholder Analysis | Requirements | 1 Day | Task 1.5 | Conflict Table |
| 2.2 | Functional and Non-functional Requirements Engineering | Requirements | 2 Days | Task 2.1 | SRS |
| 2.3 | Use case Diagram | Requirements | 2 Days | Task 2.2 | Use Case Model |
| 2.4 | SRS Validation and Review of requirements | Requirements | 0.5 Day | Task 2.2-2.3 | Gate 1 |
| 3.1 | Architecture specification | Design | 1 Day | Task 2.4 | Architecture section |
| 3.2 | Design patterns and Class diagram | Design | 2 Days | Task 3.1 | Class Diagram |
| 3.3 | Behavioural Models | Design | 2 Days | Task 3.2 | Behavioural Model |
| 3.4 | Database Design | Design | 1.5 Days | Task 3.2 | ERD and data dictionary |
| 3.5 | Review of Design | Design | 0.5 Day | Task 3.1-3.4 | Gate 2 |
| 4.1 | Build Skeleton for CMake and GoogleTest | Implementation | 0.5 Day | Task 3.5 | Buildable Skeleton |
| 4.2 | Data Layer: Database wrapper and repositories | Implementation | 1.5 Days | Task 4.1 | Data layer and tests |
| 4.3 | Domain Layer: User hierarchy and Penalty policy | Implementation | 2 Days | Task 4.1 | Domain layer and tests |
| 4.4 | Authentication system | Implementation | 1 Day | Task 4.2-4.3 | Authentication and tests |
| 4.5 | Admin Features (users, courses, enrolment) | Implementation | 1 Day | Task 4.2-4.4 | FR3 -FR4 |
| 4.6 | Add Lecturer features (assignments, extensions, marking) | Implementation | 1.5 Days | Task 4.3-4.4 | FR6-FR6, FR10 |
| 4.7 | Add Student features (submit, resubmit, view) | Implementation | 1.5 Days | Task 4.3-4.4 | FR7-FR9 |
| 4.8 | Add HoD features | Implementation | 1.5 Days | Task 4.3, 4.6 | FR11-FR13 |
| 4.9 | Implement Console UI | Implementation | 1 Day | Task 4.5-4.8 | User Interface |
| 5.1 | Formulate test strategy and design | Validation and Verification | 1 Day | Task 3.5 | Test plan |
| 5.2 | Perform unit and integration tests | Validation and Verification | 11 Days | Task 4.1 | Test code |
| 5.3 | Perform system test from use cases | Validation and Verification | 1.5 Days | Task 4.9 | System test results |
| 5.4 | Compile Defect log and perform fixes and retests | Validation and Verification | 1 Day | Task 5.3 | Defect log |
| 6.1 | Conduct static analysis run with fixes | Validation and Verification | 1 Day | Task 5.4 | QA report |
| 7.1 | Compile Install guide and user manuals | Deployment | 1 Days | Task 6.1 | Deploy pack v1.0 |
| 7.2 | Deploy the system | Deployment | | | |
| 8.1 | CR1 classification and impact analysis | Maintenance | 0.5 Day | Task 7.1 | CR1 analysis |
| 8.2 | CR1 implementation and design documentation update | Maintenance | 1.5 Day | Task 8.1 | CR1 code |
| 8.3 | CR1 regression testing | Maintenance | 0.5 Day | Task 8.2 | v1.1 |
| 9.1 | Legal and Ethics write up | Completion | 0.5 Day | Task 2.4 | Ethics section |
| 9.2 | Final Report compilation | Completion | 0.5 Day | Task 8.3 | Final PDF |
| 9.3 | Viva preparation | Completion | 0.5 Day | 9.2 | Script |

## Gantt Chart (Weeks)

| Task | W1 | W2 | W3 | W4 | W5 | W6 | W7 | Milestone |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1.1 Kickoff/repo | ● | | | | | | | |
| 1.2 Feasibility | ● | | | | | | | |
| 1.3 WBS/schedule | ● | | | | | | | |
| 1.4 Risk register v0 | ● | | | | | | | |
| 1.5 Technical spike | ● | | | | | | | |
| 1.6 Phase 1 review | ● | | | | | | | Phase 1 complete |
| 2.1 Stakeholder analysis | ● | ● | | | | | | |
| 2.2 SRS | | ● | | | | | | |
| 2.3 Use case model | | ● | | | | | | |
| 2.4 Gate 1 review | | ● | | | | | | Gate 1 |
| 3.1 Architecture | | ● | ● | | | | | |
| 3.2 Class diagram | | | ● | | | | | |
| 3.3 Behavioural models | | | ● | | | | | |
| 3.4 Database design | | | ● | | | | | |
| 3.5 Gate 2 review | | | ● | | | | | Gate 2 |
| 4.1 Build skeleton | | | | ● | | | | |
| 4.2 Data layer | | | | ● | | | | |
| 4.3 Domain layer | | | | ● | | | | |
| 4.4 Auth | | | | ● | | | | |
| 4.5 Admin features | | | | ● | ● | | | |
| 4.6 Lecturer features | | | | | ● | | | |
| 4.7 Student features | | | | | ● | | | |
| 4.8 HoD + audit | | | | | ● | | | |
| 4.9 UI assembly | | | | | ● | | | |
| 5.1 Test strategy | | | ● | ● | | | | |
| 5.2 Unit/integration tests | | | | ● | ● | | | |
| 5.3 System tests | | | | | ● | ● | | |
| 5.4 Defects/Gate 3 | | | | | | ● | | Gate 3 |
| 6.1 CM/QA | | | | | | ● | | |
| 7.1 Deployment / v1.0 | | | | | | ● | | v1.0 |
| 8.1 CR1 analysis | | | | | | | ● | |
| 8.2 CR1 implementation | | | | | | | ● | |
| 8.3 CR1 regression | | | | | | | ● | v1.1 |
| 9.1 Ethics write-up | ● | ● | ● | ● | ● | ● | | |
| 9.2 Report assembly | | | | | | | ● | |
| 9.3 Viva prep | | | | | | | ● | |

## Critical Path

> 1.1 → 1.2 → 1.6 → 2.1 → 2.2 → 2.3 → 2.4 (Gate 1) → 3.1 → 3.2 → 3.3 → 3.4 → 3.5 (Gate 2) → 4.1 → 4.2 → 4.3 → 4.4 → 4.8 → 4.9 → 5.3 → 5.4 (Gate 3) → 6.1 → 7.1 (v1.0) → 8.1 → 8.2 → 8.3 (v1.1) → 9.2 → 9.3

# Risk Register

| Risk | Category | Likelihood | Impact | Exposure | Mitigation | Owner |
| --- | --- | --- | --- | --- | --- | --- |
| Requirements change after Gate 1 | Project | Medium | High | High | Resolve all stakeholder conflicts before Gate 1 | Me |
| 7-week schedule is tight risking incomplete project | Project | High | High | High | The project scope must be held to MUST requirements only | Me |
| Due to single developer, device failure or lost work stalls could stall the whole project | People | Medium | High | High | Keep local and cloud backups of non-code artefacts and commit daily | Me |
| Chosen tools may underperform or have unbudgeted learning curve | Product | Low | Medium | Low | Confirming of all the tools and a minimal working example in week 1 | Me |
| Performance target may not be met on the actual hardware used | Technical | Medium | Medium | Medium | Seed 800 test students and benchmarking early not only at the end | Me |
| Report writing left until week 7 produces rushed inconsistent final report document | Project | Medium | High | High | Drafting of each report section upon its phase completion | Me |
