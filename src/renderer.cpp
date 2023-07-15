#include "renderer.h"

Renderer::~Renderer()
{
}

void Renderer::Run()
{
	m_Window->setTitle("Vulkan renderer");
	m_Window->setWidth(800);
	m_Window->setHeight(600);
	m_Window->createWindow();
	while (!m_Window->windowShouldCose())
	{
		m_Window->pollEvents();
	}
}
