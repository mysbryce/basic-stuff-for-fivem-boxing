#include <iostream>
#include <Windows.h>
#include <vector>
#include <TlHelp32.h>

HANDLE OpenProcessById(DWORD processId) {
    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (handle == NULL) {
        return 0;
    }
    return handle;
}

uintptr_t GetModuleBaseAddress(HANDLE handle, const wchar_t* moduleName) {
    MODULEENTRY32W moduleEntry;
    moduleEntry.dwSize = sizeof(MODULEENTRY32W);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetProcessId(handle));

    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    uintptr_t moduleBaseAddress = 0;

    if (Module32FirstW(hSnapshot, &moduleEntry)) {
        do {
            if (wcscmp(moduleEntry.szModule, moduleName) == 0) {
                moduleBaseAddress = reinterpret_cast<uintptr_t>(moduleEntry.modBaseAddr);
                break;
            }
        } while (Module32NextW(hSnapshot, &moduleEntry));
    }

    CloseHandle(hSnapshot);

    if (moduleBaseAddress == 0) {
        return 0;
    }

    return moduleBaseAddress;
}

uintptr_t FindPtrAddress(HANDLE handle, uintptr_t address, const std::vector<int>& offsets) {
    for (int offset : offsets) {
        ReadProcessMemory(handle, reinterpret_cast<LPCVOID>(address), &address, 8, nullptr);
        address += offset;
    }

    return address;
}

template<typename T>
T ReadProcessMemoryAsTarget(HANDLE handle, uintptr_t address) {
    T value;
    ReadProcessMemory(handle, reinterpret_cast<LPCVOID>(address), &value, sizeof(value), nullptr);
    return value;
}

template<typename T>
void optionList(const wchar_t* name, HANDLE handle, uintptr_t address, void (*func)()) {
    system("cls");

    T value = ReadProcessMemoryAsTarget<T>(handle, address);

    std::wcout << "\n\n\t\x1B[96m" << name << ":\033[0m" << std::endl;
    std::cout << "\t    \x1B[96m[Address]\033[0m \x1B[90m0x" << address << "\033[0m" << std::endl;
    std::cout << "\t    \x1B[96m[Parent Value]\033[0m \x1B[90m" << value << "\033[0m\n" << std::endl;

    for (;;) {
        std::cout << "\t    \x1B[90mNew Value:\033[0m ";
        if (std::cin >> value)
            break;
        std::cin.clear();
        std::cin.ignore(512, '\n');
    }

    SetLastError(0);
    WriteProcessMemory(handle, (LPVOID)address, &value, sizeof(value), NULL);

    func();
}

int GetProcessIDFromName(const wchar_t* procname) {
    HANDLE hSnapshot;
    PROCESSENTRY32 pe;
    int pid = 0;
    BOOL hResult;

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == hSnapshot) return 0;

    pe.dwSize = sizeof(PROCESSENTRY32);
    hResult = Process32First(hSnapshot, &pe);

    while (hResult) {
        if (wcscmp(procname, pe.szExeFile) == 0) {
            pid = pe.th32ProcessID;
            break;
        }
        hResult = Process32Next(hSnapshot, &pe);
    }

    CloseHandle(hSnapshot);
    return pid;
}

void mainScreen() {
    system("cls");
    const wchar_t* moduleName = L"FiveM_GTAProcess.exe";
    DWORD processId = GetProcessIDFromName(moduleName);
    HANDLE handle = OpenProcessById(processId);
    int option;

    if (handle != NULL) {
        std::cout << "\n\n\t    \x1B[96mBasic Stuff For Playing Pro (Coded by Bryce Conquers)\033[0m \n" << std::endl;
        std::cout << "\t    \x1B[96m[1]\033[0m \x1B[90mPunch\033[0m" << std::endl;
        std::cout << "\t    \x1B[96m[2]\033[0m \x1B[90mBodyFlow\033[0m" << std::endl;
        std::cout << "\t    \x1B[96m[3]\033[0m \x1B[90mHealth\033[0m" << std::endl;
        std::cout << "\t    \x1B[96m[4]\033[0m \x1B[90mArmor\033[0m" << std::endl;
        std::cout << "\t    \x1B[96m[5]\033[0m \x1B[90mSprint\033[0m" << std::endl;
        std::cout << "\t    \x1B[96m[6]\033[0m \x1B[90mGodMode\033[0m \n" << std::endl;

        for (;;) {
            std::cout << "\t    \x1B[90mSelect Option:\033[0m ";
            if (std::cin >> option)
                break;
            std::cin.clear();
            std::cin.ignore(512, '\n');
        }

        if (option >= 1 && option <= 6) {
            uintptr_t moduleBaseAddress = GetModuleBaseAddress(handle, moduleName);

            if (option == 1) {
                optionList<float>(L"Punch", handle, FindPtrAddress(handle, moduleBaseAddress + 0x0212B208, { 0x60, 0x20, 0x70 }), mainScreen);
            }
            else if (option == 2) {
                optionList<float>(L"BodyFlow", handle, FindPtrAddress(handle, moduleBaseAddress + 0x0212B208, { 0x0, 0x7D8, 0x0, 0x0, 0x10, 0x1AC }), mainScreen);
            }
            else if (option == 3) {
                optionList<float>(L"Health", handle, FindPtrAddress(handle, moduleBaseAddress + 0x247F840, { 0x8, 0x280 }), mainScreen);
            }
            else if (option == 4) {
                optionList<float>(L"Armor", handle, FindPtrAddress(handle, moduleBaseAddress + 0x247F840, { 0x8, 0x14B8 }), mainScreen);
            }
            else if (option == 5) {
                optionList<float>(L"Sprint", handle, FindPtrAddress(handle, moduleBaseAddress + 0x247F840, { 0x8, 0x10B8, 0x14C }), mainScreen);
            }
            else if (option == 6) {
                optionList<bool>(L"GodMode", handle, FindPtrAddress(handle, moduleBaseAddress + 0x247F840, { 0x8, 0x189 }), mainScreen);
            }
        }
        else {
            mainScreen();
        }

        CloseHandle(handle);
    }
}

int main() {
    mainScreen();

    return 0;
}
