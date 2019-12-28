/* Eldritchd HTTP servlet.
 *
 * Can be used to manipulate a serving instance of eldritchd.
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

#if !defined(ELDRITCHD_HTTPD_ELDRITCHD_H)
#define ELDRITCHD_HTTPD_ELDRITCHD_H

#include <cxxhttp/httpd.h>
#include <eldritchd/process.h>

namespace eldritchd {
namespace httpd {
/* HTTP resource regex.
 *
 * What locations the servlet is using.
 */
static const char *resource = "/eldritchd";

/* HTTP method regex.
 *
 * The supported methods for the servlet. This currently only allows getting
 * the current configuration.
 */
static const char *method = "GET";

/* HTTP content negotiations.
 *
 * This is an API that currently only works on JSON data.
 */
static const cxxhttp::http::headers negotiations = {
    {"Accept", "application/json"}};

/* HTTP servlet description.
 *
 * Describes what that servlet can do.
 */
static const char *description = "Get current configuration of eldritchd.";

static void servlet(typename cxxhttp::http::sessionData &session,
                    std::smatch &) {
  using efgy::json::json;
  using efgy::json::to_string;

  auto &processes = efgy::global<efgy::beacons<process>>();

  json config;
  auto &procs = config("processes").toArray();

  for (const auto &p : processes) {
    const auto &pj = json(*p);
    procs.push_back(pj);
  }

  session.reply(200, to_string(config));
}

static cxxhttp::http::servlet eldritch(resource, servlet, method, negotiations,
                                       description);
}  // namespace httpd
}  // namespace eldritchd

#endif
