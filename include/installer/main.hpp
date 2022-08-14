#pragma once

#include <tinyxml2.h>
#include <string>
#include <filesystem>
#include "installer/main.hpp"
#include "easylogging++.h"

namespace fs = std::filesystem;


class installer {
 public:
  installer(fs::path backup_dir);
  int mkdir(const fs::path &path);
  void backup(const fs::path &path);
  void install_bds();
  static bool checkAapaVersion();
  void setup_sm();
  void setup_mmui();
  void configure_opera();
  void copy_file(const fs::path& from, const fs::path& to);
  void install_files();
  void generate_uninstaller();

 private:
  fs::path backup_path = "/mnt/data_persist/dev/backup";
  std::vector<std::filesystem::path> backup_files;
  std::vector<std::filesystem::path> installed_files;
};
