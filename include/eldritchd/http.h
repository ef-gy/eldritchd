/**\file
 *
 * \copyright
 * This file is part of eldritchd, which is released as open source under the
 * terms of an MIT/X11-style licence, described in the COPYING file.
 *
 * \see Documentation: https://ef.gy/documentation/eldritchd
 * \see Source Code: https://github.com/ef-gy/eldritchd
 * \see Licence Terms: https://github.com/ef-gy/eldritchd/COPYING
 */

#if !defined(ELDRITCHD_HTTP_H)
#define ELDRITCHD_HTTP_H

#include <ef.gy/http.h>

#include <eldritchd/process.h>

namespace eldritchd {
namespace http {
static const std::string regex = "/eldritchd";

template <class transport>
static bool
servlet(typename efgy::net::http::server<transport>::session &session,
        std::smatch &) {
  session.reply(200, "...");

  return true;
}
}
}

#endif
