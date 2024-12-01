// Copyright (c) 2013 GitHub, Inc. All rights reserved.
// Use of this source code is governed by MIT license that can be found in the
// LICENSE file.

#include <string.h>
#include <vector>

#include <windows.h>
#include <winver.h>

#include "rescle.h"
#include "export.h"

namespace {

std::vector<uint8_t> get_file_version_info() {
  DWORD zero = 0;
  std::vector<wchar_t> filename(MAX_PATH);
  SetLastError(ERROR_SUCCESS);

  do {
    GetModuleFileNameW(NULL, filename.data(), filename.size());

    // Double the buffer size in case the path is longer
    filename.resize(filename.size() * 2);
  } while (GetLastError() == ERROR_INSUFFICIENT_BUFFER);

  if (GetLastError() != ERROR_SUCCESS) {
    return std::vector<uint8_t>();
  }

  DWORD file_ver_info_size = GetFileVersionInfoSizeW(filename.data(), &zero);
  if (file_ver_info_size == 0) {
    return std::vector<uint8_t>();
  }

  std::vector<uint8_t> file_ver_info(file_ver_info_size);
  if (!GetFileVersionInfoW(filename.data(), NULL, file_ver_info.size(), file_ver_info.data())) {
    return std::vector<uint8_t>();
  }

  return file_ver_info;
}

void print_help(VS_FIXEDFILEINFO* file_info) {
  fprintf(stdout,
"Rcedit v%d.%d.%d: Edit resources of exe.\n\n"
"Usage: rcedit <filename> [options...]\n\n"
"Options:\n"
"  -h, --help                                 Show this message\n"
"  --set-version-string <key> <value>         Set version string\n"
"  --get-version-string <key>                 Print version string\n"
"  --set-file-version <version>               Set FileVersion\n"
"  --set-product-version <version>            Set ProductVersion\n"
"  --set-icon <path-to-icon>                  Set file icon\n"
"  --set-requested-execution-level <level>    Pass nothing to see usage\n"
"  --application-manifest <path-to-file>      Set manifest file\n"
"  --set-resource-string <key> <value>        Set resource string\n"
"  --get-resource-string <key>                Get resource string\n"
"  --set-rcdata <key> <path-to-file>          Replace RCDATA by integer id\n",
(file_info->dwProductVersionMS >> 16) & 0xff,
(file_info->dwProductVersionMS >>  0) & 0xff,
(file_info->dwProductVersionLS >> 16) & 0xff);
}

bool print_error(const char* message) {
  fprintf(stderr, "Fatal error: %s\n", message);
  return 1;
}

bool print_warning(const char* message) {
  fprintf(stderr, "Warning: %s\n", message);
  return 1;
}

bool parse_version_string(const wchar_t* str, unsigned short *v1, unsigned short *v2, unsigned short *v3, unsigned short *v4) {
  *v1 = *v2 = *v3 = *v4 = 0;
  return (swscanf_s(str, L"%hu.%hu.%hu.%hu", v1, v2, v3, v4) == 4) ||
         (swscanf_s(str, L"%hu.%hu.%hu", v1, v2, v3) == 3) ||
         (swscanf_s(str, L"%hu.%hu", v1, v2) == 2) ||
         (swscanf_s(str, L"%hu", v1) == 1);
}

}  // namespace

int ChangeExecutablePermission(wchar_t* path, wchar_t* permissionType)
{
  bool loaded = false;
  rescle::ResourceUpdater updater;

  if (!updater.SetExecutionLevel(permissionType))
  {
    fprintf(stderr, "Unable to set execution level. Valid execution level: asInvoker, highestAvailable or requireAdministrator\n");
    return 2;
  }

  if (!updater.Load(path))
  {
    fprintf(stderr, "Unable to load file: %s\n", path);
    return 3;
  }

  if (!updater.Commit())
  {
    fprintf(stderr, "Unable to apply update for: %s\n", path);
    return 1;
  }

  return 0;
}
