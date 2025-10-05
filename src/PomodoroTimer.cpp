#include <Arduino.h>

enum TimerState {
    STOPPED,
    RUNNING,
    PAUSED,

    COMPLETED,
};

/**
 *  Pomodoro phases
 *
 * - WORK: 25 min.
 * - BREAK: 5 min.
 */
enum Phase: unsigned long {
#ifdef DEBUG
    WORK = 5L * 1000L,
    BREAK = 3L * 1000,
#else
    WORK = 25L * 60L * 1000L,
    BREAK = 5L * 60 * 1000,
#endif
};


class PomodoroTimer {
    unsigned long started_at_ = 0L;
    TimerState state_ = STOPPED;
    Phase phase_ = WORK;
    unsigned long paused_at_ = 0L;
    unsigned long duration_ = WORK;

    void completed() {
        state_ = COMPLETED;

        // switch to next phase and apply duration
        switch (phase_) {
            case WORK:
                phase_ = BREAK;
                break;
            case BREAK:
                phase_ = WORK;
        }
        setDurationMillis(phase_, false);
    }

public:
    /**
     * Set the duration in millis.
     * Setting the duration resets the timer.
     *
     * @param duration duration in millis.
     */
    void setDurationMillis(const unsigned long duration, const bool reset = true) {
        if (reset) {
            stop();
        }
        duration_ = duration;
    }

    void stop() {
        started_at_ = 0L;
        state_ = STOPPED;
    }

    void start() {
        started_at_ = millis();
        state_ = RUNNING;
    }

    void pause() {
        if (state_ != RUNNING) {
            return;
        }
        paused_at_ = millis();
        state_ = PAUSED;
    }

    void resume() {
        if (state_ != PAUSED) {
        }
        started_at_ = millis() - (paused_at_ - started_at_);
        paused_at_ = 0L;
        state_ = RUNNING;
    }


    /**
     * @return elapsed millis
     */
    [[nodiscard]] unsigned long tick() {
        // TODO タイマー終了時の処理

        switch (state_) {
            case STOPPED:
                return duration_;
            case RUNNING: {
                const auto elapsed = millis() - started_at_;
                if (duration_ <= elapsed) {
                    completed();
                    return 0L;
                }
                return duration_ - elapsed;
            }
            case PAUSED:
                return duration_ - (paused_at_ - started_at_);
            default:
                return 0L;
        }
    }

    [[nodiscard]] TimerState state() const {
        return state_;
    }

    [[nodiscard]] Phase phase() const {
        return phase_;
    }
};
