cmake_minimum_required(VERSION 3.5.1)
project(openauto CXX)

set(base_directory ${CMAKE_CURRENT_SOURCE_DIR})
set(sources_directory ${base_directory}/src)
set(include_directory ${base_directory}/include)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${base_directory}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${base_directory}/lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${base_directory}/bin)
set(EXECUTABLE_OUTPUT_PATH ${base_directory}/bin)
file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})

SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
SET(BUILD_SHARED_LIBS OFF)
set(CMAKE_LINK_SEARCH_START_STATIC ON)
set(OPENSSL_USE_STATIC_LIBS TRUE) #Statically link openssl for Mazda target
set(Protobuf_USE_STATIC_LIBS ON) #Statically link protobuf for Mazda target

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")
SET(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_INIT} -Wall -pedantic -Wno-psabi")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O3")
set(CMAKE_CXX_FLAGS_RELEASE "-g -O3")

message("CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}")

find_package(Protobuf REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ALSA REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(asio REQUIRED)
find_package(fdk-aac REQUIRED)
pkg_check_modules(DBUS REQUIRED "dbus-cxx-2.0")
pkg_check_modules(SIGC++ REQUIRED sigc++-3.0)
find_package(libusb-1.0 REQUIRED)
pkg_check_modules(LIBEVDEV REQUIRED libevdev)

SET(AASDK_BUILD_STATIC ON CACHE BOOL "Build Static AASDK")
SET(AASDK_BUILD_SHARED OFF CACHE BOOL "Don't Build Shared AASDK")
SET(AASDK_TEST OFF CACHE BOOL "Don't Test AASDK")
add_subdirectory(external/aasdk)

SET(USE_STANDALONE_ASIO ON CACHE BOOL "use standalone Asio")
SET(USE_OPENSSL OFF CACHE BOOL "Don't need SSL in our webserver")
add_subdirectory(external/Simple-Web-Server)

set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")
add_subdirectory(external/json)

set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(BUILD_TESTS OFF CACHE INTERNAL "")
add_subdirectory(external/tinyxml2)

add_compile_definitions(ELPP_THREAD_SAFE)
add_compile_definitions(ELPP_STL_LOGGING)
add_subdirectory(external/easylogging++)

add_subdirectory(external/toml++)


find_package(Git)
if (GIT_EXECUTABLE)
    execute_process(
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMAND "${GIT_EXECUTABLE}" describe --tags --always
            OUTPUT_VARIABLE GIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(BUILD_HASH ${GIT_HASH})

    if (NOT REPRODUCIBLE)
        execute_process(
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                COMMAND "whoami"
                OUTPUT_VARIABLE GIT_USER
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        execute_process(
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                COMMAND "hostname"
                OUTPUT_VARIABLE GIT_HOST
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        string(REGEX REPLACE "([^\\])[\\]([^\\])" "\\1\\\\\\\\\\2" GIT_USER ${GIT_USER})
        set(BUILD_HASH ${GIT_USER}@${GIT_HOST}-${GIT_HASH})
    endif ()

    message("Git commit hash: ${BUILD_HASH}")

    configure_file(cmake/version.in.h ${CMAKE_CURRENT_BINARY_DIR}/version.h @ONLY)
endif ()

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}
        ${PROTOBUF_INCLUDE_DIR}
        ${OPENSSL_INCLUDE_DIR}
        ${ALSA_INCLUDE_DIRS}
        ${SIGC++_INCLUDE_DIRS}
        ${ASIO_INCLUDE_DIRS}
        ${LIBEVDEV_INCLUDE_DIRS}
        ${DBUS_INCLUDE_DIRS})

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

set(autoapp_sources_directory ${sources_directory}/autoapp)
set(autoapp_include_directory)
file(GLOB_RECURSE autoapp_source_files ${autoapp_sources_directory}/*.cpp)

add_executable(autoapp ${autoapp_source_files})
target_include_directories(autoapp PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/generated/)
target_include_directories(autoapp PRIVATE ${include_directory})

add_subdirectory(src/Mazda/Dbus/com.jci.bca)
target_link_libraries(autoapp com.jci.bca)

add_subdirectory(src/Mazda/Dbus/com.jci.bucpsa)
target_link_libraries(autoapp com.jci.bucpsa)

add_subdirectory(src/Mazda/Dbus/com.jci.nativeguictrl)
target_link_libraries(autoapp com.jci.nativeguictrl)

add_subdirectory(src/Mazda/Dbus/com.jci.aapa)
target_link_libraries(autoapp com.jci.aapa)

add_subdirectory(src/Mazda/Dbus/com.jci.lds.control)
target_link_libraries(autoapp com.jci.lds.control)

add_subdirectory(src/Mazda/Dbus/com.jci.lds.data)
target_link_libraries(autoapp com.jci.lds.data)

add_subdirectory(src/Mazda/Dbus/com.xsembedded.ServiceProvider)
target_link_libraries(autoapp com.xsembedded.ServiceProvider)

add_subdirectory(src/Mazda/Dbus/com.jci.navi2IHU.HUDSettings)
target_link_libraries(autoapp com.jci.navi2IHU.HUDSettings)

add_subdirectory(src/Mazda/Dbus/com.jci.vbs.navi.tmc)
target_link_libraries(autoapp com.jci.vbs.navi.tmc)

add_subdirectory(src/Mazda/Dbus/com.jci.vbs.navi)
target_link_libraries(autoapp com.jci.vbs.navi)

target_compile_options(autoapp PUBLIC "-ffile-prefix-map=${CMAKE_SOURCE_DIR}=.")

target_link_libraries(autoapp aasdk_static)
target_link_libraries(autoapp aasdk_proto_static)
target_link_libraries(autoapp libusb)

target_include_directories(autoapp PRIVATE external/mINI/src/mini)


target_link_libraries(autoapp
        ${SIGC++_LIBRARIES}
        ${ALSA_LIBRARIES}
        ${Protobuf_LITE_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        ${LIBEVDEV_LIBRARIES}
        ${DBUS_LIBRARIES}
        fdk-aac
        simple-web-server
        nlohmann_json::nlohmann_json
        easylogging++
        toml++
        tinyxml2
        -static
        -static-libstdc++
        -static-libgcc
        -pthread
        )

add_custom_command(
        TARGET autoapp
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} --only-keep-debug autoapp autoapp.debug
        WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
)
add_custom_command(
        TARGET autoapp
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} --strip-all autoapp
        WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
)
add_custom_command(
        TARGET autoapp
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=autoapp.debug autoapp
        WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
)


file(MAKE_DIRECTORY ${base_directory}/install/mnt/data_persist/dev/bin/)
add_custom_command(
        TARGET autoapp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${EXECUTABLE_OUTPUT_PATH}/autoapp
        ${base_directory}/install/mnt/data_persist/dev/bin/)


set(installer_sources_directory ${sources_directory}/installer)
set(installer_include_directory)
file(GLOB_RECURSE installer_source_files ${installer_sources_directory}/*.cpp)

add_executable(aa_installer
        ${installer_source_files})
target_include_directories(aa_installer PRIVATE ${include_directories})
target_include_directories(aa_installer PRIVATE external/mINI/src/mini)
target_include_directories(aa_installer PRIVATE ${include_directory})

target_link_libraries(aa_installer
        nlohmann_json::nlohmann_json
        easylogging++
        toml++
        tinyxml2
        -static
        -static-libstdc++
        -static-libgcc
        -pthread
        )

add_custom_command(
        TARGET aa_installer
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} --only-keep-debug aa_installer aa_installer.debug
        WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
)
add_custom_command(
        TARGET aa_installer
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} --strip-all aa_installer
        WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
)
add_custom_command(
        TARGET aa_installer
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=aa_installer.debug aa_installer
        WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
)

add_custom_command(
        TARGET aa_installer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${EXECUTABLE_OUTPUT_PATH}/aa_installer
        ${base_directory}/install/mnt/data_persist/dev/bin/)


add_custom_target(create_tar ALL COMMAND
        external/makeself.sh --target / --notemp --keep-umask --nochown install installer/openauto_installer.run openauto /mnt/data_persist/dev/bin/aa_installer
        WORKING_DIRECTORY ${base_directory})
add_dependencies(create_tar autoapp)
add_dependencies(create_tar aa_installer)
