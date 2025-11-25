#include <stdatomic.h>
#include <stdlib.h>

typedef struct {
    void* data;
    size_t size;
    _Atomic int ref_count;
    int owned;  // 是否拥有内存
} buffer_block_t;

typedef struct {
    buffer_block_t* blocks;
    size_t block_count;
    size_t capacity;
} zero_copy_buffer_t;

int buffer_init(zero_copy_buffer_t* buf, size_t initial_capacity) {
    buf->blocks = malloc(initial_capacity * sizeof(buffer_block_t));
    if (!buf->blocks) return -1;
    
    buf->capacity = initial_capacity;
    buf->block_count = 0;
    return 0;
}

// 添加外部数据
int buffer_add_external(zero_copy_buffer_t* buf, void* data, size_t size) {
    if (buf->block_count >= buf->capacity) {
        size_t new_cap = buf->capacity * 2;
        buffer_block_t* new_blocks = realloc(buf->blocks, new_cap * sizeof(buffer_block_t));
        if (!new_blocks) return -1;
        buf->blocks = new_blocks;
        buf->capacity = new_cap;
    }
    
    buf->blocks[buf->block_count] = (buffer_block_t){
        .data = data,
        .size = size,
        .ref_count = 1,
        .owned = 0
    };
    buf->block_count++;
    return 0;
}

// 增加引用计数
void buffer_ref(buffer_block_t* block) {
    atomic_fetch_add(&block->ref_count, 1);
}

// 减少引用计数
void buffer_unref(buffer_block_t* block) {
    if (atomic_fetch_sub(&block->ref_count, 1) == 1) {
        if (block->owned) {
            free(block->data);
        }
    }
}
