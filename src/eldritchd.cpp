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

#define ASIO_DISABLE_THREADS
#define _BSD_SOURCE
#include <cxxhttp/httpd.h>

#include <prometheus/httpd-metrics.h>

#include <eldritchd/http.h>

#include <unistd.h>

using namespace cxxhttp;
using namespace efgy;

namespace tcp {
using asio::ip::tcp;
static httpd::servlet<tcp> eldritch(eldritchd::http::regex,
                                    eldritchd::http::servlet<tcp>);
}

namespace unix {
using asio::local::stream_protocol;
static httpd::servlet<stream_protocol> eldritch(
    eldritchd::http::regex, eldritchd::http::servlet<stream_protocol>);
}

static cli::flag<bool> daemonise(
    "daemonise", "Whether or not to have eldritchd run in the background.");

static cli::flag<std::string> name("name",
                                   "Instance name used for monitoring output.");

int main(int argc, char *argv[]) {
  auto &service = global<cxxhttp::service>();
  cli::options options(argc, argv);

  if (options.remainder.size() == 0) {
    std::cerr << "\nThe stars aren't right!\n";
    return 3;
  } else if (daemonise && daemon(0, 0)) {
    std::cerr << "Couldn't turn into a daemon for some reason.\n";
    return 2;
  } else {
    eldritchd::process proc(options.remainder, name);

    if (!proc.run()) {
      return 1;
    }

    service.run();
  }

  return 0;
}
