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

namespace cli {
using efgy::cli::flag;

static flag<bool> daemonise(
    "daemonise", "whether or not to have eldritchd run in the background");

static flag<std::string> name("name",
                              "instance name used for monitoring output");

static flag<std::string> json("json",
                              "specify instance metadata as JSON string");

static flag<std::string> file("file", "configuration file to use");
}  // namespace cli

int main(int argc, char *argv[]) {
  auto &service = efgy::global<cxxhttp::service>();
  efgy::cli::options options(argc, argv);

  auto &procs = efgy::global<efgy::beacons<eldritchd::process>>();
  const std::string &json = cli::json;
  const std::string &file = cli::file;

  if (!json.empty()) {
    eldritchd::config::merge(json);
  }

  if (options.remainder.size() > 0) {
    new eldritchd::process(
        eldritchd::config::to_json(options.remainder, cli::name));
  }

  if (!file.empty()) {
    eldritchd::config::mergeFromFile(file);
  }

  if (procs.size() == 0) {
    std::cerr << "\nThe stars aren't right!\n";
    return 3;
  } else if (cli::daemonise && daemon(0, 0)) {
    std::cerr << "Couldn't turn into a daemon for some reason.\n";
    return 2;
  } else {
    for (auto &proc : procs) {
      if (!(*proc)()) {
        return 1;
      }
    }

    service.run();
  }

  return 0;
}
