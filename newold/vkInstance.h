#pragma once
#include "window.h"
#include "vulkan/vulkan.h"
#include <iostream>

namespace kge
{
	class vkInstance : public Window
	{
	public:
		vkInstance(const bool& enableValidationLayers);
		~vkInstance();

		void cleanUp(const bool& enableValidationLayers);

		[[nodiscard]] const VkInstance getInstance() const { return m_Instance; }
	private:
		/* Class members. */
		VkInstance m_Instance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;

		/* Private functions. */
		void setupDebugMessenger(const bool& enableValidationLayers);
	};
}
