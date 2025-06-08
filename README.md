# DirectoryWatcher

## Overview
DirectoryWatcher is a simple C program that monitors specified directories for file system events like creation, deletion, modification, and moving of files or folders. It uses the Linux `inotify` API to track changes in real time.

## Features
- Monitors multiple directories (up to 10).
- Tracks events like file/folder creation, deletion, modification, and renaming.
- Displays events with timestamps and details about the affected file or folder.
- Easy to use from the command line.

## Requirements
- Linux system (since it uses the `inotify` API).
- GCC or another C compiler to build the program.

## Usage

Run the program by specifying one or more directories to monitor:
```bash
./directory_watcher /path/to/dir1 [/path/to/dir2 ...]
```

Example:
bash
./directory_watcher /home/user/Documents /tmp

The program will print events like file creation, deletion, or modification with timestamps.
Example Output
```bash
Started monitoring folder: /home/user/Documents
Started monitoring folder: /tmp
Monitoring started. Press Ctrl+C to exit.
=================================================
[2025-06-08 19:05:23] Created file: test.txt (in /home/user/Documents)
[2025-06-08 19:05:25] Modified file: test.txt (in /home/user/Documents)
[2025-06-08 19:05:30] Deleted folder: new_folder (in /tmp)
```

## Contributing

Feel free to submit issues or pull requests if you have ideas to improve the program!

## License

This project is licensed under the MIT License. See the LICENSE file for details.
