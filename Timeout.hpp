#include <cstdint>
#include <algorithm>
#include <cmath>

class Timeout {
  public:
    bool is_primed = false; // XXX dump hack

    explicit Timeout(int ticks_per_second)
        : ticks_per_second_(ticks_per_second > 0 ? ticks_per_second : 1) {}

    void start(int milliseconds) {
        expired_once_reported_ = false;
        is_primed = true;

        if (milliseconds <= 0) {
            remaining_ticks_ = 0;
            return;
        }

        // Round up so the timeout does not expire too early.
        remaining_ticks_ = (milliseconds * ticks_per_second_ + 999) / 1000;
        if (remaining_ticks_ < 0) {
            remaining_ticks_ = 0;
        }
    }

    void bump() {
        if (remaining_ticks_ > 0) {
            --remaining_ticks_;
        }
    }

    bool is_expired() const {
        return remaining_ticks_ == 0;
    }

    bool is_expired_once() {
        if (!is_expired()) {
            return false;
        }

        if (expired_once_reported_) {
            return false;
        }

        expired_once_reported_ = true;
        is_primed = false;
        return true;
    }

    int rem() const {
        if (remaining_ticks_ <= 0) {
            return 0;
        }

        // Round up to the next millisecond.
        return static_cast<int>((remaining_ticks_ * 1000LL + ticks_per_second_ - 1) / ticks_per_second_);
    }

  private:
    int ticks_per_second_ = 1;
    std::int64_t remaining_ticks_ = 0;
    bool expired_once_reported_ = false;
};
