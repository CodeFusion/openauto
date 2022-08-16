#include <string>
#include <filesystem>
#include <utility>
#include "installer/main.hpp"
#include "easylogging++.h"
#include "sys/mount.h"
#include <getopt.h>

#define MINI_CASE_SENSITIVE
#include <ini.h>

INITIALIZE_EASYLOGGINGPP

int installer::mkdir(const fs::path &path) {
  fs::directory_entry dir_path{path};
  if (!dir_path.exists()) {
    if (!fs::create_directories(dir_path)) {
      LOG(ERROR) << "Couldn't create " << dir_path;
      return -1;
    }
    installed_files.emplace_back(path);
  }
  LOG(INFO) << path << " Created";
  return 0;
}

void installer::backup(const fs::path &path) {
  fs::path backup_destination = backup_path;
  backup_destination += path;
  if (!std::filesystem::exists(backup_destination)) {
    if (mkdir(backup_destination.parent_path()) == 0) {
      LOG(INFO) << "Backing up " << path << " to " << backup_destination << std::endl;
      try {
        fs::copy(path, backup_destination);
      }
      catch (fs::filesystem_error &error) {
        LOG(INFO) << backup_destination << " " << error.what() << std::endl;
      }
    }
  }
  backup_files.emplace_back(backup_destination);
}

void installer::install_bds() {
  /// Configure BDS so that Android Auto can connect to our Bluetooth service to get the WiFi settings

  backup("/jci/bds/BdsConfiguration.xml");

  tinyxml2::XMLDocument doc;
  doc.LoadFile("/jci/bds/BdsConfiguration.xml");

  tinyxml2::XMLNode *docRoot = doc.FirstChild();

  tinyxml2::XMLElement *serviceconfig = docRoot->FirstChildElement("serviceConfiguration");

  if (serviceconfig == nullptr) {
    LOG(DEBUG) << "Couldn't find serviceConfiguration in /jci/bds/BdsConfiguration.xml";
  } else {
    int lastId = 0;
    bool bdsconfigured = false;
    for (tinyxml2::XMLElement *element = serviceconfig->FirstChildElement(); element != nullptr;
         element = element->NextSiblingElement()) {
      lastId = element->IntAttribute("id", lastId);
      if (std::string(element->Attribute("name")) == "AndroidAuto") {
        bdsconfigured = true;
        LOG(INFO) << "/jci/bds/BdsConfiguration.xml already configured";
      }
    }
    if (!bdsconfigured) {
      tinyxml2::XMLElement *element = serviceconfig->InsertNewChildElement("serialPort");
      element->SetAttribute("id", lastId + 1);
      element->SetAttribute("name", "AndroidAuto");
      element->SetAttribute("noOfInstances", 1);
      element->SetAttribute("critical", false);
      element->SetAttribute("enabled", true);
      element->SetAttribute("uuidServer", "4DE17A0052CB11E6BDF40800200C9A66");
      element->SetAttribute("uuidClient", "4DE17A0052CB11E6BDF40800200C9A66");
      element->SetAttribute("writeDelay", 3);
      doc.SaveFile("/jci/bds/BdsConfiguration.xml");
      LOG(INFO) << "/jci/bds/BdsConfiguration.xml configured";
    }
  }
}

bool installer::checkAapaVersion() {
  /// Check if CMU Firmware already has Android Auto built in.

  mINI::INIFile file("/jci/version.ini");
  mINI::INIStructure ini;
  file.read(ini);
  return ini["VersionInfo"].has("JCI_BLM_AAPA-IHU");
}

