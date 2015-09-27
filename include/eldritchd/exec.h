/**\file
 *
 * \copyright
 * Copyright (c) 2015, Magnus Achim Deininger <magnus@ef.gy>
 * \copyright
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * \copyright
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * \copyright
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * \see Documentation: https://ef.gy/documentation/eldritchd
 * \see Source Code: https://github.com/ef-gy/eldritchd
 * \see Licence Terms: https://github.com/ef-gy/eldritchd/COPYING
 */

#if !defined(ELDRITCHD_EXEC_H)
#define ELDRITCHD_EXEC_H

#include <ef.gy/cli.h>
#include <ef.gy/server.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace eldritchd {
class process {
public:
  process(std::vector<std::string> pcmd,
          efgy::io::service &pService = efgy::io::service::common())
      : cmd(pcmd), service(pService), signals(pService.get(), SIGCHLD) {}

  static void closeFDs(int from = 3) {
    for (int fd = from; fd < sysconf(_SC_OPEN_MAX); fd++) {
      close(fd);
    }
  }

  bool run(void) {
    service.get().notify_fork(asio::io_service::fork_prepare);
    pid_t p = fork();
    switch (p) {
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

      signals.async_wait([this](const asio::error_code &, int) { sigchld(); });

      return true;
    }

    return false;
  }

protected:
  const std::vector<std::string> cmd;
  efgy::io::service &service;
  asio::signal_set signals;

  void sigchld(void) {
    int status;
    waitpid(-1, &status, WNOHANG);
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
