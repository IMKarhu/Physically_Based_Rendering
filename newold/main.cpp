#include "vkApplication.h"

int main()
{
	kge::vkApplication app;
	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	EXIT_SUCCESS;
}