#pragma once
#include "utils/vkUtils.hpp"
#include "types.hpp"
#include <unordered_map>


namespace karhu
{
    class Texture;
    struct Vulkan_Device;
    class kDescriptors
    {
    public:
        kDescriptors(Vulkan_Device& device);
        ~kDescriptors();

        kDescriptors(const kDescriptors&) = delete;
        void operator=(const kDescriptors&) = delete;
        kDescriptors(kDescriptors&&) = delete;
        kDescriptors& operator=(kDescriptors&&) = delete;

        VkDescriptorSetLayout createDescriptorSetLayout(VkDescriptorSetLayout layout);
        void addBind(uint32_t binding, VkDescriptorType type, uint32_t descriptorcount, VkShaderStageFlagBits stageflags);
        void createDescriptorPool(const int maxFramesInFlight);
        std::vector<VkDescriptorSet> createDescriptorSets(const int maxFramesInFlight, VkDescriptorSetLayout layout, const std::vector<VkBuffer>& uniformBuffers, const Texture& texture);
        //void cleanUp(VkDevice device);

    private:
        Vulkan_Device& m_VkDevice;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_Bindings;
        VkDescriptorPool m_DescriptorPool;
    };
}