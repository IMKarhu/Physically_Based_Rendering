#pragma once

#include "window.h"

#include <vector>

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class VulkanRenderer
{
public:
	VulkanRenderer();

	~VulkanRenderer();

	void createInstance();
	void Run();
private:
	std::unique_ptr<Window> m_Window = std::make_unique<Window>("Vulkan renderer", 800, 600);

	VkInstance m_Instance;
	bool checkValidationSupport() const;
};