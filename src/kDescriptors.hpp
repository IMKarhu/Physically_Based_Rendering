#pragma once
#include "kDevice.hpp"
#include "utils/vkUtils.hpp"
#include "types.hpp"
#include <unordered_map>


namespace karhu
{
    class kEntity;

    //class kDescriptors
    //{
    //public:
    //    kDescriptors(Vulkan_Device& device);
    //    ~kDescriptors();

    //    kDescriptors(const kDescriptors&) = delete;
    //    void operator=(const kDescriptors&) = delete;

    //    VkDescriptorSetLayout createDescriptorSetLayout();
    //    VkDescriptorPool createDescriptorPool(uint32_t maxSets);
    //    void updateDescriptorSet();
    //    void addBinding(uint32_t binding, VkDescriptorType type, uint32_t descriptorCount, VkShaderStageFlags flags);
    //    void addPool(VkDescriptorType type, uint32_t count);
    //    void allocateSet(VkDescriptorPool pool, VkDescriptorSetLayout layout, VkDescriptorSet& set);
    //    void prepareBuffer(VkPhysicalDeviceDescriptorBufferPropertiesEXT properties);

    //    void write(kBuffer& buffer, VkDescriptorSet& set);
    //    void writeImg(kEntity& entity);
    //    void freeBindings();

    //    void cleanUp(VkDevice device);

    //    //const VkDescriptorSetLayout& getDescriptorLayout() const { return m_DescriptorLayout; }

    //private:
    //    Vulkan_Device& m_Device;
    //    //VkDescriptorSetLayout m_DescriptorLayout;
    //    VkDescriptorPool m_DescriptorPool;
    //    std::vector<VkDescriptorPoolSize> m_PoolSizes;
    //    std::vector<VkDescriptorSetLayoutBinding> m_Bindings;
    //    std::vector<VkWriteDescriptorSet> m_SetWrites;
    //};

    class LveDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(Vulkan_Device& device) : m_Device{ device } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<LveDescriptorSetLayout> build() const;

        private:
            Vulkan_Device& m_Device;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        LveDescriptorSetLayout(
            Vulkan_Device& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~LveDescriptorSetLayout();
        LveDescriptorSetLayout(const LveDescriptorSetLayout&) = delete;
        LveDescriptorSetLayout& operator=(const LveDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        Vulkan_Device& m_Device;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class LveDescriptorWriter;
    };

    class LveDescriptorPool {
    public:
        class Builder {
        public:
            Builder(Vulkan_Device& device) : m_Device{ device } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<LveDescriptorPool> build() const;

        private:
            Vulkan_Device& m_Device;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        LveDescriptorPool(
            Vulkan_Device& device,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~LveDescriptorPool();
        LveDescriptorPool(const LveDescriptorPool&) = delete;
        LveDescriptorPool& operator=(const LveDescriptorPool&) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        Vulkan_Device& m_Device;
        VkDescriptorPool descriptorPool;

        friend class LveDescriptorWriter;
    };

    class LveDescriptorWriter {
    public:
        LveDescriptorWriter(LveDescriptorSetLayout& setLayout, LveDescriptorPool& pool);

        LveDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        LveDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        LveDescriptorSetLayout& setLayout;
        LveDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}