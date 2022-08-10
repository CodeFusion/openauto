# Mazda-OpenAuto

[![Join the chat at https://gitter.im/mazda_openauto/community](https://badges.gitter.im/mazda_openauto/community.svg)](https://gitter.im/mazda_openauto/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

### Description

Mazda-OpenAuto is an AndroidAuto(tm) headunit emulator based on aasdk library, and minimal external dependencies.

It is forked from the original [f1xpl/openauto](https://github.com/f1xpl/openauto) with some fixes ported
from [openDsh/openauto](https://github.com/openDsh/openauto)
and [opencardev/openauto](https://github.com/opencardev/openauto)

Code to interace with the Mazda CMU has been ported from [headunit](https://github.com/gartnera/headunit).

### Supported functionalities

- 480p video
- Audio playback from all audio channels (Media, System and Speech)
- Audio input for voice commands
- Touchscreen and buttons input
- Bluetooth
- Automatic launch after device hotplug
- Automatic detection of connected Android devices

### Supported platforms

- Mazda CMU based on the NXP i.MX6 platform, running linux. Tested on firmware  "MAZ_CMU-150_59.00.546"

### License

GNU GPLv3

Copyrights (c) 2020 silverchris (Chris Schimp)

Copyrights (c) 2018 f1x.studio (Michal Szwaj)

*AndroidAuto is registered trademark of Google Inc.*

### Used software

- [aasdk](https://github.com/f1xpl/aasdk)
- [easylogging++](https://github.com/amrayn/easyloggingpp)
- [JSON for Modern C++](https://github.com/nlohmann/json)
- [SIGC++](https://github.com/libsigcplusplus/libsigcplusplus)
- [dbus]()
- [dbus-c++]()
- [CMake](https://cmake.org/)

### Remarks

**This software is not certified by Google Inc. It is created for R&D purposes and may not work as expected by the
original authors. Do not use while driving. You use this software at your own risk.**
