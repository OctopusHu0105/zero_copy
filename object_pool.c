#include <pthread.h>

typedef struct object_pool_node {
    void* object;
    struct object_pool_node* next;
} object_pool_node_t;

typedef struct {
    object_pool_node_t* head;
    pthread_mutex_t mutex;
    size_t object_size;
    size_t max_size;
    size_t current_size;
} object_pool_t;

int object_pool_init(object_pool_t* pool, size_t object_size, size_t max_size) {
    pool->head = NULL;
    pool->object_size = object_size;
    pool->max_size = max_size;
    pool->current_size = 0;
    return pthread_mutex_init(&pool->mutex, NULL);
}

void* object_pool_acquire(object_pool_t* pool) {
    pthread_mutex_lock(&pool->mutex);
    
    if (pool->head) {
        object_pool_node_t* node = pool->head;
        pool->head = node->next;
        void* obj = node->object;
        free(node);
        pool->current_size--;
        pthread_mutex_unlock(&pool->mutex);
        return obj;
    }
    
    pthread_mutex_unlock(&pool->mutex);
    return malloc(pool->object_size);
}

void object_pool_release(object_pool_t* pool, void* object) {
    pthread_mutex_lock(&pool->mutex);
    
    if (pool->current_size < pool->max_size) {
        object_pool_node_t* node = malloc(sizeof(object_pool_node_t));
        if (node) {
            node->object = object;
            node->next = pool->head;
            pool->head = node;
            pool->current_size++;
            pthread_mutex_unlock(&pool->mutex);
            return;
        }
    }
    
    pthread_mutex_unlock(&pool->mutex);
    free(object);  // 池已满，直接释放
}
