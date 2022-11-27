#ifndef AVOCADO_VULKAN_PRESENT_QUEUE
#define AVOCADO_VULKAN_PRESENT_QUEUE

#include "queue.hpp"
#include "swapchain.hpp"

namespace avocado::vulkan {

class PresentQueue final: public Queue {
public:
    explicit PresentQueue(VkQueue queue);
    
    void setWaitSemaphores(const std::vector<VkSemaphore> &waitSemaphores);
    void setSwapchains(std::vector<Swapchain> &swapchains);
    void setImageIndices(const std::vector<uint32_t> &imageIndices);
    void present();
 
private:
    std::vector<VkSwapchainKHR> _swapchainHandles;
    VkPresentInfoKHR _presentInfo{};
};

}

#endif

