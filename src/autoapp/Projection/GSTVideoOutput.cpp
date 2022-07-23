#include <fstream>
#include <unistd.h>
#include <autoapp/Projection/GSTVideoOutput.hpp>
#include <easylogging++.h>
#include <spawn.h>
#include <regex>

namespace autoapp::projection {

GSTVideoOutput::GSTVideoOutput(asio::io_service &ioService) : ioService_(ioService), running(false) {}

void GSTVideoOutput::message_handler(asio::error_code ec, size_t bytes_transferred) {
  if (bytes_transferred || !ec) {
    std::string s((std::istreambuf_iterator<char>(&buffer)), std::istreambuf_iterator<char>());
    buffer.consume(bytes_transferred);
    unsigned int newline = s.rfind('\n');
    s.erase(newline); // Remove extranious new line

    std::smatch m;
    std::regex e("\x1b[[0-9;]*m");

    LOG(INFO) << regex_replace(s, e, "");
  }
  if (ec != asio::error::operation_aborted) {
    asio::async_read_until(*sd, buffer, '\n', [this](asio::error_code ec, size_t bytes_transferred) {
      this->message_handler(ec, bytes_transferred);
    });
  }
}

void GSTVideoOutput::spawn_gst() {
  posix_spawn_file_actions_t fa;

  if (pipe2(p_stdin, O_CLOEXEC) || pipe2(p_stdout, O_CLOEXEC) != 0) return;
  gst_file = fdopen(p_stdin[1], "w");


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

  if (posix_spawn_file_actions_init(&fa) == 0) {
    if (posix_spawn_file_actions_adddup2(&fa, p_stdin[0], 0) == 0) {
      if (posix_spawn_file_actions_adddup2(&fa, p_stdout[1], 1) == 0) {
        if (posix_spawn(&gstpid, "/bin/sh", &fa, nullptr, launch, environment) == 0) {
          posix_spawn_file_actions_destroy(&fa);
          fcntl(p_stdin[1], F_SETFD, 0);
          fcntl(p_stdout[0], F_SETFD, 0);
          close(p_stdout[1]);
          close(p_stdin[0]);
          return;
        }
      }
    }
    posix_spawn_file_actions_destroy(&fa);
  }
}

bool GSTVideoOutput::open() {

  //Drop caches before staing new video
  sync();
  std::ofstream ofs("/proc/sys/vm/drop_caches");
  ofs << "3" << std::endl;

  if (gstpid == -1) {
    spawn_gst();

    sd = new asio::posix::stream_descriptor(ioService_, p_stdout[0]);
    asio::async_read_until(*sd, buffer, '\n', [this](asio::error_code ec, size_t bytes_transferred) {
      this->message_handler(ec, bytes_transferred);
    });
  }
  return true;
}

bool GSTVideoOutput::init() {
  return true;
}

void GSTVideoOutput::write(__attribute__((unused)) uint64_t timestamp,
                           const aasdk::common::DataConstBuffer &buf) {
  if (gstpid != -1) {
    fwrite(buf.cdata, sizeof(buf.cdata[0]), buf.size, gst_file);
  }
}

void GSTVideoOutput::stop() {
  if (gstpid != -1) {
    kill(gstpid, SIGTERM);
    gstpid = -1;
    fclose(gst_file);
    close(p_stdin[1]);
    sd->close();
  }
}

GSTVideoOutput::~GSTVideoOutput() = default;

VideoMargins GSTVideoOutput::getVideoMargins() const {
  VideoMargins margins(0, 0);
  return margins;
}
}