#include <fstream>
#include <unistd.h>
#include <autoapp/Projection/GSTVideoOutput.hpp>
#include <easylogging++.h>
#include <spawn.h>
#include <regex>

namespace autoapp::projection {

GSTVideoOutput::GSTVideoOutput(asio::io_service &ioService) : ioService_(ioService) {}

void GSTVideoOutput::message_handler(asio::error_code errorCode, size_t bytes_transferred) {
  if (bytes_transferred > 0 || !errorCode) {
    std::string logString((std::istreambuf_iterator<char>(&buffer)), std::istreambuf_iterator<char>());
    buffer.consume(bytes_transferred);

    logString.erase(logString.rfind('\n')); // Remove extranious new line

    std::smatch match;
    std::regex eraseRegex("\x1b.*m"); //Regex to remove terminal escape sequences

    LOG(INFO) << regex_replace(logString, eraseRegex, "");
  }
  if (errorCode != asio::error::operation_aborted) {
    if(sd != nullptr) {
      asio::async_read_until(*sd, buffer, '\n', [this](asio::error_code errorCode, size_t bytes_transferred) {
        this->message_handler(errorCode, bytes_transferred);
      });
    }
  }
}

void GSTVideoOutput::spawn_gst() {
  posix_spawn_file_actions_t file_actions;
  char *const launch[] = {
      (char *) "sh",
      (char *) "-c",
      (char *) "gst-launch fdsrc fd=0 timeout=1000 do-timestamp=true ! queue " \
      "! capsfilter caps=video/x-h264,framerate=30 " \
      "! vpudec low-latency=true framedrop=true framedrop-level-mask=0x200 frame-plus=2 "\
      "!  mfw_v4lsink name=aavideo 2>&1",
      nullptr};

  char *const environment[] = {
      (char *) "USER=jci",
      (char *) "LD_LIBRARY_PATH=/jci/lib:/jci/opera/3rdpartylibs/freetype:/usr/lib/imx-mm/audio-codec:/usr/lib/imx-mm/parser:/data_persist/dev/lib:",
      (char *) "HOME=/root",
      (char *) "WAYLAND_IVI_SURFACE_ID=2",
      (char *) "PATH=/sbin:/usr/sbin:/bin:/usr/bin:/jci/bin/:/resources/dev/bin:/resources/dev/sbin:/data_persist/dev/bin:/data_persist/dev/sbin:/resources/dev/usr/bin",
      (char *) "XDG_RUNTIME_DIR=/tmp",
      (char *) "SHELL=/bin/sh",
      (char *) "PWD=/tmp/root",
      (char *) "GST_PLUGIN_PATH=/usr/lib/gstreamer-0.10",
      nullptr
  };

  if (posix_spawn_file_actions_init(&file_actions) == 0) {
    if (posix_spawn_file_actions_adddup2(&file_actions, p_stdin[0], 0) == 0) {
      if (posix_spawn_file_actions_adddup2(&file_actions, p_stdout[1], 1) == 0) {
        if (posix_spawn(&gstpid, "/bin/sh", &file_actions, nullptr, launch, environment) == 0) {
          posix_spawn_file_actions_destroy(&file_actions);
          fcntl(p_stdin[1], F_SETFD, 0);
          fcntl(p_stdout[0], F_SETFD, 0);
          close(p_stdout[1]);
          close(p_stdin[0]);
          return;
        }
      }
    }
    posix_spawn_file_actions_destroy(&file_actions);
  }
}

bool GSTVideoOutput::open() {

  //Drop caches before staing new video
  sync();
  std::ofstream ofs("/proc/sys/vm/drop_caches");
  ofs << "3" << std::endl;

  if (gstpid == -1) {

    if (pipe2(p_stdin.data(), O_CLOEXEC) != 0 || pipe2(p_stdout.data(), O_CLOEXEC) != 0) {
      return false;
    }


    /* If the child's end of the pipe happens to already be on the final
     * fd number to which it will be assigned (either 0 or 1), it must
     * be moved to a different fd. Otherwise, there is no safe way to
     * remove the close-on-exec flag in the child without also creating
     * a file descriptor leak race condition in the parent. */
    if (p_stdin[0] == 0) {
      int tmp = fcntl(F_DUPFD_CLOEXEC, 0, 0);
      close(p_stdin[0]);
      p_stdin[0] = tmp;
    }
    const int pipe_max = 1048576; //maximum size of pipe on our system
    fcntl(p_stdin[1], F_SETPIPE_SZ, pipe_max);
    spawn_gst();

//    sd = new asio::posix::stream_descriptor(ioService_, p_stdout[0]);
//    asio::async_read_until(*sd, buffer, '\n', [this](asio::error_code error_code, size_t bytes_transferred) {
//      this->message_handler(error_code, bytes_transferred);
//    });
  }
  return true;
}

bool GSTVideoOutput::init() {
  return true;
}

void GSTVideoOutput::write(__attribute__((unused)) uint64_t timestamp,
                           const aasdk::common::DataConstBuffer &buf) {
  ::write(p_stdin[1], buf.cdata, buf.size);
}

void GSTVideoOutput::stop() {
  if (gstpid != -1) {
    kill(gstpid, SIGTERM);
    gstpid = -1;
    close(p_stdin[1]);
    if(sd != nullptr) {
      sd->close();
    }
  }
}

GSTVideoOutput::~GSTVideoOutput() = default;

VideoMargins GSTVideoOutput::getVideoMargins() const {
  VideoMargins margins(0, 0);
  return margins;
}
}