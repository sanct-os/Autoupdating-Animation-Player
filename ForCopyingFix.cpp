// This file is incase you can't find AnimPlayer.cpp in the project and the project doesn't run/work for you
// Incase it doesn't work, you need to make your own visual studio c++ file and paste this files source code into your own c++ file
// Make sure visual studios settings will work with this


// Terrible C++ External RBX Animation Player //

// made by: adam/sanct.os //

// use in r6 games only! //
// it's possible to make the code work in r15 if you're smart enough //
// this project is old and only recently revamped //

// if you're gonna use some of my code for your own project, credit me please (adam | sanct.os on discord).


#include <windows.h>
#include <vector>
#include <iostream>
#include <TlHelp32.h>
#include <tchar.h>
#include <sstream>

auto proc_get(const TCHAR* rbx_name) -> DWORD {
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    auto snap_proc = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Process32First(snap_proc, &pe)) {
        do {
            if (_tcsicmp(pe.szExeFile, rbx_name) == 0) {
                CloseHandle(snap_proc);
                return pe.th32ProcessID;
            }
        } while (Process32Next(snap_proc, &pe));
    }
    CloseHandle(snap_proc);
    return 0;
}

auto read_mem(HANDLE proc, LPCVOID base_addy, void* buffer, SIZE_T size) -> bool {
    return ReadProcessMemory(proc, base_addy, buffer, size, nullptr);
}

auto write_mem(HANDLE proc, LPVOID base_addy, LPCVOID buffer, SIZE_T size) -> bool {
    return WriteProcessMemory(proc, base_addy, buffer, size, nullptr);
}

auto steal_localplayer_anim_script_ids(HANDLE proc, std::vector<LPVOID>& known_addys, const std::vector<std::string>& targets) -> void {
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    auto start_addy = sys_info.lpMinimumApplicationAddress;
    auto end_addy = sys_info.lpMaximumApplicationAddress;

    MEMORY_BASIC_INFORMATION mbi;
    while (start_addy < end_addy) {
        if (VirtualQueryEx(proc, start_addy, &mbi, sizeof(mbi)) == sizeof(mbi)) {
            if (mbi.State == MEM_COMMIT && (mbi.Protect & (PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY))) {
                SIZE_T buffer_size = mbi.RegionSize;
                if (buffer_size > 1024 * 1024) {
                    buffer_size = 1024 * 1024;
                }
                std::vector<char> buffer(buffer_size);

                if (read_mem(proc, mbi.BaseAddress, buffer.data(), buffer_size)) {
                    for (const auto& target : targets) {
                        for (SIZE_T i = 0; i < buffer_size - target.size(); ++i) {
                            if (memcmp(buffer.data() + i, target.c_str(), target.size()) == 0) {
                                auto addy = reinterpret_cast<LPVOID>(reinterpret_cast<std::uintptr_t>(mbi.BaseAddress) + i);
                                known_addys.push_back(addy);
                                if (known_addys.size() >= 50000) return; // the 50k is just something i dont feel like changing to what it really should be, but it doesn't effect anything but do not change it unless you know what you are doing
                            }
                        }
                    }
                }
            }
            start_addy = reinterpret_cast<LPVOID>(reinterpret_cast<std::uintptr_t>(start_addy) + mbi.RegionSize);
        }
    }
}

auto load_anim(HANDLE proc, const std::vector<LPVOID>& addys, const std::string& new_id) -> void {
    for (const auto& addy : addys) {
        DWORD older_protect;
        if (VirtualProtectEx(proc, addy, new_id.size(), PAGE_EXECUTE_READWRITE, &older_protect)) {
            if (write_mem(proc, addy, new_id.c_str(), new_id.size())) {
                
            }
            else {
                std::cerr << "[ ! error ! ] unable to write memory to the original animation address at -> " << addy << std::endl;
            }
            if (!VirtualProtectEx(proc, addy, new_id.size(), older_protect, &older_protect)) {
                std::cerr << "[ ! error ! ] cant restore memory protection at this address -> " << addy << std::endl;
            }
        }
        else {
            std::cerr << "[ ! error ! ] cannot change memory protection at -> " << addy << std::endl;
        }
    }
}

void set_c_title(const std::string& title) {
    std::wstring lol(title.begin(), title.end());
    SetConsoleTitle(lol.c_str());
}

auto handle_anim(HANDLE proc, int anim_id) -> void {
    std::vector<LPVOID> anim_addys;
    // roblox built in default R6 animations that will get changed based on your input to play the new animation
    std::vector<std::string> targets = {
        // im sorry i forgot to write which animation is which on your character but i dont feel like finding it again now either
        "http://www.roblox.com/asset/?id=180426354",
        "http://www.roblox.com/asset/?id=180435571",
        "http://www.roblox.com/asset/?id=180435792",
        "http://www.roblox.com/asset/?id=125750702",
        "http://www.roblox.com/asset/?id=180436148",
        "http://www.roblox.com/asset/?id=180436334",
    };
    steal_localplayer_anim_script_ids(proc, anim_addys, targets);

    if (anim_addys.empty()) {
        std::cerr << "[ ! error ! ] no animation addresses found or the code did something wrong?" << std::endl;
        return;
    }

    std::ostringstream oss;
    oss << "http://www.roblox.com/asset/?id=" << anim_id;
    std::string replace_id = oss.str();

    load_anim(proc, anim_addys, replace_id);
    std::cout << "[ ! complete ! ] done loading animation, walk around to activate it\n";
}

auto main() -> int {
    set_c_title("External Animation Player | by: @sanct.os (adam)");
    HANDLE h_console = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(h_console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << "============ Main ============\n" << std::endl;
    std::cout << "** created by @sanct.os (adam)\n" std::endl;
    std::cout << "-- you'll have to find a list of roblox made r6 animations to use it, you cannot use animations made by others, it wont work\n" << std::endl;
    HWND console_win = GetConsoleWindow();
    if (console_win != nullptr) {
        SetWindowPos(console_win, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    }

    const TCHAR* proc_rbx = _T("RobloxPlayerBeta.exe");
    auto pid = proc_get(proc_rbx);

    if (pid == 0) {
        std::cerr << "[ ! error ! ]  roblox is not open. open it then run the program again (make sure your on web app and not uwp)" << std::endl;
        return 1;
    }

    auto open_proc = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);

    if (open_proc == nullptr) {
        std::cerr << "[ ! error ! ]  failed to openprocess()." << std::endl;
        return 1;
    }

    while (true) {
        std::cout << "[ ! TIP ! ] note: you can only use Roblox made animations. it cannot be an animation created by another user.\n";
        std::cout << "[ ! TIP ! ] you can set the animation id to the /e dance animation id if you want to /e dance while moving or whatever.\n\n";
        std::cout << "----------------------------------------------------\n\n";
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
        std::cout << "[*] enter new animation ID: ";
        std::string input;
        std::getline(std::cin, input);

        try {
            int id = std::stoi(input);
            std::cout << "[ ! INFO ! ] just a little warning, this legit takes a long time to load depending on your pc (like 30 seconds to 3 minutes), its pretty shit but you can use this code to learn from or if your cheat is patched\n";
            handle_anim(open_proc, id);
        }
        catch (const std::invalid_argument&) {
            std::cerr << "[ ! error ! ] input needs to be numbers for the animation id (example: 1234567890) \n";
        }
        CloseHandle(open_proc);
        return 0;
    }
}