void installer::setup_sm() {
  /// Configure SM to start autoapp on boot.

  backup("/jci/sm/sm.conf");

  tinyxml2::XMLDocument doc;
  doc.LoadFile("/jci/sm/sm.conf");

  tinyxml2::XMLNode *docRoot = doc.FirstChild()->NextSibling();

  LOG(DEBUG) << docRoot->GetLineNum();

  tinyxml2::XMLElement *serviceconfig = docRoot->FirstChildElement("services");

  if (serviceconfig == nullptr) {
    LOG(DEBUG) << "Couldn't find services in /jci/sm/sm.conf";
  } else {
    bool serviceconfigured = false;
    for (tinyxml2::XMLElement *element = serviceconfig->FirstChildElement(); element != nullptr;
         element = element->NextSiblingElement()) {
      if (std::string(element->Attribute("name")) == "autoapp") {
        serviceconfigured = true;
        LOG(INFO) << "/jci/sm/sm.conf already configured";
      }
    }
    if (!serviceconfigured) {
      tinyxml2::XMLElement *element = serviceconfig->InsertNewChildElement("service");
      element->SetAttribute("type", "process");
      element->SetAttribute("name", "autoapp");
      element->SetAttribute("path", "/mnt/data_persist/dev/bin/autoapp");
      element->SetAttribute("autorun", "yes");
      element->SetAttribute("reset_board", "no");
      element->SetAttribute("retry_count", 6);
      element->SetAttribute("args", "");
      tinyxml2::XMLElement *dependancy = element->InsertNewChildElement("dependency");
      dependancy->SetAttribute("type", "service");
      dependancy->SetAttribute("value", "bds");
      dependancy = element->InsertNewChildElement("dependency");
      dependancy->SetAttribute("type", "service");
      dependancy->SetAttribute("value", "audio_manager");

    }
    if (checkAapaVersion()) {
      for (tinyxml2::XMLElement *element = serviceconfig->FirstChildElement(); element != nullptr;
           element = element->NextSiblingElement()) {
        if (std::string(element->Attribute("name")) == "jciAAPA") {
          LOG(INFO) << "Disabling jciAAPA";
          element->SetAttribute("autorun", false);
        }
        if (std::string(element->Attribute("name")) == "aap_service") {
          LOG(INFO) << "Disabling aap_service";
          element->SetAttribute("autorun", false);
        }
      }
    }
    doc.SaveFile("/jci/sm/sm.conf");
    LOG(INFO) << "/jci/sm/sm.conf configured";

  }

}

void installer::setup_mmui() {
  /// Setup the MMUI configuration.
  /// We need to increase the priority of androidauto in MMUI, so that it doesn't loose video focus to phone calls

  backup("/jci/mmui/mmui_config.xml");

  tinyxml2::XMLDocument doc;
  doc.LoadFile("/jci/mmui/mmui_config.xml");

  tinyxml2::XMLNode *docRoot = doc.FirstChild()->NextSibling();

  LOG(DEBUG) << docRoot->GetLineNum();

  for (tinyxml2::XMLElement *element = docRoot->FirstChildElement(); element != nullptr;
       element = element->NextSiblingElement()) {
    if (std::string(element->Attribute("name")) == "androidauto") {
      element->SetAttribute("priority", 30);
      LOG(DEBUG) << "Set androidauto priority to 30";
      doc.SaveFile("/jci/mmui/mmui_config.xml");
      break;
    }
  }

}

void installer::configure_opera() {
  /// Configure Opera settings for Android Auto if not using a version of CMU Firmware that has it built in.

  backup("/jci/opera/opera_home/opera.ini");

  mINI::INIFile file("/jci/opera/opera_home/opera.ini");
  mINI::INIStructure ini;
  file.read(ini);

  std::string &user_javascript = ini["User Prefs"]["User JavaScript"];
  std::string &allow_xmlhttprequests = ini["User Prefs"]["Allow File XMLHttpRequest"];

  user_javascript.assign("1");
  allow_xmlhttprequests.assign("1");
  file.write(ini, false);

  if (fs::exists("/jci/opera/opera_dir/userjs/fps.js")) {
    backup("/jci/opera/opera_dir/userjs/fps.js");
    fs::remove("/jci/opera/opera_dir/userjs/fps.js");
  }
}

void installer::copy_file(const fs::path &from, const fs::path &to) {
  if (std::filesystem::exists(to.parent_path())) {
    mkdir(to.parent_path());
    installed_files.emplace_back(to.parent_path());
  }
  try {
    std::filesystem::copy(from, to);
  }
  catch (fs::filesystem_error &error) {
    LOG(INFO) << to << " " << error.what() << std::endl;
  }
  installed_files.emplace_back(to);
}

