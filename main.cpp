#include <algorithm>
#include <atomic>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace fs = std::filesystem;

struct DirEntry {
    std::string name;
    uintmax_t size;
    DirEntry(std::string n, uintmax_t s) : name(std::move(n)), size(s) {}
};

std::mutex mtx;
std::vector<DirEntry> entries;
std::atomic<uintmax_t> total_size{0};

uintmax_t get_directory_size(const fs::path& path) {
    uintmax_t size = 0;
    try {
        for (const auto& entry : fs::recursive_directory_iterator(
                 path, fs::directory_options::skip_permission_denied)) {
            if (fs::is_regular_file(entry.status())) {
                size += entry.file_size();
            }
        }
    } catch (const fs::filesystem_error&) {
        // Silently skip directories that can't be accessed
    }
    return size;
}

void process_entry(const fs::directory_entry& entry) {
    uintmax_t size;
    if (fs::is_directory(entry)) {
        size = get_directory_size(entry);
    } else if (fs::is_regular_file(entry)) {
        size = entry.file_size();
    } else {
        return; // Skip special files
    }

    total_size += size;
    {
        std::lock_guard<std::mutex> lock(mtx);
        entries.emplace_back(entry.path().filename().string(), size);
    }
}

std::string human_readable_size(uintmax_t size) {
    const char* units[] = {"B", "K", "M", "G", "T", "P", "E"};
    int i = 0;
    double dsize = size;
    while (dsize >= 1024 && i < 6) {
        dsize /= 1024;
        i++;
    }
    char buf[20];
    snprintf(buf, sizeof(buf), "%.1f%s", dsize, units[i]);
    return buf;
}

int main(int argc, char* argv[]) {
    const char* path = (argc > 1) ? argv[1] : ".";
    
    std::vector<std::thread> threads;
    for (const auto& entry : fs::directory_iterator(path)) {
        threads.emplace_back(process_entry, entry);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::sort(entries.begin(), entries.end(),
              [](const DirEntry& a, const DirEntry& b) { return a.size > b.size; });

    for (const auto& entry : entries) {
        std::cout << human_readable_size(entry.size) << "\t\t" << entry.name << std::endl;
    }

    std::cout << "\nTotal size: " << human_readable_size(total_size) << std::endl;

    return 0;
}
