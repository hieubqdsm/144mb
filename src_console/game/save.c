/* =====================================================================
   SAVE/LOAD - Implementation (binary, versioned)
   ===================================================================== */
#include "save.h"
#include <windows.h>
#include <string.h>

#define SAVE_MAGIC 0x52414745   /* "GAME" */
#define SAVE_VERSION 1

int save_game(const char *path, const SaveData *data){
    HANDLE h = CreateFileA(path, GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(h == INVALID_HANDLE_VALUE) return 0;
    DWORD wr;
    int magic = SAVE_MAGIC, ver = SAVE_VERSION;
    WriteFile(h, &magic, sizeof(magic), &wr, NULL);
    WriteFile(h, &ver, sizeof(ver), &wr, NULL);
    WriteFile(h, data, sizeof(SaveData), &wr, NULL);
    CloseHandle(h);
    return 1;
}

int load_game(const char *path, SaveData *data){
    HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(h == INVALID_HANDLE_VALUE) return 0;
    DWORD rd;
    int magic, ver;
    if(!ReadFile(h, &magic, sizeof(magic), &rd, NULL) || magic != SAVE_MAGIC){
        CloseHandle(h); return 0;
    }
    if(!ReadFile(h, &ver, sizeof(ver), &rd, NULL) || ver != SAVE_VERSION){
        CloseHandle(h); return 0;
    }
    int ok = ReadFile(h, data, sizeof(SaveData), &rd, NULL) && rd == sizeof(SaveData);
    CloseHandle(h);
    return ok;
}

int save_exists(const char *path){
    DWORD attr = GetFileAttributesA(path);
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}
