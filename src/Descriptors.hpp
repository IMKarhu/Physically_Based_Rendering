#pragma once
#include "Device.hpp"
#include "types.hpp"

#include <unordered_map>


namespace karhu
{
    class Entity;

    class Descriptors
    {
    public:
        Descriptors(Device& device);
        ~Descriptors();

        Descriptors(const Descriptors&) = delete;
        void operator=(const Descriptors&) = delete;

        VkDescriptorSetLayout createDescriptorSetLayout(std::vector< VkDescriptorSetLayoutBinding>& bindings);
        void bind(std::vector<VkDescriptorSetLayoutBinding>& bindings, uint32_t binding, VkDescriptorType type, uint32_t descriptorCount, VkShaderStageFlags flags);
        VkDescriptorPool createDescriptorPool(uint32_t maxSets);
        void createDescriptorSets(std::vector<Entity>& entities, VkDescriptorSetLayout layout, VkDescriptorPool pool);
        void createDescriptorSets(VkDescriptorSetLayout layout, VkDescriptorPool pool);
        void cleanUp(VkDevice device);
        void addPoolElement(VkDescriptorType type, uint32_t count);
        void writeImg(VkDescriptorSet& set, uint32_t binding, VkDescriptorImageInfo info, uint32_t bindingID);
        void writeBuffer(VkDescriptorSet& set, uint32_t binding, VkDescriptorType type, VkDescriptorBufferInfo info, uint32_t bindingID);
        void fillWritesMap(uint32_t bindingID);
        void allocateDescriptor(VkDescriptorSet& set, VkDescriptorSetLayout layout, VkDescriptorPool pool);

    private:
        Device& m_device;
        std::vector<VkDescriptorPoolSize> m_poolSizes;
        std::vector<VkWriteDescriptorSet>m_writesData;
        std::unordered_map<uint32_t, std::vector<VkWriteDescriptorSet>> m_writes;
    };
}
