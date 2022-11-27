#include "queue.hpp"

#include <vulkan/vulkan_core.h>

using namespace std::string_literals;

namespace avocado::vulkan {

Queue::Queue(VkQueue vq):
    _queue(vq) {
}

VkQueue Queue::getHandle() {
    return _queue;
}

}

