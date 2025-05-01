#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#define DEVICE_PATH "/dev/int_stack"
#define INT_STACK_SET_MAX_SIZE _IOW('s', 1, int)

static int fd = -1;

void cleanup()
{
    if (fd != -1)
        close(fd);
}

void print_usage()
{
    fprintf(stderr, "Usage: kernel_stack [command] [args]\n");
    fprintf(stderr, "Commands:\n");
    fprintf(stderr, "  set-size <size>  Set the maximum stack size\n");
    fprintf(stderr, "  push <value>     Push an integer onto the stack\n");
    fprintf(stderr, "  pop              Pop and print the top element\n");
    fprintf(stderr, "  unwind           Pop and print all elements until empty\n");
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        print_usage();
        exit(EXIT_FAILURE);
    }

    atexit(cleanup);
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    char *command = argv[1];
    if (fd < 0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    if (strcmp(command, "set-size") == 0)
    {
        if (argc != 3)
        {
            fprintf(stderr, "ERROR: set-size requires one argument\n");
            close(fd);
            exit(EXIT_FAILURE);
        }

        char *endptr;
        long size = strtol(argv[2], &endptr, 10);
        if (*endptr != '\0' || size <= 0)
        {
            fprintf(stderr, "ERROR: size should be > 0\n");
            close(fd);
            exit(-EINVAL);
        }

        if (ioctl(fd, INT_STACK_SET_MAX_SIZE, &size) == -1)
        {
            switch (errno)
            {
            case EBUSY:
                fprintf(stderr, "ERROR: stack size already configured\n");
                break;
            case EINVAL:
                fprintf(stderr, "ERROR: invalid size\n");
                break;
            default:
                perror("ioctl");
            }
            close(fd);
            exit(-errno);
        }
    }
    else if (strcmp(command, "push") == 0)
    {
        if (argc != 3)
        {
            fprintf(stderr, "ERROR: push requires one argument\n");
            close(fd);
            exit(EXIT_FAILURE);
        }

        char *endptr;
        long value = strtol(argv[2], &endptr, 10);
        if (*endptr != '\0')
        {
            fprintf(stderr, "ERROR: invalid value\n");
            close(fd);
            exit(-EINVAL);
        }

        int val = (int)value;
        if (write(fd, &val, sizeof(val)) != sizeof(val))
        {
            if (errno == ERANGE)
            {
                fprintf(stderr, "ERROR: stack is full\n");
            }
            else
            {
                perror("write");
            }
            close(fd);
            exit(-errno);
        }
    }
    else if (strcmp(command, "pop") == 0)
    {
        if (argc != 2)
        {
            fprintf(stderr, "ERROR: pop requires no arguments\n");
            close(fd);
            exit(EXIT_FAILURE);
        }

        int value;
        ssize_t bytes_read = read(fd, &value, sizeof(value));
        if (bytes_read == 0)
        {
            printf("Stack is empty\n");
        }
        else if (bytes_read == sizeof(value))
        {
            printf("%d\n", value);
        }
        else
        {
            perror("read");
            close(fd);
            exit(-errno);
        }
    }
    else if (strcmp(command, "unwind") == 0)
    {
        if (argc != 2)
        {
            fprintf(stderr, "ERROR: unwind requires no arguments\n");
            close(fd);
            exit(EXIT_FAILURE);
        }

        int value;
        while (1)
        {
            ssize_t bytes_read = read(fd, &value, sizeof(value));
            if (bytes_read == 0)
            {
                printf("Stack is empty\n");
                break;
            }

            if (bytes_read == sizeof(value))
            {
                printf("%d\n", value);
            }
            else
            {
                perror("read");
                close(fd);
                exit(-errno);
            }
        }
    }
    else
    {
        fprintf(stderr, "ERROR: unknown command '%s'\n", command);
        print_usage();
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
    return 0;
}