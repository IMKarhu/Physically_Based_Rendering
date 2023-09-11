#pragma once
#include "vulkan/vulkan.h"

namespace kge
{
	class vkDevice;

	class vkDescriptorSetLayout
	{
	public:
		vkDescriptorSetLayout(vkDevice* device);
		~vkDescriptorSetLayout();
		vkDescriptorSetLayout(const vkDescriptorSetLayout&) = delete; /* Disallows copying. */
		vkDescriptorSetLayout& operator=(const vkDescriptorSetLayout&) = delete;

		VkDescriptorSetLayoutBinding createBinding(VkDescriptorSetLayoutBinding Layoutbinding, uint32_t binding, VkDescriptorType type, uint32_t count, VkShaderStageFlags flags);
		VkDescriptorSetLayout createDescriptorSetLayout(VkDescriptorSetLayout layout, const VkDescriptorSetLayoutBinding& binding, uint32_t bindingCount);

		[[nodiscard]] const VkDescriptorSetLayout getLayout() const { return m_DescriptorSetLayout; }
	private:
		vkDevice* m_Device;
		VkDescriptorSetLayout m_DescriptorSetLayout;
		VkPipelineLayout m_PipelineLayout;

		
	};
}