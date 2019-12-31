/* Programme context.
 *
 * Eldritchd has a few things that it needs to keep track of, which is
 * is implemented in this file. It should be possible to run multiple instaces
 * of eldritchd in the same process, although not entirely sure why one would
 * do that.
 *
 * See also:
 * * Documentation: https://ef.gy/documentation/eldritchd
 * * Source Code: https://github.com/ef-gy/eldritchd
 * * Licence Terms: https://github.com/ef-gy/eldritchd/COPYING
 *
 * @copyright
 * This file is part of eldritchd, which is released as open source under the
 * terms of an MIT/X11-style licence, described in the COPYING file.
 */

#if !defined(ELDRITCHD_CONTEXT_H)
#define ELDRITCHD_CONTEXT_H

#define _BSD_SOURCE

#include <cxxhttp/network.h>
#include <ef.gy/global.h>

#include <sys/types.h>
#include <unistd.h>

namespace eldritchd {
template <typename service, typename process>
class context {
 public:
  context(service &pService = efgy::global<service>(),
          efgy::beacons<process> &pProcesses =
              efgy::global<efgy::beacons<process>>())
      : service_(pService),
        processes_(pProcesses),
        signals_(service_, SIGCHLD) {}

  /* Get reference to full list of processes.
   *
   * This should only be used to construct beacons for new processes.
   *
   * @returns `this->processes_`.
   */
  efgy::beacons<process> &processes(void) const { return processes_; }

  void update(void) {
    for (auto &p : processes_) {
      p->update();
    }
  }

  void run(void) {
    for (auto &p : processes_) {
      if (p->status() != process::running) {
        (*p)();
      }
    }

    signals_.async_wait([this](const asio::error_code &, int) { sigchld(); });
  }

  /* Call libasio main loop.
   *
   * Calls service_.run(), which is basically libasio's main loop. This won't
   * usually return, unless there's absolutely nothing to do and no daemons to
   * take care of.
   */
  void loop(void) { service_.run(); }

  /* `fork()` syscall wrapper.
   *
   * Ensures that libasio is aware of the `fork()` call and otherwise just calls
   * that function.
   *
   * @returns Same as `fork()`.
   */
  int fork(void) {
    pid_t pid;

    service_.notify_fork(asio::io_service::fork_prepare);
    switch (pid = ::fork()) {
      case -1:
        break;
      case 0:
        service_.notify_fork(asio::io_service::fork_child);
        break;
      default:
        service_.notify_fork(asio::io_service::fork_parent);
        break;
    }

    return pid;
  }

  bool haveTasks(void) const { return processes_.size() > 0; }

 protected:
  /* libasio I/O service.
   *
   * We use this instead of low-level primitives because we also integrate with
   * cxxhttp, which is based on libasio. The I/O service has functions for
   * dealing with signals, so this is quite convenient.
   */
  service &service_;
  efgy::beacons<process> &processes_;
  asio::signal_set signals_;

  /* SIGCHLD handler.
   *
   * We want our processes to be running continuously, so respawn them if
   * they die. If spawning them fails this would put them in an unmonitored
   * state.
   *
   * We use one SIGCHLD handler for all processes and just tell the process
   * class to check all of them. This might do extra syscalls, but skipping a
   * handler won't cause issues this way.
   */
  void sigchld(void) {
    update();
    run();
  }
};
}  // namespace eldritchd

#endif
