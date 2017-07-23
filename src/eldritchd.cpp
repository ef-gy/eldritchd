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

#define ASIO_DISABLE_THREADS
#define _BSD_SOURCE
#include <cxxhttp/httpd.h>

#include <prometheus/httpd-metrics.h>

#include <eldritchd/httpd-eldritchd.h>

#include <unistd.h>

static efgy::cli::flag<bool> daemonise(
    "daemonise", "Whether or not to have eldritchd run in the background.");

static efgy::cli::flag<std::string> name(
    "name", "Instance name used for monitoring output.");

static efgy::cli::flag<std::string> json(
    "json", "specify instance metadata as JSON string");

int main(int argc, char *argv[]) {
  auto &service = efgy::global<cxxhttp::service>();
  efgy::cli::options options(argc, argv);

  auto &procs = efgy::global<efgy::beacons<eldritchd::process>>();
  const std::string &j = json;

  if (!j.empty()) {
    efgy::json::json v;
    efgy::json::parse(j, v);
    new eldritchd::process(v);
  }

  if (options.remainder.size() > 0) {
    new eldritchd::process(options.remainder, name);
  }

  if (procs.size() == 0) {
    std::cerr << "\nThe stars aren't right!\n";
    return 3;
  } else if (daemonise && daemon(0, 0)) {
    std::cerr << "Couldn't turn into a daemon for some reason.\n";
    return 2;
  } else {
    for (auto &proc : procs) {
      if (!proc->run()) {
        return 1;
      }
    }

    service.run();
  }

  return 0;
}
