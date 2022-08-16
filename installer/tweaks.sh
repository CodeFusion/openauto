#!/bin/ash

# some usefull things (thanks to oz_paulb from mazda3revolution.com)

get_cmu_sw_version() {
  _ver=$(grep "^JCI_SW_VER=" /jci/version.ini | sed 's/^.*_\([^_]*\)\"$/\1/')
  _patch=$(grep "^JCI_SW_VER_PATCH=" /jci/version.ini | sed 's/^.*\"\([^\"]*\)\"$/\1/')
  _flavor=$(grep "^JCI_SW_FLAVOR=" /jci/version.ini | sed 's/^.*_\([^_]*\)\"$/\1/')

  printf '%s%s%s\n' "$_ver" "$_patch" "${_flavor:+-$_flavor}"
}

MYDIR=$(dirname "$(readlink -f "$0")")
CMU_SW_VER=$(get_cmu_sw_version)
if [ -f "${MYDIR}/installer_log.txt" ]; then
  #save old logs
  logidx=1
  while [ -f "${MYDIR}/installer_log_${logidx}.txt" ]; do
     _=$((logidx++))
  done
  mv "${MYDIR}/installer_log.txt" "${MYDIR}/installer_log_${logidx}.txt"
fi

log_message() {
  printf '%b' "$*" >>"${MYDIR}/installer_log.txt"
  fsync "${MYDIR}/installer_log.txt"
}

#============================= DIALOG FUNCTIONS

show_message() { # $1 - title, $2 - message
  killall jci-dialog  > /dev/null 2>&1
  log_message "= POPUP INFO: $*\n"
  jci-dialog --info --title="$1" --text="$2" --no-cancel
}

show_confirmation() { # $1 - title, $2 - message
  killall jci-dialog  > /dev/null 2>&1
  log_message "= POPUP CONFIRM: $*\n"
  jci-dialog --confirm --title="$1" --text="$2" --no-cancel
}

show_error() { # $1 - title, $2 - message
  killall jci-dialog  > /dev/null 2>&1
  log_message "= POPUP ERROR: $*\n"
  jci-dialog --error --title="$1" --text="$2" --ok-label='OK' --no-cancel
}

show_question() { # $1 - title, $2 - message, $3 - ok label, $4 - cancel label
  killall jci-dialog > /dev/null 2>&1
  log_message "= POPUP: $*\n"
  jci-dialog --question --title="$1" --text="$2" --ok-label="$3" --cancel-label="$4"
}

#============================= INSTALLATION HELPERS

check_mount_point() {
  if [ -f "${MYDIR}/installer_log.txt" ]; then
    log_message "OK\n"
  else
    log_message "FAILED!\n"
    show_error "ERROR!" "Mount point not found, have to reboot again."
    sleep 1
    reboot
    exit
  fi
}

disable_watchdog_and_remount() {
  echo 1 >/sys/class/gpio/Watchdog\ Disable/value || {
    log_message "Disabling Watchdog failed"
    return 1
  }
  mount -o rw,remount / || {
    log_message "Remounting root filesystem failed"
    return 1
  }
  return 0
}

install_openauto() {
  wifi=$1
  log_message "Wifi ${wifi}\n"
  parameters="--"

  [ "${wifi}" -eq 1 ] && parameters=$parameters" -w"
  log_message "${parameters}\n"
  log_message "Running Installer... "
  sh -c "${MYDIR}/openauto_installer.run ${parameters}" >> "${MYDIR}/installer_log.txt"|| {
    log_message "Installing OpenAuto Failed"
    return 1
  }
  log_message "DONE\n"
  return 0
}

