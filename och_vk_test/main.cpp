#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <vector>
#include <cstdlib>

#include "och_fmt.h"
#include "error_handling.h"

#define OCH_VALIDATE

struct hello_vulkan
{
	uint32_t window_width = 1440;
	uint32_t window_height = 810;

	GLFWwindow* window;

	err_info run()
	{
		init_window();

		if (err_info err = init_vulkan())
			return err;

		if (err_info err = main_loop())
			return err;

		cleanup();
	}

	void init_window()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(window_width, window_height, "Hello Vulkan", nullptr, nullptr);
	}

	err_info init_vulkan()
	{
		return {};
	}

	err_info main_loop()
	{
		while (!glfwWindowShouldClose(window))
			glfwPollEvents();

		return {};
	}

	void cleanup()
	{
		glfwDestroyWindow(window);

		glfwTerminate();
	}
};

int main()
{
	hello_vulkan vk;

	err_info err = vk.run();

	if (err)
		och::print("\nError {} on line {}\n", err.errcode, err.line_number);
	else
		och::print("\nProcess terminated successfully\n");
}
