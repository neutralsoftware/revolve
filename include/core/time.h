#ifndef REVOLVE_TIME
#define REVOLVE_TIME

#include <cstdint>
#include <functional>
#include <queue>
#include <string>
#include <vector>

using tick = uint64_t;

constexpr tick BROADWAY_CLOCK = 729000000ull; // 729 MHz

struct Event {
    tick time;
    uint64_t id;
    std::function<void()> callback;
    std::string name;

    bool operator>(const Event &other) const {
        if (time != other.time)
            return time > other.time;

        return id > other.id;
    }
};

class Scheduler {
  public:
    tick now() const { return currentTime; }
    tick ticksUntilNextEvent() const {
        if (eventQueue.empty() || eventQueue.top().time <= currentTime)
            return 0;
        return eventQueue.top().time - currentTime;
    }

    void advance(tick ticks);

    void schedule(std::string name, std::function<void()> callback,
                  tick ticksFromNow) {
        eventQueue.push({currentTime + ticksFromNow, nextEventId++,
                         std::move(callback), std::move(name)});
    }

  private:
    tick currentTime = 0;
    uint64_t nextEventId = 0;

    std::priority_queue<Event, std::vector<Event>, std::greater<Event>>
        eventQueue;
};

#endif
