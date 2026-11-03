# Appendix E — User Manuals

## Head of Department Manual

**Purpose:** Moderate and release marks in your own department. Other departments' marks are inaccessible (FR11).

### Moderation Workflow

Marked → (Approve / Adjust / Return) → Moderated → Release → Released

Approve, adjust, and return are one step. **Release is a separate step** (BR6) so marks can be batched.

### Menu

1\. List MARKED submissions in my dept
2\. Approve mark (unchanged)
3\. Adjust mark
4\. Return to lecturer 8. Change password
5\. List MODERATED submissions
6\. Release mark
7\. View audit log
0\. Logout

### Key Tasks

| Task | Menu | Inputs | Effect |
| --- | --- | --- | --- |
| Review queue | `1` | — | Lists MARKED submissions in your dept |
| Approve | `2` | Submission id | State → Moderated, mark unchanged |
| Adjust | `3` | Submission id, new mark, **reason** | State → Moderated, **audit entry written** |
| Return | `4` | Submission id, comment | State → Returned; lecturer must re-mark |
| Release | `6` | Submission id | State → Released; student sees mark and feedback |

### Reading the Audit Log

Option `7` for any submission shows every mark change:

### Key Tasks

| Task | Menu | Inputs | Effect |
| --- | --- | --- | --- |
| Review queue | `1` | — | Lists MARKED submissions in your dept |
| Approve | `2` | Submission id | State → Moderated, mark unchanged |
| Adjust | `3` | Submission id, new mark, **reason** | State → Moderated, **audit entry written** |
| Return | `4` | Submission id, comment | State → Returned; lecturer must re-mark |
| Release | `6` | Submission id | State → Released; student sees mark and feedback |

### Reading the Audit Log

Option `7` for any submission shows every mark change:

[2026-11-01 14:22] by #3 old=0 new=68 reason=Marked by lecturer
[2026-11-02 09:15] by #2 old=68 new=78 reason=Moderated: Late penalty too harsh

### Operating Rules

- **Never approve without reading the feedback** — feedback is part of the moderation
- **Adjust requires a written reason** — enforced by the system
- **When in doubt, return** — the lecturer has the submission context
- **Release in batches** — approve all, then one release session
