#pragma once
#include "vulkan/vulkan.h"
#include "../uniformBuffers.h"
#include "../device.h"
#include <vector>

namespace kge
{
	class Descriptor
	{
	public:
		Descriptor(Device* device);
		~Descriptor();
		void createDescriptorSetLayout();
		void createDescriptorPool(const int maxframesinflight);
		void createDescriptorSets(UniformBuffers *uniformbuffers, const int maxframesinflight, const VkImageView& textureImageView, const VkSampler& textureSampler);

		[[nodiscard]] const VkDescriptorSetLayout getDSlayout() const { return m_DescriptorSetLayout; }
		[[nodiscard]] const std::vector<VkDescriptorSet> getDSsets() const { return m_DescriptorSets; }
	private:
		/* Pointers to classes. */
		Device* m_Device;
		/* Class members. */
		VkDescriptorSetLayout m_DescriptorSetLayout;
		VkDescriptorPool m_DescriptorPool;
		std::vector<VkDescriptorSet> m_DescriptorSets;
	};
}