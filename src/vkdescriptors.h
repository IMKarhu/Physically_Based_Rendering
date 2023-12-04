#pragma once
#include "vulkan/vulkan.h"
#include <vector>

namespace kge
{
	class vkDevice;
	class kgeModel;

	class vkDescriptorSetLayout
	{
	public:
		typedef struct UBOBuffer;
		vkDescriptorSetLayout(vkDevice* device, kgeModel* model);
		~vkDescriptorSetLayout();
		vkDescriptorSetLayout(const vkDescriptorSetLayout&) = delete; /* Disallows copying. */
		vkDescriptorSetLayout& operator=(const vkDescriptorSetLayout&) = delete;

		VkDescriptorSetLayoutBinding createBinding(VkDescriptorSetLayoutBinding Layoutbinding, uint32_t binding, VkDescriptorType type, uint32_t count, VkShaderStageFlags flags);
		VkDescriptorSetLayout createDescriptorSetLayout(VkDescriptorSetLayout layout, const VkDescriptorSetLayoutBinding& binding, uint32_t bindingCount);
		void createDescriptorPool(int maxframesinflight);
		void createDescriptorSets(int maxframesinflight);

		[[nodiscard]] const VkDescriptorSetLayout getLayout() const { return m_DescriptorSetLayout; }
		[[nodiscard]] const std::vector<VkDescriptorSet> getDescriptorSets() const { return m_DescriptorSets; }
	private:
		vkDevice* m_Device;
		kgeModel* m_Model;
		VkDescriptorSetLayout m_DescriptorSetLayout;
		VkDescriptorPool m_DescriptorPool;
		std::vector<VkDescriptorSet> m_DescriptorSets;
		VkPipelineLayout m_PipelineLayout;
		
	};
}