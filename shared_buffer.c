typedef struct shared_message {
    void* data;
    size_t size;
    _Atomic int ref_count;
    void (*destructor)(void*);  // 可选的析构函数
} shared_message_t;

shared_message_t* shared_message_create(void* data, size_t size, int owned) {
    shared_message_t* msg = malloc(sizeof(shared_message_t));
    if (!msg) return NULL;
    
    msg->data = data;
    msg->size = size;
    atomic_store(&msg->ref_count, 1);
    msg->destructor = owned ? free : NULL;
    return msg;
}

void shared_message_ref(shared_message_t* msg) {
    if (msg) atomic_fetch_add(&msg->ref_count, 1);
}

void shared_message_unref(shared_message_t* msg) {
    if (msg && atomic_fetch_sub(&msg->ref_count, 1) == 1) {
        if (msg->destructor) {
            msg->destructor(msg->data);
        }
        free(msg);
    }
}
