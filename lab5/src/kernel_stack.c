#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int fd;
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s push <value> | pop\n", argv[0]);
        return 1;
    }

    /* Attempt to open the char device */
    fd = open("/dev/int_stack", O_RDWR);
    if (fd < 0)
    {
        if (errno == ENOENT)
        {
            fprintf(stderr, "error: USB key not inserted\n");
        }
        else
        {
            perror("open");
        }
        return 1;
    }

    if (strcmp(argv[1], "push") == 0)
    {
        if (argc < 3)
        {
            fprintf(stderr, "Usage: %s push <value>\n", argv[0]);
            close(fd);
            return 1;
        }
        int value = atoi(argv[2]);
        ssize_t ret = write(fd, &value, sizeof(value));
        if (ret < 0)
        {
            perror("write");
            close(fd);
            return 1;
        }
        if (ret != sizeof(value))
        {
            fprintf(stderr, "Error: incomplete write\n");
            close(fd);
            return 1;
        }
        printf("Pushed %d\n", value);
    }
    else if (strcmp(argv[1], "pop") == 0)
    {
        int value;
        ssize_t ret = read(fd, &value, sizeof(value));
        if (ret < 0)
        {
            perror("read");
            close(fd);
            return 1;
        }
        if (ret == 0)
        {
            printf("error: stack is empty\n");
        }
        else
        {
            printf("%d\n", value);
        }
    }
    else
    {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
}
