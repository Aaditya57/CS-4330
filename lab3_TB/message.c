#define _XOPEN_SOURCE 700
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

pid_t other_pid = 0;

#define BOX_SIZE 4096
char *my_inbox;
char *other_inbox;

char my_inbox_shm_open_name[NAME_MAX];
char other_inbox_shm_open_name[NAME_MAX];

void cleanup_inboxes(void);

static void handle_sig(int signum) {
    if (signum == SIGUSR1) {
        fputs(my_inbox, stdout);
        fflush(stdout);
        my_inbox[0] = '\0';
    }
    else if (signum == SIGINT) {
        cleanup_inboxes();
        kill(other_pid, SIGTERM);
        exit(0);
    }
    else if (signum == SIGTERM) {
        cleanup_inboxes();
        exit(0);
    }
}

char *setup_inbox_for(pid_t pid, char *filename) {
    snprintf(filename, NAME_MAX, "/%d-chat", pid);
    
    // Unlink any existing shared memory with this name
    shm_unlink(filename);
    
    int fd = shm_open(filename, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        perror("shm_open");
        abort();
    }
    
    if (ftruncate(fd, BOX_SIZE) != 0) {
        perror("ftruncate");
        abort();
    }
    
    char *ptr = mmap(NULL, BOX_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == (char*) MAP_FAILED) {
        perror("mmap");
        abort();
    }
    
    // Initialize the memory
    memset(ptr, 0, BOX_SIZE);
    return ptr;
}

void setup_inboxes() {
    my_inbox = setup_inbox_for(getpid(), my_inbox_shm_open_name);
    other_inbox = setup_inbox_for(other_pid, other_inbox_shm_open_name);
}

void cleanup_inboxes() {
    if (my_inbox) {
        munmap(my_inbox, BOX_SIZE);
        shm_unlink(my_inbox_shm_open_name);
    }
    if (other_inbox) {
        munmap(other_inbox, BOX_SIZE);
        shm_unlink(other_inbox_shm_open_name);
    }
}

// Validate that a PID exists
int pid_exists(pid_t pid) {
    return kill(pid, 0) == 0 || errno == EPERM;
}

int main(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sig;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &sa, NULL) == -1 ||
        sigaction(SIGTERM, &sa, NULL) == -1 ||
        sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    printf("This process's ID: %ld\n", (long) getpid());
    
    char *line = NULL;
    size_t line_length = 0;
    do {
        printf("Enter other process ID: ");
        if (-1 == getline(&line, &line_length, stdin)) {
            perror("getline");
            abort();
        }
        other_pid = strtol(line, NULL, 10);
        if (other_pid <= 0) {
            printf("Invalid PID. Please enter a positive number.\n");
            continue;
        }
        if (!pid_exists(other_pid)) {
            printf("Process %d does not exist. Please enter a valid PID.\n", other_pid);
            other_pid = 0;
        }
    } while (other_pid == 0);

    free(line);
    setup_inboxes();

    // Main chat loop
    char input[BOX_SIZE];
    while (fgets(input, sizeof(input), stdin) != NULL) {
        // Copy message to other's inbox
        strncpy(other_inbox, input, sizeof(input)+1);
        other_inbox[sizeof(input)] = '\0';  // Ensure null termination
        
        if (kill(other_pid, SIGUSR1) == -1) {
            perror("Failed to send message");
            continue;
        }

        // Wait for message to be acknowledged
        while (other_inbox[0] != '\0') {
            struct timespec ts = { .tv_sec = 0, .tv_nsec = 10000000 };
            nanosleep(&ts, NULL);
        }
    }

    kill(other_pid, SIGTERM);
    cleanup_inboxes();
    return EXIT_SUCCESS;
}
