// 共享环形缓冲区零拷贝

#include <stdatomic.h>
#include <string.h>

typedef struct {
    void* buffer;
    size_t capacity;
    _Atomic size_t head;
    _Atomic size_t tail;
    int fd; // 关联的文件描述符（可选）
} ring_buffer_t;

// 初始化环形缓冲区
int ring_buffer_init(ring_buffer_t* rb, size_t capacity) {
    rb->buffer = malloc(capacity);
    if (!rb->buffer) return -1;
    
    rb->capacity = capacity;
    atomic_store(&rb->head, 0);
    atomic_store(&rb->tail, 0);
    rb->fd = -1;
    return 0;
}

// 零拷贝方式获取写入位置
void* ring_buffer_prepare_write(ring_buffer_t* rb, size_t* available) {
    size_t head = atomic_load(&rb->head);
    size_t tail = atomic_load(&rb->tail);
    
    size_t free_space;
    if (tail >= head) {
        free_space = rb->capacity - tail + head;
    } else {
        free_space = head - tail;
    }
    
    if (available) *available = free_space;
    
    if (free_space == 0) return NULL;
    
    return (char*)rb->buffer + tail;
}

// 提交写入数据
void ring_buffer_commit_write(ring_buffer_t* rb, size_t size) {
    size_t tail = atomic_load(&rb->tail);
    tail = (tail + size) % rb->capacity;
    atomic_store(&rb->tail, tail);
}

// 零拷贝方式获取读取位置
const void* ring_buffer_prepare_read(ring_buffer_t* rb, size_t* available) {
    size_t head = atomic_load(&rb->head);
    size_t tail = atomic_load(&rb->tail);
    
    size_t data_available;
    if (tail >= head) {
        data_available = tail - head;
    } else {
        data_available = rb->capacity - head + tail;
    }
    
    if (available) *available = data_available;
    
    if (data_available == 0) return NULL;
    
    return (const char*)rb->buffer + head;
}

// 提交读取数据
void ring_buffer_commit_read(ring_buffer_t* rb, size_t size) {
    size_t head = atomic_load(&rb->head);
    head = (head + size) % rb->capacity;
    atomic_store(&rb->head, head);
}
