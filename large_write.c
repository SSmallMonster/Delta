#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define FILE_SIZE_BIG 1024 // MB
#define FILE_SIZE (FILE_SIZE_BIG * 1024 * 1024)  // 1024 MB
#define BUFFER_SIZE (1024 * 4)  // 4KB

#ifndef accl
static int threads_all = 0;
#else
extern int threads_all;
#endif

char* random_str() {
    char *str = malloc(BUFFER_SIZE);
    for (int i = 0; i < 7; i++) {
        str[i] = 'a' + rand() % 26; // 生成随机小写字母
    }
    str[7] = '\n'; // 字符串结束符
    return str;
}

int main() {
    // 打开或创建文件，准备写入测试数据
    int fd = open("/mnt/hdd/test_output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    // 分配一个 1MB 的缓冲区，填充一些数据
    srand((unsigned)time(NULL)); // 设置随机数种子
    char *buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        perror("Error allocating buffer");
        close(fd);
        return 1;
    }
    // memset(buffer, 'A', BUFFER_SIZE);  // 填充 'A' 字符

    // 记录写入开始时间
    time_t start_time = time(NULL);
    printf("start time: %s\n", ctime(&start_time));

    // 写入 2GB 的数据
    ssize_t total_written = 0;
    while (total_written < FILE_SIZE) {
        sprintf(buffer,"%s",random_str());
        ssize_t written = write(fd, buffer, BUFFER_SIZE);
        if (written == -1) {
            perror("Error writing to file");
            break;
        }
        total_written += written;
    }
    
    free(buffer);
    close(fd);

    // 记录写入结束时间
    time_t end_time = time(NULL);
    printf("total writing %d(MB)\n", FILE_SIZE_BIG);
    printf("memory write end time: %s\n", ctime(&end_time));

    // 等待子线程全部结束 - 动态库异步写入原始存储
    while(1){
    	if (threads_all == 0){
	    break;
    	}
	sleep(1);
    }
    
    end_time = time(NULL);
    printf("origin storage write end time: %s\n", ctime(&end_time));


    return 0;
}
