#ifndef GLOBAL_ARENA
#   define GLOBAL_ARENA
#else
#   error "Unity build is broken."
#endif

#include <memory_resource>

std::pmr::monotonic_buffer_resource global_arena;

template<typename T, typename... Args>
T* arena_new(Args&&... args) {
    std::pmr::polymorphic_allocator<T> alloc{ &global_arena };
    T* ptr = alloc.allocate(1);
    std::construct_at(ptr, std::forward<Args>(args)...);
    return ptr;
}
