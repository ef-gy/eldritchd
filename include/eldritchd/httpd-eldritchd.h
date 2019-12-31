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

#include <eldritchd/config.h>
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

/* HTTP servlet.
 * @session The HTTP session to respond to queries from.
 *
 * This servlet allows querying configuration information from a running
 * eldritchd instance.
 */
static void servlet(typename cxxhttp::http::sessionData &session,
                    std::smatch &) {
  auto &ctx = efgy::global<process::context>();

  session.reply(200, config::to_string(config::to_json(ctx)));
}

/* Default servlet.
 *
 * Default global instance of the configuration servlet.
 */
static cxxhttp::http::servlet eldritch(resource, servlet, method, negotiations,
                                       description);
}  // namespace httpd
}  // namespace eldritchd

#endif
