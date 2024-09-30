#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <dlfcn.h>  // For RTLD_NEXT and dlsym()
#include <stdarg.h>

#ifndef RTLD_NEXT
#define RTLD_NEXT ((void *) -1l)
#endif

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// 定义原始 write 函数指针
ssize_t (*orig_write)(int fd, const void *buf, size_t count) = NULL;
// static int (*real_close)(int fd) = NULL;
static int (*orig_open)(const char *pathname, int flags, ...);

int mem_fd = -1;
int orig_fd = -1;
extern int threads_created = 0;
extern int threads_destroied = 0;

int open(const char *pathname, int flags, ...) {
    // 打印劫持信息
    printf("Intercepted open: %s\n", pathname);

    // 获取原始的 open 函数地址
    if (!orig_open) {
        orig_open = dlsym(RTLD_NEXT, "open");
    }
    
    char* mem_filepath = malloc(256);
    snprintf(mem_filepath, 256, "/dev/shm/%s", strrchr(pathname, '/') + 1);

    mem_fd = orig_open(mem_filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    orig_fd = orig_open(pathname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    free(mem_filepath);

    return mem_fd;
}

// 获取原始文件路径的函数
char* get_original_filepath(int fd) {
    static char filepath[256];
    snprintf(filepath, sizeof(filepath), "/proc/self/fd/%d", fd);
    ssize_t len = readlink(filepath, filepath, sizeof(filepath) - 1);
    if (len != -1) {
        filepath[len] = '\0';
        return filepath;
    }
    return NULL;
}

// 定义用于异步写入的结构体
struct write_data {
    char* buffer;
    size_t count;
};

// 异步写回原始文件的函数
void* async_write_to_original(void* arg) {
    struct write_data* data = (struct write_data*) arg;

    // 打开原始文件进行写入
    if (orig_fd == -1) {
        perror("Error opening original file for async write");
        free(data->buffer);
        free(data);
        return NULL;
    }

    sleep(3);

    // 写入原始文件
    ssize_t written = orig_write(orig_fd, data->buffer, data->count);
    if (written == -1) {
        perror("Error writing to original file asynchronously");
    } else {
        // printf("Asynchronously wrote %ld bytes to %s\n", written, data->orig_filepath);
    }

    free(data->buffer);
    free(data);

    pthread_mutex_lock(&lock);
    threads_destroied++;
    pthread_mutex_unlock(&lock);
    
    return NULL;
}

// 劫持 close 函数
//int close(int fd) {
//    // 加载真正的 close 函数
//    if (!real_close) {
//        real_close = dlsym(RTLD_NEXT, "close");
//        if (!real_close) {
//            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
//            exit(EXIT_FAILURE);
//        }
//    }
//
//    // real_close(orig_fd); 不要关闭原始的文件，子线程可能还没完成写入
//
//    // 调用真正的 close 函数关闭原始文件描述符
//    return real_close(fd);
//}

// 劫持 write 函数
ssize_t write(int fd, const void *buf, size_t count) {
    // 如果原始 write 函数指针为空，加载原始 write
    if (!orig_write) {
        orig_write = dlsym(RTLD_NEXT, "write");
    }

    if (fd == 1 || fd == 2) {
#ifdef debug
       printf("direct file is stdout/err, ignore. \n");
#endif
       return orig_write(fd, buf, count);
    }

    if (mem_fd == -1){
	// 初始化内存文件
        perror("Error opening memory file");
        return -1;
    }

    // 为异步写回准备数据
    pthread_t tid;
    struct write_data* data = malloc(sizeof(struct write_data));
    
    // 复制 buf 数据到 data->buffer 中
    data->buffer = malloc(count);
    memcpy(data->buffer, buf, count);
    data->count = count;

    // 创建异步写回线程
    int ret = pthread_create(&tid, NULL, async_write_to_original, data);
    if (ret != 0) {
        printf("Error creating thread: %d\n", ret);
        free(data->buffer);
        free(data);
    } else {
        threads_created++;
#ifdef debug
	    printf("created threads: %d\n", threads_all);
#endif        
        pthread_detach(tid);  // 分离线程
    }

    // 返回写入内存文件的字节数
    return orig_write(fd, buf, count);
}
