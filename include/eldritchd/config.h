/* Read/write JSON configuration data.
 *
 * Eldritchd supports configuration in JSON, this header has helpers to deal
 * with that.
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

#if !defined(ELDRITCHD_CONFIG_H)
#define ELDRITCHD_CONFIG_H

#include <ef.gy/cli.h>
#include <ef.gy/global.h>
#include <ef.gy/json.h>

#include <eldritchd/process.h>

namespace eldritchd {
namespace config {
using efgy::json::json;
using efgy::json::to_string;

static const json to_json(const process &process) { return json(process); }

static const json to_json(const process *process) { return to_json(*process); }

/* Create JSON process descripton from a command line.
 * @pCMD The process command line.
 * @pName The name of the command.
 *
 * This creates a JSON process descriptor from a command line and a name, which
 * can be used to create a new `process` instance for running jobs.
 *
 * @returns JSON process description.
 */
static const json to_json(const std::vector<std::string> &pCMD,
                          const std::string &pName) {
  json process;
  auto &r = process("process");
  auto &a = r("command").toArray();

  if (pName != "") {
    r("name") = pName;
  } else {
    r("name") = "unnamed-process";
  }
  for (const auto &c : pCMD) {
    a.push_back(c);
  }

  return process;
}

/* Create JSON array from process list.
 * @processes A global beacon set with processes.
 *
 * This function takes a list of processes (as a beacon set) and turns them into
 * a JSON array that can be stored for later use.
 *
 * @returns a JSON array with all the processes listed.
 */
static const json to_json(const efgy::beacons<process> &processes) {
  json procs;
  auto &ps = procs.toArray();

  for (const auto &p : processes) {
    const auto &pj = to_json(p);
    ps.push_back(pj);
  }

  return procs;
}
}  // namespace config
}  // namespace eldritchd

#endif