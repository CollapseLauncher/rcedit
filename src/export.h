#pragma once
#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)

EXTERN_DLL_EXPORT int ChangeExecutablePermission(wchar_t* path, wchar_t* permissionType);
