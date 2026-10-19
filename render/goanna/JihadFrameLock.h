/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#ifndef JIHAD_FRAME_LOCK_H
#define JIHAD_FRAME_LOCK_H

// T-515, frames made off the main thread: the daemon's frame lock.
//
// WHY. The daemon made every frame on its main thread, the engine's, so a page's script that ran
// for a second (desktop Wikipedia restyling after a scroll, a zoom's resize handlers) held every
// frame for that second: the card's pan and zoom waited with it (cavekit-gpu-rendering R3, R6).
// The compositor composes the layer tree on its own thread already; what kept the frame on the
// main thread was the daemon's own bookkeeping (the buffers, the slots, the publish). With the
// express lane (BrowserPageGoanna.cpp) a second thread composes and publishes frames from the
// layer tree as it stands while the main thread is busy in the engine.
//
// THE LOCK. One per daemon, around every part of that bookkeeping. The main thread holds it for
// the whole of its tick and of every card command, and lets go of it only where it runs the
// engine for long: the pump (the page's script, timers, network events) and the layer paint of a
// frame. The express lane, the YAP reader threads (the card's scroll, zoom and returned buffers)
// and the inject pan take it for their short steps. Re-entrant on the thread that holds it, so a
// function that needs it takes it whether or not its caller already did; release() lets go of all
// the holds at once and reacquire() takes them back, so a release deep in a call chain works.
//
// THE WANT LOCK. The YAP reader thread never waits for the frame lock: a proxy's teardown joins
// its reader, and the main thread can tear one down holding the frame lock (a card that leaves
// while a command's nested loop runs). The card's newest scroll and zoom (and the page that a
// proxy's commands go to) are under a lock of their own, held only for a few assignments; a
// returned buffer takes the frame lock with a bound (tryLockFor) and is left to the main loop
// when it is busy. Order: the frame lock, then the want lock.
//
// Off (every call a no-op) unless the express lane is on (jihadExpressOn): with it off the daemon
// is single-threaded as it always was.

#include <atomic>
#include <cstdint>
#include <mutex>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

namespace jihad {

bool jihadExpressOn();   // BrowserPageGoanna.cpp: JIHAD_GPU_EXPRESS, the GPU path only
void jihadExpressStop(); // BrowserPageGoanna.cpp: the express thread ended (the daemon's exit; the frame lock NOT held)

class FrameLock {
public:
  void lock() {
    if (owned()) { ++mDepth; return; }
    mMutex.lock();
    mOwner.store(pthread_self()); mHeld.store(true);
    mDepth = 1;
  }
  void unlock() {
    if (--mDepth == 0) { mHeld.store(false); mMutex.unlock(); }
  }
  // lock() with a bound: false (not held) when another thread kept it for aMs
  bool tryLockFor(int aMs) {
    if (owned()) { ++mDepth; return true; }
    const int64_t until = nowMs() + aMs;
    for (;;) {
      if (mMutex.try_lock()) {
        mOwner.store(pthread_self()); mHeld.store(true);
        mDepth = 1;
        return true;
      }
      if (nowMs() >= until) return false;
      usleep(300);
    }
  }
  // all of this thread's holds let go of; the count to give reacquire(). aAway: the main thread
  // goes into the engine (the pump, a layer paint) and is away from its frames until it is back.
  int release(bool aAway = false) {
    if (!owned()) return 0;
    const int d = mDepth;
    if (aAway && !mAwayMs.load()) mAwayMs.store(nowMs());
    mDepth = 0; mHeld.store(false);
    mMutex.unlock();
    return d;
  }
  // aAwayBefore: the away mark as release() found it (a release nested in an outer one's engine
  // call, a layer paint from a command the pump's nested loop ran, leaves the outer one's mark)
  void reacquire(int depth, int64_t aAwayBefore = 0) {
    if (depth <= 0) return;
    mMutex.lock();
    mOwner.store(pthread_self()); mHeld.store(true);
    mDepth = depth;
    mAwayMs.store(aAwayBefore);
  }
  // since when the main thread has been in the engine with the lock let go of (0: it is not)
  int64_t awaySinceMs() const { return mAwayMs.load(); }
  static int64_t nowMs() {
    struct timeval tv; gettimeofday(&tv, nullptr);
    return (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
  }
  bool owned() const { return mHeld.load() && pthread_equal(mOwner.load(), pthread_self()); }

private:
  std::mutex mMutex;
  std::atomic<pthread_t> mOwner{};
  std::atomic<bool> mHeld{false};
  std::atomic<int64_t> mAwayMs{0};
  int mDepth = 0;   // the owner's holds; only the owner reads or writes it
};

inline FrameLock& jihadFrameLock() {
  static FrameLock sLock;
  return sLock;
}

inline std::mutex& jihadWantLock() {
  static std::mutex sLock;
  return sLock;
}

// The lock for a scope, when the express lane is on.
class FrameLockGuard {
public:
  FrameLockGuard() : mOn(jihadExpressOn()) { if (mOn) jihadFrameLock().lock(); }
  ~FrameLockGuard() { if (mOn) jihadFrameLock().unlock(); }
  FrameLockGuard(const FrameLockGuard&) = delete;
  FrameLockGuard& operator=(const FrameLockGuard&) = delete;
private:
  bool mOn;
};

// The lock let go of for a scope (the main thread's pump or layer paint: it is away from its
// frames until the scope ends) and taken back after.
class FrameLockRelease {
public:
  FrameLockRelease()
    : mAwayBefore(jihadExpressOn() ? jihadFrameLock().awaySinceMs() : 0),
      mDepth(jihadExpressOn() ? jihadFrameLock().release(true) : 0) {}
  ~FrameLockRelease() { jihadFrameLock().reacquire(mDepth, mAwayBefore); }
  FrameLockRelease(const FrameLockRelease&) = delete;
  FrameLockRelease& operator=(const FrameLockRelease&) = delete;
private:
  int64_t mAwayBefore;
  int mDepth;
};

}  // namespace jihad

#endif  // JIHAD_FRAME_LOCK_H
