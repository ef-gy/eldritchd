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

#if !defined(ELDRITCHD_PROCESS_H)
#define ELDRITCHD_PROCESS_H

#include <ef.gy/cli.h>
#include <ef.gy/json.h>

#include <prometheus/metric.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace eldritchd {
static prometheus::metric::counter<> spawns("eldritchd_spawns_total",
                                            {"instance"});
static prometheus::metric::gauge<> child_pid("eldritchd_child_pid",
                                             {"instance"});

class process {
public:
  process(std::vector<std::string> pcmd, std::string pName = "",
          efgy::io::service &pService = efgy::io::service::common())
      : cmd(pcmd), name(pName), service(pService),
        signals(pService.get(), SIGCHLD) {
    static size_t instance = 0;
    this->instance = instance++;
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
    service.get().notify_fork(asio::io_service::fork_prepare);
    switch (pid = fork()) {
    case -1:
      return false;
    case 0:
      service.get().notify_fork(asio::io_service::fork_child);
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
      service.get().notify_fork(asio::io_service::fork_parent);

    signals
      .async_wait([this](const asio::error_code &, int) { sigchld(); });

      spawns.labels({name}).inc();
      child_pid.labels({name}).set(pid);

      return true;
    }

    return false;
  }

  efgy::json::value<> json(void) {
    efgy::json::value<> r;

    auto &p = r("process");
    p("name").toString() = name;

    return r;
  }

protected:
  const std::vector<std::string> cmd;
  efgy::io::service &service;
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
    signals
      .async_wait([this](const asio::error_code &, int) { sigchld(); });
    }
  }
};
}

#endif