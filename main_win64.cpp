#define NOMINMAX

#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <mutex>
#include <queue>

struct DirEntry {
    std::wstring name;
    ULONGLONG size;
    DirEntry(const std::wstring& n, ULONGLONG s) : name(n), size(s) {}
};

std::mutex mtx;
std::queue<std::wstring> dir_queue;
std::vector<DirEntry> entries;

ULONGLONG GetDirectorySize(const std::wstring& path) {
    ULONGLONG total_size = 0;
    WIN32_FIND_DATAW findFileData;
    HANDLE hFind = FindFirstFileW((path + L"\\*").c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return 0;
    }

    do {
        const std::wstring fileOrDir = findFileData.cFileName;
        if (fileOrDir == L"." || fileOrDir == L"..") {
            continue;
        }

        std::wstring fullPath = path + L"\\" + fileOrDir;
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // Recursively get the size of directories
            total_size += GetDirectorySize(fullPath);
        } else {
            // Get file size
            ULARGE_INTEGER fileSize;
            fileSize.LowPart = findFileData.nFileSizeLow;
            fileSize.HighPart = findFileData.nFileSizeHigh;
            total_size += fileSize.QuadPart;
        }
    } while (FindNextFileW(hFind, &findFileData) != 0);

    FindClose(hFind);
    return total_size;
}

DWORD WINAPI Worker(LPVOID) {
    while (true) {
        std::wstring dir_path;
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (dir_queue.empty()) return 0;
            dir_path = std::move(dir_queue.front());
            dir_queue.pop();
        }

        ULONGLONG size = GetDirectorySize(dir_path);

        {
            std::lock_guard<std::mutex> lock(mtx);
            entries.emplace_back(dir_path, size);
        }
    }
    return 0;
}

std::wstring HumanReadableSize(ULONGLONG size) {
    const wchar_t* units[] = {L"B", L"KB", L"MB", L"GB", L"TB", L"PB"};
    int i = 0;
    double dsize = static_cast<double>(size);
    while (dsize >= 1024 && i < 5) {
        dsize /= 1024;
        i++;
    }
    wchar_t buf[20];
    swprintf(buf, sizeof(buf) / sizeof(buf[0]), L"%.1f %s", dsize, units[i]);
    return std::wstring(buf);
}

int wmain(int argc, wchar_t* argv[]) {
    if (argc != 2) {
        std::wcerr << L"Usage: " << argv[0] << L" <directory_path>" << std::endl;
        return 1;
    }

    std::wstring root_path = argv[1];
    WIN32_FIND_DATAW findFileData;
    HANDLE hFind = FindFirstFileW((root_path + L"\\*").c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        std::wcerr << L"Error: Invalid directory" << std::endl;
        return 1;
    }

    // Queue up directories for processing
    do {
        const std::wstring dirOrFile = findFileData.cFileName;
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (dirOrFile != L"." && dirOrFile != L"..") {
                dir_queue.push(root_path + L"\\" + dirOrFile);
            }
        }
    } while (FindNextFileW(hFind, &findFileData) != 0);

    FindClose(hFind);

    // Start worker threads
    const unsigned int num_threads = std::min(static_cast<unsigned int>(dir_queue.size()), 4u); // Limit to 4 threads for simplicity
    HANDLE threads[4];

    for (unsigned int i = 0; i < num_threads; ++i) {
        threads[i] = CreateThread(nullptr, 0, Worker, nullptr, 0, nullptr);
    }

    // Wait for all threads to complete
    WaitForMultipleObjects(num_threads, threads, TRUE, INFINITE);

    // Sort directories by size
    std::sort(entries.begin(), entries.end(), [](const DirEntry& a, const DirEntry& b) { return a.size > b.size; });

    // Print results
    ULONGLONG total_size = 0;
    for (const auto& entry : entries) {
        std::wcout << HumanReadableSize(entry.size) << L"\t\t" << entry.name << std::endl;
        total_size += entry.size;
    }

    std::wcout << L"\nTotal size: " << HumanReadableSize(total_size) << std::endl;

    return 0;
}

