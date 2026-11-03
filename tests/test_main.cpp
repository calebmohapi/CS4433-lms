#include "domain.h"
#include "util.h"
#include <iostream>
#include <cassert>

static int failures = 0;
#define CHECK(cond)                                                     \
    do                                                                  \
    {                                                                   \
        if (!(cond))                                                    \
        {                                                               \
            std::cerr << "FAIL: " #cond " @ line " << __LINE__ << "\n"; \
            failures++;                                                 \
        }                                                               \
    } while (0)

static void testPenaltyPolicy()
{
    TenPercentPerDayPolicy p;
    CHECK(p.compute(100.0, 0) == 0.0);
    CHECK(p.compute(100.0, 1) == 10.0);
    CHECK(p.compute(100.0, 2) == 20.0);
    CHECK(p.compute(100.0, 3) == 30.0);
    CHECK(p.compute(100.0, 4) == 100.0); // >72h = zero
}

static void testStateMachine()
{
    Submission s;
    s.state = SubmissionState::SUBMITTED;
    CHECK(s.canTransitionTo(SubmissionState::MARKED));
    CHECK(s.canTransitionTo(SubmissionState::SUBMITTED));
    CHECK(s.canTransitionTo(SubmissionState::LATE));
    CHECK(!s.canTransitionTo(SubmissionState::RELEASED));

    s.state = SubmissionState::LATE;
    CHECK(s.canTransitionTo(SubmissionState::MARKED));
    CHECK(!s.canTransitionTo(SubmissionState::SUBMITTED));

    s.state = SubmissionState::MARKED;
    CHECK(s.canTransitionTo(SubmissionState::MODERATED));
    CHECK(s.canTransitionTo(SubmissionState::RETURNED));
    CHECK(!s.canTransitionTo(SubmissionState::RELEASED));

    s.state = SubmissionState::MODERATED;
    CHECK(s.canTransitionTo(SubmissionState::RELEASED));

    s.state = SubmissionState::RELEASED;
    CHECK(!s.canTransitionTo(SubmissionState::MARKED));
}

static void testPasswordHash()
{
    std::string h = util::hashPassword("hunter2");
    CHECK(util::verifyPassword("hunter2", h));
    CHECK(!util::verifyPassword("wrong", h));
    CHECK(h.find("hunter2") == std::string::npos); // never plaintext
}

static void testTimeHelpers()
{
    std::string t = "2024-03-01 12:00:00";
    int64_t e = util::parseIsoToEpoch(t);
    CHECK(e != 0);
    CHECK(util::addHours(t, 24) == "2024-03-02 12:00:00");
    CHECK(util::addHours(t, -12) == "2024-03-01 00:00:00");
}

int main()
{
    testPenaltyPolicy();
    testStateMachine();
    testPasswordHash();
    testTimeHelpers();
    if (failures == 0)
    {
        std::cout << "All tests passed.\n";
        return 0;
    }
    std::cerr << failures << " test(s) failed.\n";
    return 1;
}