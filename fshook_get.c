
#include <windows.h>
#include <stdio.h>
#include <string.h>

static inline size_t get_filename_without_extension(char* path) {
    GetModuleFileNameA(NULL, path, MAX_PATH);

    // 提取文件名部分
    char* last_slash = strrchr(path, '\\');
    char* filename = (last_slash != NULL) ? (last_slash + 1) : path;

    // 移除扩展名
    char* last_dot = strrchr(filename, '.');
    if (last_dot != NULL && last_dot != filename) {  // 确保不是隐藏文件（以点开头的文件）
        *last_dot = '\0';
    }

    return last_dot - path;
}

static DWORD WINAPI ReadFlagThread(LPVOID lpParameter) {
    Sleep(1000); // 等待1秒

    char path[MAX_PATH];
    size_t len1 = get_filename_without_extension(path);
    char b[] = "_Data\\level0";
    size_t len2 = strlen(b);

    strncat(path + len1, b, MAX_PATH - 1);
    path[len1 + len2] = '\0'; // 再次确保终止符

    HANDLE hFile = CreateFileA(
        path,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Failed to open %s\n", path);
        return 1;
    }

    HANDLE hOutput = CreateFileA(
        "_level0",
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hOutput == INVALID_HANDLE_VALUE) {
        printf("Failed to open _level0\n");
        goto fin2;
    }

    CHAR buffer[4096];

    DWORD bytesRead;
    while(1) {
        if (!ReadFile(hFile, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
            printf("Failed to read level0\n");
            goto fin;
        }

        if (!bytesRead) break;

        DWORD bytesWritten;
        WriteFile(hOutput, buffer, bytesRead, &bytesWritten, NULL);
    }

    fin:
    CloseHandle(hOutput);
    fin2:
    CloseHandle(hFile);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        // 创建线程以避免阻塞DllMain
        CreateThread(NULL, 0, ReadFlagThread, NULL, 0, NULL);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

void APIENTRY UnityMain(void) {}