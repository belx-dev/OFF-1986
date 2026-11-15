#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/Core/Game/FixedTick.hpp>

using Catch::Approx;
using Poseidon::FixedTickAccumulator;

namespace
{

int RunTicks(FixedTickAccumulator& acc)
{
    int n = 0;
    while (acc.ConsumeTick())
        ++n;
    return n;
}

} // namespace

TEST_CASE("FixedTickAccumulator: hz 0 keeps the variable step", "[core][fixedtick]")
{
    FixedTickAccumulator acc;
    acc.Configure(0);
    REQUIRE_FALSE(acc.Enabled());
    acc.Add(1.0f, 1.0f);
    REQUIRE_FALSE(acc.ConsumeTick());
    REQUIRE(acc.TickCount() == 0);
}

TEST_CASE("FixedTickAccumulator: whole ticks per frame, remainder carried", "[core][fixedtick]")
{
    FixedTickAccumulator acc;
    acc.Configure(30);
    REQUIRE(acc.Quantum() == Approx(1.0f / 30.0f));

    // 200 FPS presentation: 5 ms frames, one tick about every 6.7 frames.
    int ticks = 0;
    for (int frame = 0; frame < 200; ++frame)
    {
        acc.Add(0.005f, 1.0f);
        ticks += RunTicks(acc);
    }
    REQUIRE(ticks == 30);
    REQUIRE(acc.TickCount() == 30);
    REQUIRE(acc.Backlog() < acc.Quantum());

    // One long 0.1 s frame yields three ticks at once.
    acc.Reset();
    acc.Add(0.1f, 1.0f);
    REQUIRE(acc.TickDue());
    REQUIRE(acc.TicksDue() == 3);
    REQUIRE(RunTicks(acc) == 3);
    REQUIRE_FALSE(acc.TickDue());
    REQUIRE(acc.TicksDue() == 0);
}

TEST_CASE("FixedTickAccumulator: acceleration runs more ticks of the same size", "[core][fixedtick]")
{
    FixedTickAccumulator acc;
    acc.Configure(30);
    int ticks = 0;
    for (int frame = 0; frame < 60; ++frame) // one wall second at 60 FPS
    {
        acc.Add((1.0f / 60.0f) * 4.0f, 4.0f);
        ticks += RunTicks(acc);
    }
    REQUIRE(ticks == 120);
    REQUIRE(acc.Quantum() == Approx(1.0f / 30.0f));
}

TEST_CASE("FixedTickAccumulator: catch-up budget drops the excess", "[core][fixedtick]")
{
    FixedTickAccumulator acc;
    acc.Configure(30, 0.3f);

    acc.Add(5.0f, 1.0f); // a 5 s stall
    REQUIRE(acc.Backlog() == Approx(0.3f));
    REQUIRE(RunTicks(acc) == 9); // floor(0.3 / (1/30))

    acc.Reset();
    acc.Add(5.0f, 4.0f); // budget scales with acceleration
    REQUIRE(acc.Backlog() == Approx(1.2f));
}

TEST_CASE("FixedTickAccumulator: reset clears the backlog, not the counter", "[core][fixedtick]")
{
    FixedTickAccumulator acc;
    acc.Configure(30);
    acc.Add(0.1f, 1.0f);
    REQUIRE(RunTicks(acc) == 3);
    acc.Add(0.02f, 1.0f);
    acc.Reset();
    REQUIRE(acc.Backlog() == 0.0f);
    REQUIRE_FALSE(acc.ConsumeTick());
    REQUIRE(acc.TickCount() == 3);
}

TEST_CASE("FixedTickAccumulator: time until the next tick", "[core][fixedtick]")
{
    FixedTickAccumulator acc;
    acc.Configure(40); // 25 ms
    REQUIRE(acc.SecondsUntilNextTick(1.0f) == Approx(0.025f));
    acc.Add(0.015f, 1.0f);
    REQUIRE(acc.SecondsUntilNextTick(1.0f) == Approx(0.010f));
    REQUIRE(acc.SecondsUntilNextTick(2.0f) == Approx(0.005f));
    acc.Add(0.010f, 1.0f);
    REQUIRE(acc.SecondsUntilNextTick(1.0f) == Approx(0.0f).margin(1e-6f));
}
