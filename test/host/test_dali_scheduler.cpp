#include <cstdlib>
#include <optional>

#include "dali/scheduler/dali_scheduler.h"
#include "test_support.h"

int main() {
    using namespace dali::scheduler;

    DaliScheduler scheduler;
    LightPollContext on{};
    on.commandedOn = true;
    scheduler.configure(0U, on, 0U);

    const auto first = scheduler.next(0U, 0.0F, 0.30F, 0.60F);
    CHECK_TRUE(first.has_value());
    CHECK_EQ(first->address, 0U);
    CHECK_EQ(first->kind, PollKind::Status);
    scheduler.complete(*first, 0U);
    CHECK_TRUE(!scheduler.next(4999U, 0.0F, 0.30F, 0.60F).has_value());
    CHECK_TRUE(scheduler.next(5000U, 0.0F, 0.30F, 0.60F).has_value());

    LightPollContext off{};
    DaliScheduler offScheduler;
    offScheduler.configure(1U, off, 0U);
    const auto offFirst = offScheduler.next(0U, 0.0F, 0.30F, 0.60F);
    CHECK_TRUE(offFirst.has_value());
    CHECK_EQ(offFirst->address, 1U);
    offScheduler.complete(*offFirst, 0U);
    CHECK_TRUE(!offScheduler.next(29999U, 0.0F, 0.30F, 0.60F).has_value());

    LightPollContext warning{};
    warning.commandedOn = true;
    warning.warningActive = true;
    warning.observationStartedMs = 0U;
    scheduler.configure(2U, warning, 0U);
    const auto warningFirst = scheduler.next(0U, 0.0F, 0.30F, 0.60F);
    CHECK_TRUE(warningFirst.has_value());
    scheduler.complete(*warningFirst, 0U);
    CHECK_TRUE(scheduler.next(1999U, 0.0F, 0.30F, 0.60F).has_value() == false);
    CHECK_TRUE(scheduler.next(2000U, 0.0F, 0.30F, 0.60F).has_value());

    PollRequest normal{3U, PollKind::Electrical, PollPriority::Normal, 100U};
    CHECK_TRUE(scheduler.admit(normal, 0.29F, 0.30F, 0.60F));
    CHECK_TRUE(!scheduler.admit(normal, 0.30F, 0.30F, 0.60F));
    const PollRequest critical{3U, PollKind::Status, PollPriority::Critical, 100U};
    CHECK_TRUE(scheduler.admit(critical, 0.59F, 0.30F, 0.60F));
    CHECK_TRUE(!scheduler.admit(critical, 0.60F, 0.30F, 0.60F));
    return EXIT_SUCCESS;
}
