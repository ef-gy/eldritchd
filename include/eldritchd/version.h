/**\file
 * \brief eldritchd core version number
 *
 * This header contains the version number of eldritchd.
 *
 * \copyright
 * This file is part of eldritchd, which is released as open source under the
 * terms of an MIT/X11-style licence, described in the COPYING file.
 *
 * \see Documentation: https://ef.gy/documentation/eldritchd
 * \see Source Code: https://github.com/ef-gy/eldritchd
 * \see Licence Terms: https://github.com/ef-gy/eldritchd/COPYING
 */

#if !defined(ELDRITCHD_VERSION_H)
#define ELDRITCHD_VERSION_H

/**\brief eldritchd base name space
 *
 * Everything related to eldritchd is contained in this namespace.
 */
namespace eldritchd {
/**\brief eldritchd version
 *
 * This is the version number of the eldritchd library. It's a single integer,
 * because I don't believe in sub-versioning.
 */
static const unsigned int version = 1;
}

#endif
