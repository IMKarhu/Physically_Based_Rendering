#pragma once
#include "kDevice.hpp"
#include "utils/vkUtils.hpp"
#include "types.hpp"


namespace karhu
{
    class kEntity;

    class kDescriptors
    {
    public:
        kDescriptors(Vulkan_Device& device);
        ~kDescriptors();

        kDescriptors(const kDescriptors&) = delete;
        void operator=(const kDescriptors&) = delete;

        void createDescriptorSetLayout();
        void createDescriptorPool(uint32_t entityCount);
        void createDescriptorSets(std::vector<kEntity>& entities);
        void cleanUp(VkDevice device);

        const VkDescriptorSetLayout& getDescriptorLayout() const { return m_DescriptorLayout; }

    private:
        Vulkan_Device& m_Device;
        VkDescriptorSetLayout m_DescriptorLayout;
        VkDescriptorPool m_DescriptorPool;
    };
}