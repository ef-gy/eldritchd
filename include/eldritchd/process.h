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

#include <cxxhttp/network.h>

#include <ef.gy/cli.h>
#include <ef.gy/json.h>
#include <ef.gy/global.h>

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

class process {
 protected:
  efgy::beacon<process> beacon;

  static size_t nextInstance(void) {
    static size_t instance = 0;
    return instance++;
  }

 public:
  process(
      const efgy::json::json &pJSON,
      cxxhttp::service &pService = efgy::global<cxxhttp::service>(),
      efgy::beacons<process> &procs = efgy::global<efgy::beacons<process>>())
      : instance(nextInstance()),
        service(pService),
        signals(pService, SIGCHLD),
        beacon(*this, procs) {
    efgy::json::json json(pJSON);
    auto &r = json("process");
    auto &a = r("command").asArray();

    name = r("name").asString();
    for (const auto &c : a) {
      cmd.push_back(c.asString());
    }

    if (name == "") {
      name = cmd[0] + "-" + std::to_string(this->instance);
    }
  }

  process(
      const std::vector<std::string> &pcmd, const std::string &pName = "",
      cxxhttp::service &pService = efgy::global<cxxhttp::service>(),
      efgy::beacons<process> &procs = efgy::global<efgy::beacons<process>>())
      : instance(nextInstance()),
        cmd(pcmd),
        name(pName),
        service(pService),
        signals(pService, SIGCHLD),
        beacon(*this, procs) {
    if (name == "") {
      name = cmd[0] + "-" + std::to_string(this->instance);
    }
  }

  static void closeFDs(int from = 3) {
    for (int fd = from; fd < sysconf(_SC_OPEN_MAX); fd++) {
      close(fd);
    }
  }

  bool run(void) {
    service.notify_fork(asio::io_service::fork_prepare);
    switch (pid = fork()) {
      case -1:
        return false;
      case 0:
        service.notify_fork(asio::io_service::fork_child);
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
        service.notify_fork(asio::io_service::fork_parent);

        signals.async_wait(
            [this](const asio::error_code &, int) { sigchld(); });

        spawns.labels({name}).inc();
        child_pid.labels({name}).set(pid);

        return true;
    }

    return false;
  }

  efgy::json::json json(void) {
    efgy::json::json r;

    auto &p = r("process");
    p("name") = name;
    auto &a = p("command").toArray();
    for (const auto &c : cmd) {
      a.push_back(c);
    }

    return r;
  }

 protected:
  std::vector<std::string> cmd;
  cxxhttp::service &service;
  asio::signal_set signals;
  pid_t pid;
  size_t instance;
  std::string name;

  void sigchld(void) {
    int status;
    waitpid(pid, &status, WNOHANG);
    if (WIFEXITED(status) || WIFSIGNALED(status)) {
      std::cout << "It died - respawning.\n";

      run();
    } else {
      signals.async_wait([this](const asio::error_code &, int) { sigchld(); });
    }
  }
};
}

#endif
