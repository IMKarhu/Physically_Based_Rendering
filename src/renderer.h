#pragma once

#include "window.h"
#include "instance.h"

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

	void Run();
private:
	std::unique_ptr<Window> m_Window = std::make_unique<Window>();
	std::unique_ptr<Instance> m_Instance = std::make_unique<Instance>();
};