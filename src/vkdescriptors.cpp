#include "vkdescriptors.h"
#include "vkDevice.h"
#include <stdexcept>

namespace kge
{
	vkDescriptorSetLayout::vkDescriptorSetLayout(vkDevice* device)
		:m_Device(device)
	{

	}

	vkDescriptorSetLayout::~vkDescriptorSetLayout()
	{
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
	}
}