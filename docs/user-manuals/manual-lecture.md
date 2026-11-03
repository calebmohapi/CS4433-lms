# Appendix E — User Manuals

## Lecturer Manual

**Purpose:** Create assignments, mark submissions, grant extensions. You cannot re-mark a submission after marking unless the HoD returns it (BR4).

### Menu

1. My courses 6. Grant individual extension

2. List assignments 7. Extend deadline for whole class (CR1)

3. Create assignment 8. Change password

4. List submissions for assignment 0. Logout

5 Mark submission

### Key Tasks

| Task | Menu | Inputs | Notes |
| --- | --- | --- | --- |
| See my courses | `1` | — | Course ids needed for assignment creation |
| Create assignment | `3` | Course id, title, deadline `YYYY-MM-DD HH:MM:SS`, max mark | Assignment id printed |
| View submissions | `4` | Assignment id | Shows state and days-late |
| Mark a submission | `5` | Submission id, mark (pre-penalty), feedback | Penalty applied automatically; final mark printed |
| Individual extension | `6` | Assignment id, student id, new deadline, reason | Reason mandatory |
| Class-wide extension | `7` | Assignment id, new deadline, reason | Returns count of students affected |

### How the Late Penalty Is Applied

The mark you enter is **before penalty**. The system stores:

| Days late | Stored final mark |
| --- | --- |
| 0 | What you entered |
| 1 | Entered × 0.90 |
| 2 | Entered × 0.80 |
| 3 | Entered × 0.70 |
| > 3 (> 72 h) | 0 (submission preserved with feedback) |

### Operating Rules

- **Cannot re-mark** a submission unless the HoD returns it
- **Extensions must be after the original deadline** — the system rejects otherwise
- **Reason is mandatory** on every extension (audit trail)
- **Do not tell students their mark** before state reaches `Released`
