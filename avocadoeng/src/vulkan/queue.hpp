#ifndef AVOCADO_VULKAN_QUEUE
#define AVOCADO_VULKAN_QUEUE

#include "../errorstorage.hpp"

#include <vector>

namespace avocado::vulkan {

class CommandBuffer;

struct VkQueue_T;

class Queue: public core::ErrorStorage {
public:
    explicit Queue(VkQueue q);
    virtual ~Queue() = default;

    VkQueue getHandle() noexcept;
    void waitIdle() noexcept;

private:
    VkQueue _queue = VK_NULL_HANDLE;
};

}

#endif

