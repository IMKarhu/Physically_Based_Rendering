#include "renderer.h"

Renderer::~Renderer()
{
	vkDestroySurfaceKHR(m_Instance->getInstance(), m_Surface, nullptr);
	vkDestroyInstance(m_Instance->getInstance(), nullptr);
}

void Renderer::createSurface()
{
	if (glfwCreateWindowSurface(m_Instance->getInstance(), m_Window->getWindow(), nullptr, &m_Surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
}

void Renderer::Run()
{
	m_Window->setTitle("Vulkan renderer");
	m_Window->setWidth(800);
	m_Window->setHeight(600);
	m_Window->createWindow();
	createSurface();
	std::make_unique<Device>(m_Surface, m_Instance->getInstance(), true, validationLayers);
	while (!m_Window->windowShouldCose())
	{
		m_Window->pollEvents();
	}
}
