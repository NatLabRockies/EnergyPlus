#ifndef FP_EXCEPTION_GUARD_H_
#define FP_EXCEPTION_GUARD_H_

#include <cfenv>

namespace Penumbra {

// Host applications (e.g., EnergyPlus debug builds) may enable floating point traps.
// GLFW, the windowing system (Cocoa, X11, Wayland, Win32) and GL drivers routinely perform
// operations that raise FP exceptions, which then crash the host (SIGFPE on x86_64, SIGILL on
// arm64 macOS). This RAII guard saves the caller's floating point environment, masks all traps
// and clears the status flags for the duration of the scope, then restores the caller's
// environment exactly as it was (any flags raised inside the scope are discarded).
class FloatingPointExceptionGuard {
public:
  FloatingPointExceptionGuard() noexcept { std::feholdexcept(&saved_environment); }
  ~FloatingPointExceptionGuard() { std::fesetenv(&saved_environment); }

  FloatingPointExceptionGuard(const FloatingPointExceptionGuard &) = delete;
  FloatingPointExceptionGuard &operator=(const FloatingPointExceptionGuard &) = delete;
  FloatingPointExceptionGuard(FloatingPointExceptionGuard &&) = delete;
  FloatingPointExceptionGuard &operator=(FloatingPointExceptionGuard &&) = delete;

private:
  std::fenv_t saved_environment{};
};

} // namespace Penumbra

#endif // FP_EXCEPTION_GUARD_H_
