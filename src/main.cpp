#include "vulkanRenderer.h"

int main()
{
	VulkanRenderer vRen;
	try
	{
		vRen.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	EXIT_SUCCESS;
}