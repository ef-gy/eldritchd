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

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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
      : context_(pContext),
        status_(idle),
        signals(context_.service, SIGCHLD),
        beacon(*this, context_.processes) {
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

    context_.service.notify_fork(asio::io_service::fork_prepare);
    switch (pid = fork()) {
      case -1:
        return false;
      case 0:
        context_.service.notify_fork(asio::io_service::fork_child);
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
        context_.service.notify_fork(asio::io_service::fork_parent);
        status_ = running;

        signals.async_wait(
            [this](const asio::error_code &, int) { sigchld(); });

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

  void update(void) {
    int status;
    waitpid(pid, &status, WNOHANG);
    if (WIFEXITED(status) || WIFSIGNALED(status)) {
      status_ = dead;
    }
  }

  enum status status(void) const { return status_; }

 protected:
  context &context_;
  enum status status_;
  std::vector<std::string> cmd;
  asio::signal_set signals;
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

  /* SIGCHLD handler.
   *
   * We want our processes to be running continuously, so respawn them if
   * they die. If spawning them fails this would put them in an unmonitored
   * state.
   */
  void sigchld(void) {
    if (status_ == running) {
      update();
    }

    if (status_ == dead) {
      (*this)();
    } else {
      signals.async_wait([this](const asio::error_code &, int) { sigchld(); });
    }
  }
};
}  // namespace eldritchd

#endif
