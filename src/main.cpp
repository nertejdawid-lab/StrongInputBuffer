#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <cstdint>

#include "JumpBuffer.hpp"

using namespace geode::prelude;

namespace {
    // A level reset, checkpoint load or death invalidates all buffered taps.
    std::uint64_t g_resetEpoch = 1;

    bool enabled() {
        return Mod::get()->getSettingValue<bool>("enabled");
    }

    bool orbBuffer() {
        return Mod::get()->getSettingValue<bool>("orb-buffer");
    }

    double windowSeconds() {
        auto ms = Mod::get()->getSettingValue<int64_t>("window-ms");
        if (ms < 0) ms = 0;
        if (ms > 250) ms = 250;
        return static_cast<double>(ms) / 1000.0;
    }

    bool supportsLandingBuffer(PlayerObject* player) {
        // No predictive jump buffer for continuously controlled modes.
        return player && !player->m_isShip && !player->m_isBird &&
               !player->m_isDart && !player->m_isSwing;
    }

    bool isTouchingRing(PlayerObject* player) {
        return player && player->m_touchingRings &&
               player->m_touchingRings->count() > 0;
    }
}

class $modify(StrongInputBufferLayer, GJBaseGameLayer) {
    struct Fields {
        strongbuffer::JumpBuffer buffer;
        std::uint64_t seenEpoch = 0;
    };

    void syncReset() {
        auto* f = m_fields.operator->();
        if (f->seenEpoch != g_resetEpoch) {
            f->buffer.reset();
            f->seenEpoch = g_resetEpoch;
        }
    }

    bool isThisPlayLayer() {
        auto* pl = PlayLayer::get();
        return pl && static_cast<GJBaseGameLayer*>(pl) == static_cast<GJBaseGameLayer*>(this);
    }

    bool sharedDualInput() {
        // In normal one-player dual, a single input controls both icons.
        return m_gameState.m_isDualMode && m_levelSettings &&
               !m_levelSettings->m_twoPlayerMode;
    }

    void observePlayer(std::size_t index, bool down) {
        auto* player = index == 0 ? m_player1 : m_player2;
        if (!player) return;
        const bool canQueue = enabled() && windowSeconds() > 0.0 &&
            supportsLandingBuffer(player) && !player->m_isOnGround &&
            !isTouchingRing(player);
        m_fields->buffer.observeInput(index, down, canQueue);
    }

    void handleButton(bool down, int button, bool isPlayer1) {
        if (button == 1 && isThisPlayLayer()) {
            syncReset();
            if (m_levelSettings && m_levelSettings->m_twoPlayerMode) {
                observePlayer(isPlayer1 ? 0 : 1, down);
            } else {
                // GD treats P2 input as P1 outside real 2-player mode.
                observePlayer(0, down);
                if (sharedDualInput()) observePlayer(1, down);
            }
        }

        // IMPORTANT: never swallow the original input. This preserves vanilla
        // orbs, holds, clicks, dual mode and other input-related mods.
        GJBaseGameLayer::handleButton(down, button, isPlayer1);
    }

    void tickPlayer(std::size_t index, double dt) {
        auto* player = index == 0 ? m_player1 : m_player2;
        if (!player) {
            m_fields->buffer.resetPlayer(index);
            return;
        }

        const bool ring = isTouchingRing(player);
        const bool touching = player->m_isOnGround || ring;
        const bool allowed = player->m_isOnGround || (ring && orbBuffer());
        auto actions = m_fields->buffer.afterPhysicsStep(
            index, dt, enabled(), supportsLandingBuffer(player),
            touching, allowed, windowSeconds()
        );

        if (actions.release) player->releaseButton(PlayerButton::Jump);
        if (actions.press) player->pushButton(PlayerButton::Jump);
    }

    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        syncReset();
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
        syncReset();

        if (!isThisPlayLayer()) return;
        tickPlayer(0, dt);
        tickPlayer(1, dt);
    }
};

class $modify(StrongInputBufferReset, PlayLayer) {
    void resetLevel() {
        ++g_resetEpoch;
        PlayLayer::resetLevel();
    }

    void resetLevelFromStart() {
        ++g_resetEpoch;
        PlayLayer::resetLevelFromStart();
    }

    void loadFromCheckpoint(CheckpointObject* checkpoint) {
        ++g_resetEpoch;
        PlayLayer::loadFromCheckpoint(checkpoint);
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        ++g_resetEpoch;
        PlayLayer::destroyPlayer(player, object);
    }
};

$on_mod(Loaded) {
    log::info("Strong Input Buffer loaded - jump/orb buffer with P1/P2 support");
}
