#pragma once

#include "window.h"
#include "instance.h"
#include "device.h"

#include <memory>

class Renderer
{
public:
	Renderer() = default;
	Renderer(const Renderer&) = delete; /* Disallows copying. */
	Renderer(Renderer&&) = delete; /* Disallows move operation. */
	~Renderer();

	Renderer& operator=(const Renderer&) = delete;
	Renderer& operator=(Renderer&&) = delete;

	void createSurface();

	void Run();
private:
	VkSurfaceKHR m_Surface;
	std::unique_ptr<Window> m_Window = std::make_unique<Window>();
	std::unique_ptr<Instance> m_Instance = std::make_unique<Instance>();
	std::unique_ptr<Device> m_Device;
};