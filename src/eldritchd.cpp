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

#define ASIO_DISABLE_THREADS
#define _BSD_SOURCE
#include <ef.gy/httpd.h>
#include <eldritchd/exec.h>
#include <unistd.h>
#include <prometheus/httpd.h>

using namespace efgy;

namespace tcp {
using asio::ip::tcp;
static httpd::servlet<tcp> quit("^/quit$", httpd::quit<tcp>);
static httpd::servlet<tcp> eldritch(eldritchd::http::regex,
                                    eldritchd::http::servlet<tcp>);
}

namespace unix {
using asio::local::stream_protocol;
static httpd::servlet<stream_protocol> quit("^/quit$",
                                            httpd::quit<stream_protocol>);
static httpd::servlet<stream_protocol> eldritch(
    eldritchd::http::regex, eldritchd::http::servlet<stream_protocol>);
}

static cli::boolean daemonise(
    "daemonise", "Whether or not to have eldritchd run in the background.");

static cli::string name("name", "Instance name used for monitoring output.");

int main(int argc, char *argv[]) {
  auto &options = cli::options<>::common();
  auto &service = io::service::common().get();

  options.apply(argc, argv);

  if (options.remainder.size() == 0) {
    std::cerr << "The stars aren't right!\n";
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
