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

#if !defined(ELDRITCHD_PROCESS_H)
#define ELDRITCHD_PROCESS_H

#include <eldritchd/context.h>

#include <ef.gy/cli.h>
#include <ef.gy/json.h>

#include <prometheus/metric.h>

#include <sys/wait.h>

namespace eldritchd {
static prometheus::metric::counter spawns(
    "eldritchd_spawns_total", "Subprocess spawn count, by instance label.",
    {"instance"});
static prometheus::metric::gauge child_pid(
    "eldritchd_child_pid", "Child process PID, by instance label.",
    {"instance"});

/* A supervised process
 *
 * This class is used to hold all the metadata necessary to spawn a single
 * process and keep it alive when it fails.
 */
class process {
 public:
  enum status { idle, running, dead };

  using context = eldritchd::context<cxxhttp::service, process>;

  process(const efgy::json::json &pJSON,
          context &pContext = efgy::global<context>())
      : context_(pContext), status_(idle), beacon(*this, context_.processes()) {
    efgy::json::json json(pJSON);

    auto &r = json("process");
    auto &a = r("command").asArray();

    name = r("name").asString();
    for (const auto &c : a) {
      cmd.push_back(c.asString());
    }

    if (name == "") {
      if (cmd.size() > 0) {
        name = cmd[0];
      } else {
        name = "empty-command";
      }
    }
  }

  /* Run process.
   *
   * Spawns a subprocess and tries to execute it as a new binary.
   */
  bool operator()(void) {
    if (cmd.size() == 0) {
      return false;
    }

    switch (pid = context_.fork()) {
      case -1:
        // an error occurred, so we break out and return false later.
        break;
      case 0:
        closeFDs();
        {
          char **argv = new char *[(cmd.size() + 1)];
          for (std::size_t i = 0; i < cmd.size(); i++) {
            argv[i] = (char *)cmd[i].c_str();
          }
          argv[cmd.size()] = 0;
          execv(argv[0], argv);
          // ... and we should never reach this part.
        }
        break;
      default:
        status_ = running;

        spawns.labels({name}).inc();
        child_pid.labels({name}).set(pid);

        return true;
    }

    return false;
  }

  operator const efgy::json::json(void) const {
    efgy::json::json r;

    auto &p = r("process");
    p("name") = name;
    auto &a = p("command").toArray();
    for (const auto &c : cmd) {
      a.push_back(c);
    }

    return r;
  }

  /* Update status of child process.
   *
   * Calls waitpid() for this process and updates `status_` accordingly.
   *
   * @returns `this->status_`.
   */
  enum status update(void) {
    int st = 0;
    pid_t p = waitpid(pid, &st, WNOHANG);
    if (p == 0 || p == 1) {
      // TODO: log an error of some form here.
    } else if (p == pid) {
      if (WIFEXITED(st) || WIFSIGNALED(st)) {
        status_ = dead;
      } else {
      }
    } else {
      // this branch should be impossible.
    }

    return status_;
  }

  enum status status(void) const { return status_; }

 protected:
  context &context_;
  enum status status_;
  std::vector<std::string> cmd;
  pid_t pid;
  std::string name;

  efgy::beacon<process> beacon;

  /* Close all file descriptors.
   * @from minimum FD to close; the default of 3 closes non-STDIO
   * descriptors.
   *
   * Processes tend to act strangely when superfluous file descriptors are
   * open. This function is called after fork()'ing off a new process, and
   * by default will close all non-STDIO file descriptors, leaving STDIN,
   * STDOUT and STDERR untouched.
   */
  static void closeFDs(int from = 3) {
    for (int fd = from; fd < sysconf(_SC_OPEN_MAX); fd++) {
      close(fd);
    }
  }
};
}  // namespace eldritchd

#endif
