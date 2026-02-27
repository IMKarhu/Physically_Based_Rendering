#pragma once
#include "vulkan/vulkan.h"
#include <iostream>

#define VK_CHECK(x) checkErrorStatus(x)

static void checkErrorStatus(VkResult x)
{
    do
    {
        VkResult err = x;
        if (err)
        {
            std::cout << "Vulkan Error: " << err << std::endl;
            std::abort();
        }
    } while (0);
}
