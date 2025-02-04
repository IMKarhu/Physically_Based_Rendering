#pragma once
#include <GLFW/glfw3.h>

namespace karhu
{
	class kEntity;

	class keyboardMovement
	{
	public:
		void update(GLFWwindow* window, float dt, kEntity& entity, int width, int height);
	private:
		float m_Speed = 2.5f;
	};
}