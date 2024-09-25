#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define FILE_SIZE (9L * 1024 * 1024 * 1024)  // 9GB
#define BUFFER_SIZE (1024 * 1024 * 1)  // 1MB

int main() {
    // 打开或创建文件，准备写入测试数据
    int fd = open("/mnt/hdd/test_output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    // 分配一个 1MB 的缓冲区，填充一些数据
    char *buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        perror("Error allocating buffer");
        close(fd);
        return 1;
    }
    memset(buffer, 'A', BUFFER_SIZE);  // 填充 'A' 字符

    // 记录写入开始时间
    time_t start_time = time(NULL);
    printf("start time: %s\n", ctime(&start_time));

    // 写入 2GB 的数据
    ssize_t total_written = 0;
    while (total_written < FILE_SIZE) {
        ssize_t written = write(fd, buffer, BUFFER_SIZE);
        if (written == -1) {
            perror("Error writing to file");
            break;
        }
        total_written += written;
    }
    
    // sync();

    // 记录写入结束时间
    time_t end_time = time(NULL);
    printf("end time: %s\n", ctime(&end_time));

    sleep(30);
    // printf("Wrote %ld bytes to test_output.txt in %.2f seconds\n", total_written, time_spent);

    free(buffer);
    close(fd);

    return 0;
}
