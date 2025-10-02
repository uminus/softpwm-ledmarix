#include <Arduino.h>

enum TimerState {
    STOPPED,
    RUNNING,
    PAUSED,

    COMPLETED,
};


class PomodoroTimer {
    unsigned long started_at_ = 0L;
    TimerState state_ = STOPPED;
    unsigned long paused_at_ = 0L;
    unsigned long duration_ = 30000L;

    void completed() {
        state_ = COMPLETED;
    }

public:
    /**
     * Set the duration in millis.
     * Setting the duration resets the timer.
     *
     * @param duration duration in millis.
     */
    void setDurationMillis(const unsigned long duration) {
        stop();
        duration_ = duration;
    }

    [[nodiscard]] unsigned long duration() const {
        return duration_;
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
};
