#include <algorithm>
#include <atomic>
#include <cstring>
#include <dirent.h>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <vector>
#include <queue>

namespace fs = std::filesystem;

struct DirEntry {
    std::string name;
    off_t size;
    DirEntry(std::string n, off_t s) : name(std::move(n)), size(s) {}
};

std::mutex mtx;
std::queue<fs::path> dir_queue;
std::vector<DirEntry> entries;

off_t get_directory_size(const fs::path& path) {
    off_t total_size = 0;
    for (const auto& entry : fs::recursive_directory_iterator(path, fs::directory_options::skip_permission_denied)) {
        try {
            if (fs::is_regular_file(entry)) {
                total_size += fs::file_size(entry);
            }
        } catch (const fs::filesystem_error& e) {
            // Silently skip files that can't be accessed
        }
    }
    return total_size;
}

void worker() {
    while (true) {
        fs::path dir_path;
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (dir_queue.empty()) return;
            dir_path = std::move(dir_queue.front());
            dir_queue.pop();
        }
        
        off_t size = get_directory_size(dir_path);
        
        {
            std::lock_guard<std::mutex> lock(mtx);
            entries.emplace_back(dir_path.filename().string(), size);
        }
    }
}

std::string human_readable_size(off_t size) {
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
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    fs::path root_path = argv[1];
    if (!fs::is_directory(root_path)) {
        std::cerr << "Error: " << root_path << " is not a directory." << std::endl;
        return 1;
    }

    for (const auto& entry : fs::directory_iterator(root_path)) {
        if (fs::is_directory(entry)) {
            dir_queue.push(entry.path());
        }
    }

    unsigned int num_threads = std::min(std::thread::hardware_concurrency(), static_cast<unsigned int>(dir_queue.size()));
    std::vector<std::thread> threads;
    for (unsigned int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::sort(entries.begin(), entries.end(), 
              [](const DirEntry& a, const DirEntry& b) { return a.size > b.size; });

    off_t total_size = 0;
    for (const auto& entry : entries) {
        std::cout << human_readable_size(entry.size) << "\t" << entry.name << std::endl;
        total_size += entry.size;
    }

    std::cout << "\nTotal size: " << human_readable_size(total_size) << std::endl;

    return 0;
}
