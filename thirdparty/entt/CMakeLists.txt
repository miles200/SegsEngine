# SegsEngine: removed docs , testing code and conan

#
# EnTT
#

cmake_minimum_required(VERSION 3.7.2)

#
# Building in-tree is not allowed (we take care of your craziness).
#

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there. Thank you.")
endif()

#
# Project configuration
#

project(EnTT VERSION 3.2.2 LANGUAGES CXX)

include(GNUInstallDirs)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(SETTINGS_ORGANIZATION "Michele Caini")
set(SETTINGS_APPLICATION ${PROJECT_NAME})
set(PROJECT_AUTHOR "Michele Caini")
set(PROJECT_AUTHOR_EMAIL "michele.caini@gmail.com")

message("*")
message("* ${PROJECT_NAME} v${PROJECT_VERSION} (${CMAKE_BUILD_TYPE})")
message("* Copyright (c) 2017-2019 ${PROJECT_AUTHOR} <${PROJECT_AUTHOR_EMAIL}>")
message("*")

option(USE_LIBCPP "Use libc++ by adding -stdlib=libc++ flag if availbale." ON)
option(USE_ASAN "Use address sanitizer by adding -fsanitize=address -fno-omit-frame-pointer flags" OFF)
option(USE_COMPILE_OPTIONS "Use compile options from EnTT." ON)

#
# Compiler stuff
#

if(NOT WIN32 AND USE_LIBCPP)
    include(CheckCXXSourceCompiles)
    include(CMakePushCheckState)

    cmake_push_check_state()

    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -stdlib=libc++")

    check_cxx_source_compiles("
        #include<type_traits>
        int main() { return std::is_same_v<int, char>; }
    " HAS_LIBCPP)

    if(NOT HAS_LIBCPP)
        message(WARNING "The option USE_LIBCPP is set (by default) but libc++ is not available. The flag will not be added to the target.")
    endif()

    cmake_pop_check_state()
endif()

#
# Add EnTT target
#

add_library(EnTT INTERFACE)
add_library(EnTT::EnTT ALIAS EnTT)

configure_file(${EnTT_SOURCE_DIR}/cmake/in/version.h.in ${EnTT_SOURCE_DIR}/src/entt/config/version.h @ONLY)

target_include_directories(
    EnTT INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_definitions(
    EnTT
    INTERFACE $<$<AND:$<CONFIG:Debug>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:DEBUG>
    INTERFACE $<$<AND:$<CONFIG:Release>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:RELEASE>
)

if(USE_ASAN)
    target_compile_options(EnTT INTERFACE $<$<AND:$<CONFIG:Debug>,$<NOT:$<PLATFORM_ID:Windows>>>:-fsanitize=address -fno-omit-frame-pointer>)
    target_link_libraries(EnTT INTERFACE $<$<AND:$<CONFIG:Debug>,$<NOT:$<PLATFORM_ID:Windows>>>:-fsanitize=address -fno-omit-frame-pointer>)
endif()

if(USE_COMPILE_OPTIONS)
    target_compile_options(
        EnTT
        INTERFACE $<$<AND:$<CONFIG:Debug>,$<NOT:$<PLATFORM_ID:Windows>>>:-O0 -g>
        # it seems that -O3 ruins a bit the performance when using clang ...
        INTERFACE $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:Clang>,$<OR:$<PLATFORM_ID:Darwin>,$<PLATFORM_ID:Linux>>>:-O2>
        # ... on the other side, GCC is incredibly comfortable with it.
        INTERFACE $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU>>:-O3>
    )
endif()

if(HAS_LIBCPP)
    target_compile_options(EnTT BEFORE INTERFACE -stdlib=libc++)
endif()

target_compile_features(EnTT INTERFACE cxx_std_17)

#
# Install EnTT
#

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(CUSTOM_INSTALL_CONFIGDIR cmake)
else()
    set(CUSTOM_INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/entt)
endif()

install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS EnTT EXPORT EnTTTargets)

export(EXPORT EnTTTargets FILE ${EnTT_BINARY_DIR}/EnTTTargets.cmake)

install(
    EXPORT EnTTTargets
    FILE EnTTTargets.cmake
    DESTINATION ${CUSTOM_INSTALL_CONFIGDIR}
    NAMESPACE EnTT::
)

#
# Build tree package config file
#

configure_file(cmake/in/EnTTBuildConfig.cmake.in EnTTConfig.cmake @ONLY)

include(CMakePackageConfigHelpers)

#
# Install tree package config file
#

configure_package_config_file(
    cmake/in/EnTTConfig.cmake.in
    ${CUSTOM_INSTALL_CONFIGDIR}/EnTTConfig.cmake
    INSTALL_DESTINATION ${CUSTOM_INSTALL_CONFIGDIR}
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR
)

write_basic_package_version_file(
    ${EnTT_BINARY_DIR}/EnTTConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(
    FILES
        ${EnTT_BINARY_DIR}/${CUSTOM_INSTALL_CONFIGDIR}/EnTTConfig.cmake
        ${EnTT_BINARY_DIR}/EnTTConfigVersion.cmake
    DESTINATION ${CUSTOM_INSTALL_CONFIGDIR}
)

export(PACKAGE EnTT)


#
# AOB
#

#FILE(GLOB GH_WORKFLOWS .github/workflows/*.yml)

#add_custom_target(
#    entt_aob
#    SOURCES
#        ${GH_WORKFLOWS}
#        .github/FUNDING.yml
#        AUTHORS
#        CONTRIBUTING.md
#        LICENSE
#        README.md
#        TODO
#)
