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

#if !defined(ELDRITCHD_HTTPD_ELDRITCHD_H)
#define ELDRITCHD_HTTPD_ELDRITCHD_H

#include <eldritchd/http.h>

namespace eldritchd {
namespace httpd {
static cxxhttp::http::servlet eldritch(http::regex, http::servlet);
}
}

#endif