remove_openauto() {
  log_message "Removing OpenAuto\n"
  smctl -s -n autoapp
  if [ -f /mnt/data_persist/dev/bin/autoapp.uninstall ]; then
    ash /mnt/data_persist/dev/bin/autoapp.uninstall
  else
    mv /mnt/data_persist/dev/backup/jci/* /jci/
    rm -rf \
            /jci/gui/apps/_androidauto/ \
            /jci/opera/opera_dir/userjs/additionalApps.js \
            /jci/opera/opera_dir/userjs/additionalAppsadditionalApps.json \
            /mnt/data_persist/dev/bin/autoapp \
            /mnt/data_persist/dev/bin/aa_installer \
            /mnt/data_persist/dev/bin/headunit_libs \
            /mnt/data_persist/dev/backup
  fi
}

remove_headunit() {
  if killall -q -9 headunit; then
    log_message "headunit KILLED\n"
  else
    log_message "FAILED! No 'headunit' process found or could not kill it.\n"
  fi
  log_message "Reverting opera.ini file ... "
  reverted=0
  # -- Revert /jci/opera/opera_home/opera.ini from backup
  if [ -f /jci/opera/opera_home/opera.ini.org ]; then
    log_message "from backup ... "
    if mv -a /jci/opera/opera_home/opera.ini.org /jci/opera/opera_home/opera.ini; then
      log_message "DONE\n"
      reverted=1
    else
      log_message "FAILED ... trying the same "
    fi
  fi
  if [ ${reverted} -eq 0 ]; then
    log_message "by reverting changes ... "

    if sed -i 's/User JavaScript=1/User JavaScript=0/g' /jci/opera/opera_home/opera.ini &&
             sed -i 'Allow File XMLHttpRequest=1/d' /jci/opera/opera_home/opera.ini; then
      log_message "DONE\n"
    else
      log_message "FAILED\n"
    fi
  fi
  #Revert fps since once we try to match original state of JCI
  if [ -f /jci/opera/opera_dir/userjs/fps.js.bak ]; then
    log_message "Reverting fps.js from backup ... "
    if mv /jci/opera/opera_dir/userjs/fps.js.bak /jci/opera/opera_dir/userjs/fps.js; then
      log_message "DONE\n"
    else
      log_message "FAILED\n"
    fi
  fi
  if grep -Fq "# Android Auto start" /jci/scripts/stage_wifi.sh; then
    log_message "Reverting stage_wifi.sh ... "
    reverted=0
    if [ -f "/jci/scripts/stage_wifi.sh.bak" ]; then
      log_message " from backup ... "
      if mv /jci/scripts/stage_wifi.sh.bak /jci/scripts/stage_wifi.sh; then
        log_message "DONE\n"
        reverted=1
      else
        log_message "FAILED ... trying the same "
      fi
    fi
    if [ ${reverted} -eq 0 ]; then
      log_message "by reverting changes ... "
      if sed -i '/# Android Auto start/d' /jci/scripts/stage_wifi.sh &&
                 sed -i '/headunit-wrapper/d' /jci/scripts/stage_wifi.sh &&
                 sed -i '/check-usb.sh/d' /jci/scripts/stage_wifi.sh; then
        log_message "DONE\n"
      else
        log_message "FAILED\n"
      fi
    fi
  fi
  # Remove Android Auto Headunit App
  log_message "Removing AA files ... "
  rm -rf /tmp/mnt/data_persist/dev/bin/headunit \
         /tmp/mnt/data_persist/dev/bin/headunit-wrapper \
         /tmp/mnt/data_persist/dev/bin/headunit.json \
         /tmp/mnt/data_persist/dev/bin/check-usb.sh \
         /tmp/mnt/data_persist/dev/bin/headunit_libs \
         /jci/gui/apps/_androidauto \
         /jci/opera/opera_dir/userjs/additionalApps.*
  log_message "DONE\n"
}

#============================= INSTALLATION STARTS HERE

log_message "Installer started.\n"

log_message "MYDIR = ${MYDIR}\n"
log_message "CMU version = ${CMU_SW_VER}\n"

# first test, if copy from MZD to sd card is working to test correct mount point
log_message "Check mount point ... "
check_mount_point

# ask if proceed with installation
if ! show_question "AA INSTALL SCRIPT" "Welcome to OpenAuto installation script.\n Would you like to proceed?" "Proceed" "Abort"; then
  log_message "Installation aborted.\n"
  show_message "Aborted" "Script aborted. Please remove the USB drive. There is no need to reboot."
  exit
fi

# disable watchdog and allow write access
log_message "Disabling watchdog and remounting for write access ... "
if disable_watchdog_and_remount; then
  log_message "SUCCESS\n"
else
  log_message "FAILED\n"
  show_error "ERROR!" "Could not disable watchdog or remount filesystem. Rebooting."
  sleep 1
  reboot
  exit 1
fi

installed=0
remove=0
# check whether we have AA already installed
# check for headunit-wrapper to see if headunit was installed
if [ -f headunit-wrapper ]; then
  log_message "YES\n"
  show_question "CHOOSE ACTION" "You have headunit already installed.\n Would you like to remove it" "REMOVE" "CANCEL"
  remove=$?
  if [ ${remove} -eq 1 ]; then
    log_message "Canceled.\n"
  else
    log_message "Removing Headunit.\n"
    remove_headunit
  fi
else
  log_message "headunit not detected\n"
  log_message "Installing OpenAuto.\n"
fi

log_message "Check whether OpenAuto is installed ... "
if [ -f /tmp/mnt/data_persist/dev/bin/autoapp ]; then
  installed=1
  log_message "YES\n"
  show_question "CHOOSE ACTION" "You have OpenAuto already installed.\n Would you like to update or remove it?" "UPDATE" "REMOVE"
  remove=$?
  if [ ${remove} -eq 1 ]; then
    log_message "Removing OpenAuto.\n"
  else
    log_message "Updating OpenAuto.\n"
  fi
  log_message "Stopping AutoApp ... "
  smctl -n autoapp -s -w
else
  log_message "NO\n"
  log_message "Installing OpenAuto.\n"
fi

if [ ${remove} -eq 0 ]; then
    show_question "WiFi" "Enable WiFi" "Yes" "No"
    wifi=$?
fi

if [ ${installed} -eq 0 ]; then
  # this is an installation path - installed=false
  show_message "INSTALLING" "OpenAuto is installing ..." &

  if install_openauto ${wifi}; then
    log_message "Installation complete!\n"
    show_confirmation "DONE" "OpenAuto has been installed. System will reboot now.\n Remember to remove USB drive."
  else
    log_message "Installation Failed!\n"
    show_confirmation "FAILED" "OpenAuto Failed to install"
  fi
elif [ ${remove} -eq 1 ]; then
  # this is a removing path - installed=true, remove=true
  show_message "UNINSTALLING" "OpenAuto is uninstalling ..."

  remove_openauto

  log_message "Uninstall complete!\n"
  show_confirmation "DONE" "Uninstall complete. System will reboot now.\n Remember to remove USB drive."
else
  # this is an update path - installed=true, remove=false
  show_message "UPDATING" "Android Auto is updating ..." &

  if install_openauto ${wifi}; then
    log_message "Update complete!\n"
    show_confirmation "DONE" "OpenAuto has been Updated. System will reboot now.\n Remember to remove USB drive."
  else
    log_message "Updated Failed!\n"
    show_confirmation "FAILED" "OpenAuto Failed to update"
  fi

  log_message "Update complete!\n"
fi

sleep 3
reboot
exit
