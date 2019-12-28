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
class process;

class context {
 public:
  cxxhttp::service &service;
  efgy::beacons<process> &processes;

  context(
      cxxhttp::service &pService = efgy::global<cxxhttp::service>(),
      efgy::beacons<process> &procs = efgy::global<efgy::beacons<process>>())
      : service(pService), processes(procs) {}
};
}  // namespace eldritchd

#endif
