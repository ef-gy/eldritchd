/*
 * See also:
 * * Documentation: https://ef.gy/documentation/eldritchd
 * * Source Code: https://github.com/ef-gy/eldritchd
 * * Licence Terms: https://github.com/ef-gy/eldritchd/COPYING
 *
 * @copyright
 * This file is part of eldritchd, which is released as open source under the
 * terms of an MIT/X11-style licence, described in the COPYING file.
 */

#if !defined(ELDRITCHD_HTTP_H)
#define ELDRITCHD_HTTP_H

#include <cxxhttp/http-session.h>

#include <ef.gy/json.h>

#include <eldritchd/process.h>

#include <sstream>

namespace eldritchd {
namespace http {
static const std::string regex = "/eldritchd";

static void servlet(typename cxxhttp::http::sessionData &session,
                    std::smatch &) {
  auto &processes = efgy::global<efgy::beacons<process>>();

  std::ostringstream oss("");

  for (const auto &p : processes) {
    oss << efgy::json::to_string(p->json());
  }

  session.reply(200, oss.str());
}
}  // namespace http
}  // namespace eldritchd

#endif
