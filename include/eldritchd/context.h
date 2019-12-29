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

#include <cxxhttp/network.h>
#include <ef.gy/global.h>

namespace eldritchd {
template <typename tService, typename tProcess>
class context {
 public:
  tService &service;
  efgy::beacons<tProcess> &processes;

  context(
      tService &pService = efgy::global<tService>(),
      efgy::beacons<tProcess> &procs = efgy::global<efgy::beacons<tProcess>>())
      : service(pService), processes(procs) {}

  void update(void) {
    for (auto &p : processes) {
      p->update();
    }
  }

  void run(void) {
    for (auto &p : processes) {
      if (p->status() != tProcess::running) {
        (*p)();
      }
    }
  }

  bool haveTasks(void) const { return processes.size() > 0; }
};
}  // namespace eldritchd

#endif
