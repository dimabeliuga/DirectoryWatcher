#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#define BUF_LEN (1024 * (sizeof(struct inotify_event) + 16))
#define MAX_PATHS 10

// Structure to store information about watched directories
typedef struct {
    int wd;             
    char path[256];     
} watch_info_t;

// Function to get the current time as a readable string
void get_current_time(char *time_str, size_t size) {
    time_t now;
    struct tm *tm_info;

    time(&now);
    tm_info = localtime(&now);
    strftime(time_str, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

// Function to check if the object is a directory
int is_directory(const char *base_path, const char *name) {
    char full_path[512];
    struct stat path_stat;

    snprintf(full_path, sizeof(full_path), "%s/%s", base_path, name);

    if (stat(full_path, &path_stat) == 0) {
        return S_ISDIR(path_stat.st_mode);
    }

    return 0;
}

// Function to find the path associated with a watch descriptor
const char* find_path_by_wd(watch_info_t *watches, int count, int wd) {
    for (int i = 0; i < count; i++) {
        if (watches[i].wd == wd) {
            return watches[i].path;
        }
    }
    return "unknown directory";
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <dir1> [dir2] [dir3] ...\n", argv[0]);
        printf("Example: %s /home/user/Documents /tmp /home/user/Downloads\n", argv[0]);
        return 1;
    }

    if (argc - 1 > MAX_PATHS) {
        printf("Error: Maximum number of directories to watch is %d\n", MAX_PATHS);
        return 1;
    }

    int fd = inotify_init();
    if (fd < 0) {
        perror("inotify_init");
        return 1;
    }

    watch_info_t watches[MAX_PATHS];
    int watch_count = 0;

    for (int i = 1; i < argc; i++) {
        int wd = inotify_add_watch(fd, argv[i],
                                   IN_CREATE | IN_DELETE | IN_MODIFY |
                                   IN_MOVED_FROM | IN_MOVED_TO);

        if (wd == -1) {
            printf("Warning: Failed to watch '%s': %s\n",
                   argv[i], strerror(errno));
            continue;
        }

        watches[watch_count].wd = wd;
        strncpy(watches[watch_count].path, argv[i], sizeof(watches[watch_count].path) - 1);
        watches[watch_count].path[sizeof(watches[watch_count].path) - 1] = '\0';
        watch_count++;

        printf("Started watching: %s\n", argv[i]);
    }

    if (watch_count == 0) {
        printf("Error: No directories were added to the watch list\n");
        close(fd);
        return 1;
    }

    printf("Monitoring started. Press Ctrl+C to exit.\n");
    printf("=================================================\n");

    char buf[BUF_LEN];
    char time_str[64];

    while (1) {
        int len = read(fd, buf, BUF_LEN);
        if (len < 0) {
            if (errno == EINTR) {
                continue; 
            }
            perror("read");
            break;
        }

        int i = 0;
        while (i < len) {
            struct inotify_event *event = (struct inotify_event *) &buf[i];

            if (event->len == 0) {
                i += sizeof(struct inotify_event) + event->len;
                continue;
            }

            get_current_time(time_str, sizeof(time_str));
            const char *watch_path = find_path_by_wd(watches, watch_count, event->wd);

            int is_dir = 0;
            if (event->mask & (IN_CREATE | IN_MOVED_TO)) {
                is_dir = is_directory(watch_path, event->name);
            } else if (event->mask & IN_ISDIR) {
                is_dir = 1;
            }

            const char *obj_type = is_dir ? "directory" : "file";

            if (event->mask & IN_CREATE) {
                printf("[%s] Created %s: %s (in %s)\n",
                       time_str, obj_type, event->name, watch_path);
            }

            if (event->mask & IN_DELETE) {
                printf("[%s] Deleted %s: %s (from %s)\n",
                       time_str, obj_type, event->name, watch_path);
            }

            if (event->mask & IN_MODIFY) {
                if (!is_dir) { 
                    printf("[%s] Modified file: %s (in %s)\n",
                           time_str, event->name, watch_path);
                }
            }

            if (event->mask & IN_MOVED_FROM) {
                printf("[%s] Moved/Renamed %s: %s (from %s)\n",
                       time_str, obj_type, event->name, watch_path);
            }

            if (event->mask & IN_MOVED_TO) {
                printf("[%s] Moved/Renamed %s: %s (to %s)\n",
                       time_str, obj_type, event->name, watch_path);
            }

            i += sizeof(struct inotify_event) + event->len;
        }
    }

    for (int i = 0; i < watch_count; i++) {
        inotify_rm_watch(fd, watches[i].wd);
    }
    close(fd);

    return 0;
}
