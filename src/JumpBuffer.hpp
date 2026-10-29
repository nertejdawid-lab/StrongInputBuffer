#pragma once

#include <array>
#include <cstddef>

// This file deliberately has no Geode dependencies. The input state machine can
// be tested without Geometry Dash, and the Geode hooks only provide observations
// (ground/ring/mode) and carry out its press/release instructions.
namespace strongbuffer {

struct StepActions {
    bool press = false;
    bool release = false;
};

struct Slot {
    bool physicallyHeld = false;
    bool waiting = false;
    bool syntheticPress = false;
    double waitedSeconds = 0.0;
};

class JumpBuffer {
public:
    void reset() { m_slots = {}; }

    void resetPlayer(std::size_t i) {
        if (i < m_slots.size()) m_slots[i] = {};
    }

    // The original input always goes to GD. This only records a possible
    // EXTRA jump to be played when a short *released* tap was too early.
    void observeInput(std::size_t i, bool down, bool queueCandidate) {
        if (i >= m_slots.size()) return;
        auto& s = m_slots[i];
        const bool alreadyHeld = s.physicallyHeld;
        s.physicallyHeld = down;

        if (down && !alreadyHeld) {
            // If a replay is in progress, the real press takes ownership.
            s.syntheticPress = false;
            s.waiting = queueCandidate;
            s.waitedSeconds = 0.0;
        }
    }

    // Call ONCE after each GD physics command step. The press is performed
    // after the GD step; its release occurs after the FOLLOWING GD step.
    StepActions afterPhysicsStep(
        std::size_t i,
        double deltaSeconds,
        bool enabled,
        bool validMode,
        bool reachedSurfaceOrRing,
        bool isAllowedTrigger,
        double windowSeconds
    ) {
        if (i >= m_slots.size()) return {};
        auto& s = m_slots[i];
        StepActions out;

        // Ensure the synthetic press is seen by GD for a whole physics step.
        if (s.syntheticPress) {
            if (!s.physicallyHeld) out.release = true;
            s.syntheticPress = false;
        }

        if (!enabled || !validMode || windowSeconds <= 0.0) {
            s.waiting = false;
            return out;
        }
        if (!s.waiting) return out;

        // `processCommands` supplies physics time. Reject invalid deltas
        // instead of allowing time to go backwards or produce a huge expiry.
        if (deltaSeconds > 0.0 && deltaSeconds < 0.5) {
            s.waitedSeconds += deltaSeconds;
        }

        if (s.waitedSeconds > windowSeconds) {
            s.waiting = false;
            return out;
        }
        if (!reachedSurfaceOrRing) return out;

        // A held original input should behave exactly like vanilla GD.
        // Only a missed, already released tap needs a rescue pulse.
        s.waiting = false;
        if (!s.physicallyHeld && isAllowedTrigger) {
            out.press = true;
            s.syntheticPress = true;
        }
        return out;
    }

    Slot const& slot(std::size_t i) const { return m_slots.at(i); }

private:
    std::array<Slot, 2> m_slots{};
};

} // namespace strongbuffer
