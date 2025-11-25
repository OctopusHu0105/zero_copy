#include <sys/sendfile.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

// 使用sendfile零拷贝发送文件
int zero_copy_send_file(int sockfd, const char* filename) {
    int filefd = open(filename, O_RDONLY);
    if (filefd == -1) {
        perror("open file failed");
        return -1;
    }

    // 获取文件大小
    struct stat file_stat;
    if (fstat(filefd, &file_stat) == -1) {
        perror("fstat failed");
        close(filefd);
        return -1;
    }

    off_t offset = 0;
    ssize_t sent_bytes = 0;
    size_t remaining = file_stat.st_size;

    // 使用sendfile零拷贝传输
    while (remaining > 0) {
        sent_bytes = sendfile(sockfd, filefd, &offset, remaining);
        if (sent_bytes == -1) {
            perror("sendfile failed");
            close(filefd);
            return -1;
        }
        remaining -= sent_bytes;
    }

    close(filefd);
    printf("Sent file %s (%ld bytes) using zero-copy\n", 
           filename, file_stat.st_size);
    return 0;
}
