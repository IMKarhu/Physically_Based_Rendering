#pragma once
#include "utils/vkUtils.hpp"
#include "types.hpp"


namespace karhu
{
    class kDescriptors
    {
    public:
        kDescriptors();
        ~kDescriptors();

        /*VkDescriptorSetLayout*/ void createDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout& layout);
        void createDescriptorPool(VkDevice device, const int maxFramesInFlight);
        void createDescriptorSets(VkDevice device, const int maxFramesInFlight, VkDescriptorSetLayout layout, const std::vector<VkBuffer>& uniformBuffers);
        void cleanUp(VkDevice device);

    private:
    };
}