void installer::install_files() {
  if (!checkAapaVersion()) {
    for (std::filesystem::recursive_directory_iterator i("jci"), end; i != end; ++i) {
      if (!is_directory(i->path())) {
        std::filesystem::path path = "/";
        path += std::filesystem::relative(i->path());
        copy_file(std::filesystem::absolute(i->path()), path);
      }
    }
  }
  mkdir("/mnt/data_persist/dev/bin");
  copy_file(fs::path("autoapp"), "/mnt/data_persist/dev/bin/autoapp");
  if (std::filesystem::exists("autoapp_configuration.toml")) {
    copy_file("autoapp_configuration.toml", "/mnt/data_persist/dev/bin/autoapp_configuration.toml");
  }
}

void installer::generate_uninstaller() {
  fs::path uninstall = "/mnt/data_persist/dev/bin/autoapp.uninstall";
  installed_files.emplace_back(uninstall);
  std::ofstream uninstallScript;
  uninstallScript.open("/mnt/data_persist/dev/bin/autoapp.uninstall");
  uninstallScript << "#!/bin/ash\n";
  uninstallScript << "mount -o remount,rw / \n";
  for (const auto &file : backup_files) {
    std::filesystem::path destination = "/";
    destination += std::filesystem::relative(std::filesystem::absolute(file), backup_path);
    uninstallScript << "mv " << file << " " << destination << " \n";
  }
  uninstallScript << "rm -rf";
  for (const auto &file : installed_files) {
    uninstallScript << " \\\n" << file;
  }
  uninstallScript << "printf \"Uninstalled\\n\"\n";
  uninstallScript.close();
  fs::permissions(uninstall, fs::perms::owner_exec | fs::perms::group_exec, fs::perm_options::add);
}

installer::installer(fs::path backup_dir) : backup_path(std::move(backup_dir)) {

}

void usage(const char *path) {
  const char *basename = strrchr(path, '/');
  basename = basename != nullptr ? basename + 1 : path;

  std::cout << "usage: " << basename << " [OPTION]\n";
  std::cout << "  -h, --help\t\t" <<"Print this help and exit.\n";
  std::cout << "  -w, --no-wifi\t\t" << "disable WiFi\n";
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) {
  int opt;
  int help_flag = 0;
  bool wifi = true;
  struct option longopts[] = {
      {"help", no_argument, &help_flag, 1},
      {"no-wifi", optional_argument, nullptr, 'w'},
      {nullptr}
  };

  if (argc > 1) {
    while (true) {
      opt = getopt_long(argc, argv, "hw", longopts, nullptr);

      if (opt == -1) {
        /* a return value of -1 indicates that there are no more options */
        break;
      }
      switch (opt) {
        case 'h':
          help_flag = 1;
          break;
        case 'w':
          wifi = false;
          break;
        case '?':
          return 1;
        default:
          break;
      }
    }
  }
  if (help_flag == 1) {
    usage(argv[0]);
    return 0;
  }

  if (mount("", "/", "", MS_REMOUNT | MS_NOATIME, "") != 0) {
    LOG(ERROR) << "Couldn't remount / rw";
    exit(1);
  }

  fs::path uninstaller = "/mnt/data_persist/dev/bin/autoapp.uninstall";
  if (fs::exists(uninstaller)) {
    LOG(INFO) << "Running uninstaller to cleanup old install";
    system(uninstaller.c_str());
  }

  const auto path_backup = fs::absolute("/mnt/data_persist/dev/backup/");
  installer install(path_backup);
  install.mkdir(path_backup);
  fs::directory_entry dir_backup{path_backup};

  if (wifi) {
    install.install_bds();
  }

  install.setup_sm();

  // Set up the right configuration depending on CMU version
  if (!install.checkAapaVersion()) {
    install.configure_opera();
  }
  else {
    install.setup_mmui();
  }

  install.install_files();
  install.generate_uninstaller();
  if (mount("", "/", "", MS_REMOUNT | MS_NOATIME | MS_RDONLY, "") != 0) {
    LOG(ERROR) << "Couldn't remount / ro";
  }
}