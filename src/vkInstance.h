#pragma once
#include "window.h"
#include <iostream>

namespace kge
{
	class vkInstance : public Window
	{
	public:
		vkInstance(const bool& enableValidationLayers);
		vkInstance(const vkInstance&) = delete; /* Disallows copying. */
		//vkInstance(vkInstance&&) = delete; /* Disallows move operation. */
		 ~vkInstance();

		 vkInstance& operator=(const vkInstance&) = delete;
		 //vkInstance& operator=(vkInstance&&) = delete;

		void cleanUp(const bool& enableValidationLayers);

		[[nodiscard]] const VkInstance getInstance() const { return m_Instance; }
		[[nodiscard]] const VkSurfaceKHR getSurface() const { return m_Surface; }
	private:
		/* Class members. */
		VkInstance m_Instance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;
		VkSurfaceKHR m_Surface;

		/* Private functions. */
		void setupDebugMessenger(const bool& enableValidationLayers);
		void createSurface();
	};
}
