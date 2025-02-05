#pragma once
#include <GLFW/glfw3.h>

namespace karhu
{
	//class kEntity;
	class kCamera;

	class keyboardMovement
	{
	public:
		void update(GLFWwindow* window, float dt, kCamera& camera, int width, int height);
	private:
		float m_Speed = 1.0f;
		float m_RotateSpeed = 0.5f;
	};
}