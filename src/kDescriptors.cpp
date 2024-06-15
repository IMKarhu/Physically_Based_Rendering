#include "kDescriptors.hpp"

namespace karhu
{
    kDescriptors::kDescriptors()
    {
    }

    kDescriptors::~kDescriptors()
    {
    }

    /*VkDescriptorSetLayout*/ void kDescriptors::createDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout& layout)
    {
    }

    void kDescriptors::createDescriptorPool(VkDevice device, const int maxFramesInFlight)
    {
    }

    void kDescriptors::createDescriptorSets(VkDevice device, const int maxFramesInFlight, VkDescriptorSetLayout layout, const std::vector<VkBuffer>& uniformBuffers)
    {
    }

    void kDescriptors::cleanUp(VkDevice device)
    {

    }


}