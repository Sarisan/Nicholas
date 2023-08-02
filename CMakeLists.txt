cmake_minimum_required(VERSION 3.10)

project(Nicholas VERSION 0.2 LANGUAGES C)

configure_file(version.h.in version.h)

option(WALL "Enable -Wall compiler option" ON)
option(WEXTRA "Enable -Wextra compiler option" ON)
option(WEVERYTHING "Enable -Weverything compiler option" OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_library(CURL_LIBRARY curl)
find_library(JSON_C_LIBRARY json-c)

if(NOT CURL_LIBRARY)
  message(FATAL_ERROR "curl library not found")
endif()

if(NOT JSON_C_LIBRARY)
  message(FATAL_ERROR "json-c library not found")
endif()

if(WALL)
  include(CheckCCompilerFlag)
  check_c_compiler_flag(-Wall HAVE_WALL)

  if(HAVE_WALL)
    add_compile_options(-Wall)
  else()
    message(FATAL_ERROR "-Wall is not supported by the compiler")
  endif()
endif()

if(WEXTRA)
  include(CheckCCompilerFlag)
  check_c_compiler_flag(-Wextra HAVE_WEXTRA)

  if(HAVE_WEXTRA)
    add_compile_options(-Wextra)
  else()
    message(FATAL_ERROR "-Wextra is not supported by the compiler")
  endif()
endif()

if(WEVERYTHING)
  include(CheckCCompilerFlag)
  check_c_compiler_flag(-Weverything HAVE_WEVERYTHING)

  if(HAVE_WEVERYTHING)
    add_compile_options(-Weverything)
  else()
    message(FATAL_ERROR "-Weverything is not supported by the compiler")
  endif()
endif()

set(API_SOURCE
  api/api_io.c
  api/update.c
  api/username.c)

set(CONFIG_SOURCE
  config/config.c)

set(DEBUG_SOURCE
  debug/debug.c)

set(INIT_SOURCE 
  init/help.c
  init/init.c
  init/main.c)

set(JSON_SOURCE
  json/json.c)

set(LEGACY_SOURCE
  legacy/bot_callback.c
  legacy/bot_commands.c
  legacy/bot_inline.c)

set(PARSERS_SOURCE
  parsers/callback.c
  parsers/command.c
  parsers/inline.c)

set(SANKAKU_SOURCE
  sankaku/api_io.c
  sankaku/legacy_parsers.c)

set(STRING_SOURCE
  string/string.c)

add_executable(nicholas
  ${API_SOURCE}
  ${CONFIG_SOURCE}
  ${DEBUG_SOURCE}
  ${INIT_SOURCE}
  ${JSON_SOURCE}
  ${LEGACY_SOURCE}
  ${PARSERS_SOURCE}
  ${SANKAKU_SOURCE}
  ${STRING_SOURCE}) 

target_include_directories(nicholas PUBLIC
  "${PROJECT_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}")

target_link_libraries(nicholas PUBLIC curl json-c)

install(TARGETS nicholas DESTINATION bin)
