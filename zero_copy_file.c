#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 8192

// 零拷贝文件服务器
void zero_copy_file_server(int port, const char* file_dir) {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int opt = 1;
    
    // 创建socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // 设置socket选项
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    // 绑定端口
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    // 监听
    if (listen(server_fd, 10) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Zero-copy file server listening on port %d\n", port);
    
    while (1) {
        socklen_t addrlen = sizeof(address);
        client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (client_fd < 0) {
            perror("accept failed");
            continue;
        }
        
        printf("Client connected: %s:%d\n", 
               inet_ntoa(address.sin_addr), ntohs(address.sin_port));
        
        // 这里可以解析HTTP请求获取文件名，然后使用sendfile发送
        // 简化示例：发送固定文件
        zero_copy_send_file(client_fd, "example.txt");
        
        close(client_fd);
    }
    
    close(server_fd);
}
