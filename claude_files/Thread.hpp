#pragma once
#include <thread>
#include <utility>  // for std::forward

// ============================================================
// Thread.hpp
// A RAII wrapper around std::thread.
//
// RAII means "Resource Acquisition Is Initialization" --
// the resource (an OS thread) is cleaned up automatically
// when this object goes out of scope. You never need to
// manually call join(). It happens in the destructor.
// ============================================================

class Thread {
public:

    // Default constructor -- creates a Thread object with NO
    // underlying OS thread running yet. Useful for storing
    // Thread objects in containers before assigning them work.
    // "= default" tells the compiler to generate this for us
    // even though we also wrote our own constructor below.
    Thread() = default;

    // Templated constructor -- spins up a REAL OS thread immediately.
    //
    // "typename Callable" means this works with ANY callable:
    //   a function pointer, a lambda, a functor (class with operator())
    //
    // "typename... Args" is a variadic pack -- zero or more arguments
    //   of any types. The "..." means "any number of".
    //
    // "Callable&& func" is a FORWARDING REFERENCE (not just rvalue).
    //   When combined with template deduction, && preserves whether
    //   the caller passed an lvalue or rvalue. This matters for move
    //   semantics -- we don't want to copy a lambda unnecessarily.
    //
    // "std::forward<Callable>(func)" is PERFECT FORWARDING.
    //   It passes func as exactly what it was when you handed it in.
    //   If you gave an lvalue, it forwards as lvalue. Rvalue as rvalue.
    //   This avoids unnecessary copies of your callable and its args.
    template<typename Callable, typename... Args>
    Thread(Callable&& func, Args&&... args) {
        m_thread = std::thread(
            std::forward<Callable>(func),
            std::forward<Args>(args)...  // the "..." expands the pack
        );
    }

    // Destructor -- called automatically when this object goes out of scope.
    // joinable() returns true if there is a real OS thread attached
    // that has not yet been joined or detached.
    // join() blocks here until that thread finishes, then cleans up.
    // Without this, destroying a joinable std::thread calls std::terminate()
    // which crashes your entire program.
    ~Thread() {
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }

    // Delete copy constructor and copy assignment.
    // You cannot copy a thread -- it represents one unique OS thread.
    // Trying to copy it would be nonsensical.
    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

    // Allow moving -- transferring ownership of the thread to another
    // Thread object is fine.
    Thread(Thread&&) = default;
    Thread& operator=(Thread&&) = default;

private:
    // The actual OS thread being managed.
    // Named m_thread (m_ prefix = member variable convention)
    // to avoid shadowing the class name "Thread".
    std::thread m_thread;
};
