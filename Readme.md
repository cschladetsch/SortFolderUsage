# Folder Size Summary Tool

## Overview

The Folder Size Summary Tool is a high-performance C++ program designed to quickly calculate and display the sizes of subdirectories within a specified directory. It's particularly optimized for use in Windows Subsystem for Linux (WSL) environments, but can run on any POSIX-compliant system.

## Features

- Fast, multi-threaded directory size calculation
- Human-readable size output (B, KB, MB, GB, etc.)
- Sorts subdirectories by size in descending order
- Handles permission errors gracefully
- Provides a total size summary

## Requirements

- C++17 compatible compiler (e.g., GCC 7+ or Clang 5+)
- POSIX-compliant operating system (Linux, macOS, WSL, etc.)
- pthread library

## Compilation

To compile the program, use the following command:

```bash
g++ -O3 -march=native -mtune=native -pthread -std=c++17 folder_size_summary.cpp -o folder_size_summary
```

This command enables optimizations and multi-threading support.

## Usage

Run the compiled program with a directory path as an argument:

```bash
./folder_size_summary /path/to/directory
```

Replace `/path/to/directory` with the actual path you want to analyze.

## Output

The program will output a list of subdirectories sorted by size in descending order, followed by a total size summary. For example:

```
1.5G    Downloads
985.3M  Documents
256.0M  Pictures
54.2M   Music
10.5M   Videos

Total size: 2.8G
```

## Performance Considerations

- The tool uses multi-threading to process directories in parallel, which can significantly speed up the analysis of directories with many subdirectories.
- It's optimized for WSL environments, taking into account potential I/O limitations.
- For very large directories or filesystems with millions of files, the tool may take longer to complete.

## Error Handling

- The program will skip files and directories it doesn't have permission to access.
- If the specified path is not a directory, the program will display an error message and exit.

## Limitations

- Symbolic links are not followed to prevent potential infinite loops.
- The tool currently only summarizes immediate subdirectories of the specified path, not the entire directory tree.

## Contributing

Contributions to improve the tool are welcome. Please submit pull requests or open issues on the project's GitHub repository.

## License

This tool is released under the MIT License. See the LICENSE file for details.
