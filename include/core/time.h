#ifndef REVOLVE_TIME
#define REVOLVE_TIME

#include <cstdint>
#include <functional>
#include <queue>
#include <string>
#include <vector>

using tick = uint64_t;

constexpr tick BROADWAY_CLOCK = 72900000; // 72.9 MHz

struct Event {
    uint64_t time;
    std::function<void()> callback;
    std::string name;

    bool operator<(const Event &other) const { return time < other.time; }
    bool operator>(const Event &other) const { return time > other.time; }
};

class Scheduler {
  public:
    tick now() const { return currentTime; }

    void advance(tick ticks);

    inline void schedule(std::string name, std::function<void()> callback,
                         tick ticksFromNow) {
        eventQueue.push({currentTime + ticksFromNow, callback, name});
    }

  private:
    tick currentTime = 0;

    std::priority_queue<Event, std::vector<Event>, std::greater<Event>>
        eventQueue;
};

#endif