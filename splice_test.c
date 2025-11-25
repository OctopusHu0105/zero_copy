#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

// 使用splice在文件描述符间零拷贝传输
int zero_copy_splice(int from_fd, int to_fd, size_t len) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe creation failed");
        return -1;
    }

    size_t total_transferred = 0;
    
    while (total_transferred < len) {
        // 从源文件描述符读取到管道
        ssize_t spliced = splice(from_fd, NULL, pipefd[1], NULL,
                                len - total_transferred, SPLICE_F_MOVE);
        if (spliced == -1) {
            if (errno == EAGAIN) continue;
            perror("splice read failed");
            close(pipefd[0]);
            close(pipefd[1]);
            return -1;
        }
        
        if (spliced == 0) break; // EOF

        // 从管道写入到目标文件描述符
        ssize_t written = splice(pipefd[0], NULL, to_fd, NULL,
                                spliced, SPLICE_F_MOVE);
        if (written == -1) {
            perror("splice write failed");
            close(pipefd[0]);
            close(pipefd[1]);
            return -1;
        }

        total_transferred += written;
    }

    close(pipefd[0]);
    close(pipefd[1]);
    return total_transferred;
}
