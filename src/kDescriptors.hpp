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
        kDescriptors(std::shared_ptr<Vulkan_Device> device);
        ~kDescriptors();

        VkDescriptorSetLayout createDescriptorSetLayout(VkDescriptorSetLayout layout);
        void addBind(uint32_t binding, VkDescriptorType type, uint32_t descriptorcount, VkShaderStageFlagBits stageflags);
        void createDescriptorPool(const int maxFramesInFlight);
        std::vector<VkDescriptorSet> createDescriptorSets(const int maxFramesInFlight, VkDescriptorSetLayout layout, const std::vector<VkBuffer>& uniformBuffers, const Texture& texture);
        //void cleanUp(VkDevice device);

    private:
        std::shared_ptr<Vulkan_Device> m_VkDevice;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_Bindings;
        VkDescriptorPool m_DescriptorPool;
        std::vector<VkDescriptorSet> m_DescriptorSets;
    };
}