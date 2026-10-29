#include "../src/JumpBuffer.hpp"
#include <cassert>
#include <cstdio>

using strongbuffer::JumpBuffer;

static strongbuffer::StepActions tick(JumpBuffer& b, int index, double dt,
                                     bool ground, bool allow = true,
                                     bool mode = true, double window = .1) {
    return b.afterPhysicsStep(index, dt, true, mode, ground, allow, window);
}

int main() {
    JumpBuffer b;
    // An early quick tap fires only on landing; release after a full step.
    b.observeInput(0, true, true);
    b.observeInput(0, false, false);
    assert(!tick(b, 0, .02, false).press);
    assert(tick(b, 0, .02, true).press);
    auto s = tick(b, 0, .004, true);
    assert(s.release && !s.press);
    assert(!tick(b, 0, .004, true).press);

    // Holding an original press should not result in an extra jump.
    b.reset();
    b.observeInput(0, true, true);
    assert(!tick(b, 0, .02, true).press);
    b.observeInput(0, false, false);
    assert(!tick(b, 0, .02, true).press);

    // Expired taps never replay.
    b.reset();
    b.observeInput(0, true, true);
    b.observeInput(0, false, false);
    assert(!tick(b, 0, .12, true).press);

    // A valid ring triggers a rescue; a disabled orb does not.
    b.reset();
    b.observeInput(0, true, true);
    b.observeInput(0, false, false);
    assert(tick(b, 0, .01, true, true).press);
    b.reset();
    b.observeInput(0, true, true);
    b.observeInput(0, false, false);
    assert(!tick(b, 0, .01, true, false).press);

    // Flying mode cancels the jump buffer.
    b.reset();
    b.observeInput(0, true, true);
    b.observeInput(0, false, false);
    assert(!tick(b, 0, .01, false, true, false).press);
    assert(!tick(b, 0, .01, true).press);

    // Player states never contaminate one another.
    b.reset();
    b.observeInput(1, true, true);
    b.observeInput(1, false, false);
    assert(!tick(b, 0, .02, true).press);
    assert(tick(b, 1, .02, true).press);

    // Death/restart clears waiting taps and synthetic pulses.
    b.reset();
    b.observeInput(0, true, true);
    b.observeInput(0, false, false);
    b.reset();
    assert(!tick(b, 0, .01, true).press);

    // Second real press cancels ownership of synthetic pulse.
    b.reset();
    b.observeInput(0, true, true);
    b.observeInput(0, false, false);
    assert(tick(b, 0, .01, true).press);
    b.observeInput(0, true, false);
    assert(!tick(b, 0, .01, true).release);

    std::puts("All StrongInputBuffer state machine tests passed.");
}
