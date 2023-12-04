#include "vkdescriptors.h"
#include "vkDevice.h"
#include "model.h"
#include <stdexcept>

namespace kge
{

	vkDescriptorSetLayout::vkDescriptorSetLayout(vkDevice* device, kgeModel* model)
		:m_Device(device), m_Model(model)
	{

	}

	vkDescriptorSetLayout::~vkDescriptorSetLayout()
	{
		vkDestroyDescriptorPool(m_Device->getDevice(), m_DescriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(m_Device->getDevice(), m_DescriptorSetLayout, nullptr);
	}

	VkDescriptorSetLayoutBinding vkDescriptorSetLayout::createBinding(VkDescriptorSetLayoutBinding LayoutBinding, uint32_t binding, VkDescriptorType type, uint32_t count, VkShaderStageFlags flags)
	{
		LayoutBinding.binding = 0;
		LayoutBinding.descriptorType = type;
		LayoutBinding.descriptorCount = count;
		LayoutBinding.stageFlags = flags;
		LayoutBinding.pImmutableSamplers = nullptr; // Optional /* Only relevant for image sampling descriptors. Leave nullptr for now. */

		return LayoutBinding;
	}

	VkDescriptorSetLayout vkDescriptorSetLayout::createDescriptorSetLayout(VkDescriptorSetLayout layout, const VkDescriptorSetLayoutBinding& binding, uint32_t bindingCount)
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = bindingCount;
		layoutInfo.pBindings = &binding;

		if (vkCreateDescriptorSetLayout(m_Device->getDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
		return m_DescriptorSetLayout;
	}

	void vkDescriptorSetLayout::createDescriptorPool(int maxframesinflight)
	{
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = static_cast<uint32_t>(maxframesinflight);

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = static_cast<uint32_t>(maxframesinflight);

		if (vkCreateDescriptorPool(m_Device->getDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error(" failed to create descriptor pool!\n");
		}
	}

	void vkDescriptorSetLayout::createDescriptorSets(int maxframesinflight)
	{
		std::vector<VkDescriptorSetLayout> layouts(maxframesinflight,m_DescriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_DescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(maxframesinflight);
		allocInfo.pSetLayouts = layouts.data();

		m_DescriptorSets.resize(maxframesinflight);
		if (vkAllocateDescriptorSets(m_Device->getDevice(), &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets!\n");
		}

		for (size_t i = 0; i < maxframesinflight; i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = m_Model->getUniformBuffers()[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(kgeModel::UniformBufferObject);

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_DescriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr;
			descriptorWrite.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(m_Device->getDevice(), 1, &descriptorWrite, 0, nullptr);


		}
	}
}