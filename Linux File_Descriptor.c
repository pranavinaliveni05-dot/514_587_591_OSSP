#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

pthread_mutex_t mutex;

void *thread_function(void *arg)
{
    int *value = (int *)arg;

    pthread_mutex_lock(&mutex);

    printf("\n========================================\n");
    printf("             THREAD EXECUTION\n");
    printf("========================================\n");
    printf("Thread ID: %lu\n", (unsigned long)pthread_self());
    printf("Thread is executing...\n");

    (*value)++;

    printf("Shared Value: %d\n", *value);
    printf("Thread completed successfully.\n");
    printf("========================================\n");

    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main()
{
    int fd = -1;
    char filename[256];
    char command[50];
    char buffer[BUFFER_SIZE];

    printf("====================================================\n");
    printf("       LINUX FILE DESCRIPTOR AND FILE I/O SYSTEM\n");
    printf("====================================================\n");

    while (1)
    {
        printf("\nEnter command: ");
        scanf("%49s", command);

        if (strcmp(command, "create") == 0)
        {
            int create_fd;

            printf("Enter file name: ");
            scanf("%255s", filename);

            create_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

            if (create_fd == -1)
            {
                perror("File creation failed");
                continue;
            }

            printf("\nFile created successfully!\n");
            printf("File Name: %s\n", filename);

            close(create_fd);
        }

        else if (strcmp(command, "open") == 0)
        {
            if (fd != -1)
            {
                printf("A file is already open.\n");
                printf("Current File Descriptor: %d\n", fd);
                continue;
            }

            printf("Enter file name: ");
            scanf("%255s", filename);

            fd = open(filename, O_RDWR);

            if (fd == -1)
            {
                perror("Open failed");
                continue;
            }

            printf("\nFile opened successfully!\n");
            printf("File Name      : %s\n", filename);
            printf("File Descriptor: %d\n", fd);
        }

        else if (strcmp(command, "write") == 0)
        {
            if (fd == -1)
            {
                printf("No file is open. Please use 'open' first.\n");
                continue;
            }

            printf("Enter data to write: ");
            getchar();

            if (fgets(buffer, BUFFER_SIZE, stdin) == NULL)
            {
                printf("Input error.\n");
                continue;
            }

            ssize_t bytes_written = write(fd, buffer, strlen(buffer));

            if (bytes_written == -1)
            {
                perror("Write failed");
                continue;
            }

            printf("\nData written successfully!\n");
            printf("Bytes written: %ld\n", bytes_written);
        }

        else if (strcmp(command, "read") == 0)
        {
            if (fd == -1)
            {
                printf("No file is open. Please use 'open' first.\n");
                continue;
            }

            if (lseek(fd, 0, SEEK_SET) == -1)
            {
                perror("File position error");
                continue;
            }

            ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE - 1);

            if (bytes_read == -1)
            {
                perror("Read failed");
                continue;
            }

            buffer[bytes_read] = '\0';

            printf("\nRead successfully!\n");
            printf("File Name      : %s\n", filename);
            printf("File Descriptor: %d\n", fd);
            printf("Bytes read     : %ld\n", bytes_read);

            printf("\n----- File Content -----\n");
            printf("%s", buffer);
            printf("------------------------\n");
        }

        else if (strcmp(command, "process") == 0)
        {
            if (fd == -1)
            {
                printf("No file is open. Please use 'open' first.\n");
                continue;
            }

            int pipefd[2];

            if (pipe(pipefd) == -1)
            {
                perror("Pipe creation failed");
                continue;
            }

            pid_t pid = fork();

            if (pid == -1)
            {
                perror("Process creation failed");
                close(pipefd[0]);
                close(pipefd[1]);
                continue;
            }

            if (pid == 0)
            {
                close(pipefd[1]);

                char message[BUFFER_SIZE];

                ssize_t bytes_read =
                    read(pipefd[0], message, BUFFER_SIZE - 1);

                if (bytes_read == -1)
                {
                    perror("Pipe read failed");
                    close(pipefd[0]);
                    exit(1);
                }

                message[bytes_read] = '\0';

                printf("\n========================================\n");
                printf("             CHILD PROCESS\n");
                printf("========================================\n");
                printf("Child Process ID : %d\n", getpid());
                printf("Parent Process ID: %d\n", getppid());
                printf("\nMessage received:\n");
                printf("%s", message);
                printf("========================================\n");

                close(pipefd[0]);
                exit(0);
            }
            else
            {
                close(pipefd[0]);

                char message[BUFFER_SIZE];

                snprintf(
                    message,
                    BUFFER_SIZE,
                    "File Name: %s\nFile Descriptor: %d\n",
                    filename,
                    fd
                );

                write(pipefd[1], message, strlen(message));

                printf("\n========================================\n");
                printf("             PARENT PROCESS\n");
                printf("========================================\n");
                printf("Parent Process ID: %d\n", getpid());
                printf("Child Process ID : %d\n", pid);
                printf("Information sent successfully.\n");
                printf("========================================\n");

                close(pipefd[1]);

                printf("\nWaiting for child process to complete...\n");

                int status;
                pid_t finished_pid = wait(&status);

                if (finished_pid == pid && WIFEXITED(status))
                {
                    printf("\nChild process completed successfully.\n");
                    printf("Child Process ID: %d\n", finished_pid);
                    printf("Child Exit Status: %d\n",
                           WEXITSTATUS(status));
                }
            }
        }

        else if (strcmp(command, "memory") == 0)
        {
            int value = 100;

            printf("\n========================================\n");
            printf("             PROCESS MEMORY\n");
            printf("========================================\n");
            printf("Initial Value: %d\n", value);

            pid_t pid = fork();

            if (pid == -1)
            {
                perror("Process creation failed");
                continue;
            }

            if (pid == 0)
            {
                value = 200;

                printf("\nChild Process ID : %d\n", getpid());
                printf("Child Value: %d\n", value);

                exit(0);
            }
            else
            {
                wait(NULL);

                printf("\nParent Process ID: %d\n", getpid());
                printf("Parent Value: %d\n", value);
                printf("\nProcess memory operation completed.\n");
                printf("========================================\n");
            }
        }

        else if (strcmp(command, "shared") == 0)
        {
            int *shared_value;

            shared_value = mmap(
                NULL,
                sizeof(int),
                PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_ANONYMOUS,
                -1,
                0
            );

            if (shared_value == MAP_FAILED)
            {
                perror("Shared memory creation failed");
                continue;
            }

            *shared_value = 0;

            int count;

            printf("Enter number of operations: ");
            scanf("%d", &count);

            pid_t pid = fork();

            if (pid == -1)
            {
                perror("Process creation failed");
                munmap(shared_value, sizeof(int));
                continue;
            }

            if (pid == 0)
            {
                for (int i = 0; i < count; i++)
                {
                    int temp = *shared_value;
                    temp++;
                    *shared_value = temp;
                }

                printf("\nChild process completed.\n");
                exit(0);
            }
            else
            {
                for (int i = 0; i < count; i++)
                {
                    int temp = *shared_value;
                    temp++;
                    *shared_value = temp;
                }

                wait(NULL);

                printf("\n========================================\n");
                printf("              SHARED DATA\n");
                printf("========================================\n");
                printf("Expected Value: %d\n", count * 2);
                printf("Final Value   : %d\n", *shared_value);
                printf("Data operations completed.\n");
                printf("========================================\n");

                munmap(shared_value, sizeof(int));
            }
        }

        else if (strcmp(command, "execute") == 0)
        {
            pthread_t thread1;
            pthread_t thread2;

            int shared_value = 0;

            if (pthread_mutex_init(&mutex, NULL) != 0)
            {
                printf("Mutex initialization failed.\n");
                continue;
            }

            printf("\n========================================\n");
            printf("          THREAD MANAGEMENT\n");
            printf("========================================\n");
            printf("Main Process ID: %d\n", getpid());

            if (pthread_create(
                    &thread1,
                    NULL,
                    thread_function,
                    &shared_value) != 0)
            {
                printf("Thread creation failed.\n");
                pthread_mutex_destroy(&mutex);
                continue;
            }

            if (pthread_create(
                    &thread2,
                    NULL,
                    thread_function,
                    &shared_value) != 0)
            {
                printf("Thread creation failed.\n");
                pthread_join(thread1, NULL);
                pthread_mutex_destroy(&mutex);
                continue;
            }

            pthread_join(thread1, NULL);
            pthread_join(thread2, NULL);

            printf("\n========================================\n");
            printf("       THREAD SYNCHRONIZATION\n");
            printf("========================================\n");
            printf("Thread 1 joined successfully.\n");
            printf("Thread 2 joined successfully.\n");
            printf("Final Shared Value: %d\n", shared_value);
            printf("Execution completed successfully.\n");
            printf("========================================\n");

            pthread_mutex_destroy(&mutex);
        }

        else if (strcmp(command, "close") == 0)
        {
            if (fd == -1)
            {
                printf("No file is currently open.\n");
                continue;
            }

            if (close(fd) == -1)
            {
                perror("Close failed");
                continue;
            }

            printf("\nFile closed successfully!\n");
            printf("File Name      : %s\n", filename);
            printf("Released FD    : %d\n", fd);

            fd = -1;
        }

        else if (strcmp(command, "exit") == 0)
        {
            if (fd != -1)
            {
                close(fd);
                printf("\nOpen file closed automatically.\n");
            }

            printf("Program terminated successfully.\n");
            break;
        }

        else
        {
            printf("\nInvalid command.\n");
            printf("Use: create, open, write, read, process, memory, shared, execute, close, exit\n");
        }
    }

    return 0;
}
