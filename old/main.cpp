#include "renderer.h"

#include <iostream>
#include <stdexcept>
#include <cstdlib>

class Application
{
public:
	void run()
	{
		printf("hello");
		ren.Run();
	}
private:
	Renderer ren;
};

int main()
{
	Application app;

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