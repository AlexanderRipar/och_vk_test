#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <vector>
#include "och_fmt.h"
#include "och_fio.h"
#include "error_handling.h"

#define OCH_VALIDATE

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback_fn(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data)
{
	user_data;

	const char* severity_msg = nullptr, * type_msg = nullptr;

	switch (severity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		severity_msg = "verbose"; break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		severity_msg = "info"; break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		severity_msg = "WARNING"; break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		severity_msg = "ERROR"; break;
	}

	switch (type)
	{
	case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
		type_msg = "general"; break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
		type_msg = "performance"; break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
		type_msg = "validation"; break;
	}

	och::print("{}/{}: {}\n\n", severity_msg, type_msg, callback_data->pMessage);

	return VK_FALSE;
}

void framebuffer_resize_callback_fn(GLFWwindow* window, int width, int height);

VkDebugUtilsMessengerCreateInfoEXT populate_messenger_create_info() noexcept
{
	VkDebugUtilsMessengerCreateInfoEXT create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	create_info.pfnUserCallback = debug_callback_fn;
	create_info.pUserData = nullptr;

	return create_info;
}

struct hello_vulkan
{
	static constexpr uint32_t max_frames_in_flight = 2;

#ifdef OCH_VALIDATE
	static constexpr const char* required_validation_layers[]{ "VK_LAYER_KHRONOS_validation" };
#endif // OCH_VALIDATE

	static constexpr const char* required_device_extensions[]{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	struct queue_family_indices
	{
		uint32_t graphics_idx = ~0u;
		uint32_t compute_idx = ~0u;
		uint32_t transfer_idx = ~0u;
		uint32_t present_idx = ~0u;

		operator bool() const noexcept { return graphics_idx != ~0u && compute_idx != ~0u && transfer_idx != ~0u && present_idx != ~0u; }

		bool discrete_present_family() const noexcept { return graphics_idx != present_idx; }
	};

	struct swapchain_support_details
	{
		VkSurfaceCapabilitiesKHR capabilites{};
		std::vector<VkSurfaceFormatKHR> formats{};
		std::vector<VkPresentModeKHR> present_modes{};
	};

	uint32_t window_width = 1440;
	uint32_t window_height = 810;

	GLFWwindow* window = nullptr;

	VkInstance vk_instance = nullptr;

	VkSurfaceKHR vk_surface = nullptr;

	VkPhysicalDevice vk_physical_device = nullptr;

	VkDevice vk_device = nullptr;

	VkQueue vk_graphics_queue = nullptr;

	VkQueue vk_present_queue = nullptr;

	VkSwapchainKHR vk_swapchain = nullptr;

	std::vector<VkImage> vk_swapchain_images;

	VkFormat vk_swapchain_format;

	VkExtent2D vk_swapchain_extent;

	std::vector<VkImageView> vk_swapchain_views;

	VkRenderPass vk_render_pass = nullptr;

	VkPipelineLayout vk_pipeline_layout = nullptr;

	VkPipeline vk_graphics_pipeline = nullptr;

	std::vector<VkFramebuffer> vk_swapchain_framebuffers;

	VkCommandPool vk_command_pool = nullptr;

	std::vector<VkCommandBuffer> vk_command_buffers;

	VkSemaphore vk_image_available_semaphores[max_frames_in_flight];
	VkSemaphore vk_render_complete_semaphores[max_frames_in_flight];
	VkFence vk_inflight_fences[max_frames_in_flight];
	std::vector<VkFence> vk_images_inflight_fences;

	size_t curr_frame = 0;

	bool framebuffer_resized = false;

#ifdef OCH_VALIDATE
	VkDebugUtilsMessengerEXT vk_debug_messenger = nullptr;
#endif // OCH_VALIDATE

	err_info run()
	{
		init_window();

		check(init_vulkan());

		check(main_loop());

		cleanup();

		return {};
	}

	void init_window()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(window_width, window_height, "Hello Vulkan", nullptr, nullptr);

		glfwSetWindowUserPointer(window, this);

		glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback_fn);
	}

	err_info init_vulkan()
	{
		check(create_vk_instance());

		check(create_vk_debug_messenger());

		check(create_vk_surface());

		check(select_vk_physical_device());

		check(create_vk_logical_device());

		check(create_vk_swapchain());

		check(get_vk_swapchain_views());

		check(create_vk_render_pass());

		check(create_vk_graphics_pipeline());

		check(create_vk_swapchain_framebuffers());

		check(create_vk_command_pool());

		check(create_vk_command_buffers());

		check(create_vk_sync_objects());

		return {};
	}

	err_info create_vk_instance()
	{
		VkApplicationInfo app_info{};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = "Hello Vulkan";
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName = "No Engine";
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion = VK_API_VERSION_1_0;

		std::vector<const char*> extensions;

		och::range<const char* const> layers;

		check(get_required_instance_extensions(extensions));

		check(get_required_instance_validation_layers(layers));

		VkInstanceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &app_info;
		create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		create_info.ppEnabledExtensionNames = extensions.data();
		create_info.enabledLayerCount = static_cast<uint32_t>(layers.len());
		create_info.ppEnabledLayerNames = layers.beg;

#ifdef OCH_VALIDATE

		VkDebugUtilsMessengerCreateInfoEXT messenger_info = populate_messenger_create_info();
		create_info.pNext = &messenger_info;

#endif // OCH_VALIDATE

		check(vkCreateInstance(&create_info, nullptr, &vk_instance));

		return {};
	}

	err_info get_required_instance_extensions(std::vector<const char*>& required_extensions)
	{
		uint32_t glfw_cnt;

		const char** glfw_names = glfwGetRequiredInstanceExtensions(&glfw_cnt);

		required_extensions = std::vector(glfw_names, glfw_names + glfw_cnt);

#ifdef OCH_VALIDATE
		required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif // OCH_VALIDATE

		uint32_t vk_cnt;

		check(vkEnumerateInstanceExtensionProperties(nullptr, &vk_cnt, nullptr));

		std::vector<VkExtensionProperties> available_exts(vk_cnt);

		check(vkEnumerateInstanceExtensionProperties(nullptr, &vk_cnt, available_exts.data()));

		for (const auto& req : required_extensions)
		{
			bool is_available = false;

			for (const auto& avl : available_exts)
				if (!strcmp(req, avl.extensionName))
				{
					is_available = true;
					break;
				}

			if (!is_available)
				return ERROR(1);
		}

		return {};
	}

	err_info get_required_instance_validation_layers(och::range<const char* const>& required_layers)
	{
#ifdef OCH_VALIDATE
		uint32_t layer_cnt;

		check(vkEnumerateInstanceLayerProperties(&layer_cnt, nullptr));

		std::vector<VkLayerProperties> available_layers(layer_cnt);

		check(vkEnumerateInstanceLayerProperties(&layer_cnt, available_layers.data()));

		for (const auto& req : required_validation_layers)
		{
			bool is_available = false;

			for (const auto& avl : available_layers)
				if (!strcmp(req, avl.layerName))
				{
					is_available = true;
					break;
				}

			if (!is_available)
				return ERROR(1);
		}

		required_layers = och::range<const char* const>(required_validation_layers);

#endif // OCH_VALIDATE

		return {};
	}

	err_info create_vk_debug_messenger()
	{
#ifdef OCH_VALIDATE

		VkDebugUtilsMessengerCreateInfoEXT create_info = populate_messenger_create_info();

		auto create_fn = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(vk_instance, "vkCreateDebugUtilsMessengerEXT"));

		if (create_fn)
		{
			check(create_fn(vk_instance, &create_info, nullptr, &vk_debug_messenger));
		}
		else
			return ERROR(1);

#endif // OCH_VALIDATE

		return {};
	}

	err_info create_vk_surface()
	{
		check(glfwCreateWindowSurface(vk_instance, window, nullptr, &vk_surface));

		return {};
	}

	err_info select_vk_physical_device()
	{
		uint32_t device_cnt;

		check(vkEnumeratePhysicalDevices(vk_instance, &device_cnt, nullptr));

		if (!device_cnt)
			return ERROR(1);

		std::vector<VkPhysicalDevice> devices(device_cnt);

		check(vkEnumeratePhysicalDevices(vk_instance, &device_cnt, devices.data()));

		for (auto& dev : devices)
		{
			VkPhysicalDeviceProperties props;

			vkGetPhysicalDeviceProperties(dev, &props);

			VkPhysicalDeviceFeatures feats;

			vkGetPhysicalDeviceFeatures(dev, &feats);

			if (props.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				continue;

			if (!feats.geometryShader)
				continue;

			err_info err;

			queue_family_indices queue_families;

			check(query_queue_families(dev, vk_surface, queue_families));

			if (!queue_families)
			{
				check(err);

				continue;
			}

			bool supports_required_dev_extensions;

			check(check_required_device_extension_support(dev, supports_required_dev_extensions));

			if (!supports_required_dev_extensions)
				continue;
			
			swapchain_support_details swapchain_details;

			check(query_swapchain_support(dev, vk_surface, swapchain_details));

			if (swapchain_details.formats.empty() || swapchain_details.present_modes.empty())
				continue;

			vk_physical_device = dev;

			break;
		}

		if (!vk_physical_device)
			return ERROR(1);

		return {};
	}

	err_info check_required_device_extension_support(VkPhysicalDevice physical_dev, bool& all_supported)
	{
		uint32_t extension_cnt;

		check(vkEnumerateDeviceExtensionProperties(physical_dev, nullptr, &extension_cnt, nullptr));

		std::vector<VkExtensionProperties> available_dev_extensions(extension_cnt);

		check(vkEnumerateDeviceExtensionProperties(physical_dev, nullptr, &extension_cnt, available_dev_extensions.data()));

		all_supported = true;

		for (const auto& req : required_device_extensions)
		{
			bool is_available = false;

			for (const auto& avl : available_dev_extensions)
				if (!strcmp(req, avl.extensionName))
				{
					is_available = true;
					break;
				}

			if (!is_available)
			{
				all_supported = false;
				return {};
			}
		}

		return {};
	}

	err_info create_vk_logical_device()
	{
		queue_family_indices family_indices;

		check(query_queue_families(vk_physical_device, vk_surface, family_indices));

		float graphics_queue_priority = 1.0F;

		VkDeviceQueueCreateInfo queue_infos[2]{};
		queue_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_infos[0].queueFamilyIndex = family_indices.graphics_idx;
		queue_infos[0].queueCount = 1;
		queue_infos[0].pQueuePriorities = &graphics_queue_priority;
		queue_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_infos[1].queueFamilyIndex = family_indices.present_idx;
		queue_infos[1].queueCount = 1;
		queue_infos[1].pQueuePriorities = &graphics_queue_priority;

		VkPhysicalDeviceFeatures enabled_dev_features{};

		VkDeviceCreateInfo dev_info{};
		dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		dev_info.pQueueCreateInfos = queue_infos;
		dev_info.queueCreateInfoCount = 1 + family_indices.discrete_present_family();
		dev_info.pEnabledFeatures = &enabled_dev_features;
		dev_info.ppEnabledExtensionNames = required_device_extensions;
		dev_info.enabledExtensionCount = sizeof(required_device_extensions) / sizeof(*required_device_extensions);
#ifdef OCH_VALIDATE
		dev_info.ppEnabledLayerNames = required_validation_layers;
		dev_info.enabledLayerCount = sizeof(required_validation_layers) / sizeof(*required_validation_layers);
#else
		dev_info.ppEnabledLayerNames = nullptr;
		dev_info.enabledLayerCount = 0;
#endif // OCH_VALIDATE

		check(vkCreateDevice(vk_physical_device, &dev_info, nullptr, &vk_device));

		vkGetDeviceQueue(vk_device, family_indices.graphics_idx, 0, &vk_graphics_queue);

		vkGetDeviceQueue(vk_device, family_indices.present_idx, 0, &vk_present_queue);

		return {};
	}

	err_info create_vk_swapchain()
	{
		swapchain_support_details swapchain_details;

		check(query_swapchain_support(vk_physical_device, vk_surface, swapchain_details));

		VkSurfaceFormatKHR chosen_format = swapchain_details.formats[0];

		for (const auto& format : swapchain_details.formats)
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				chosen_format = format;

				break;
			}

		VkPresentModeKHR chosen_present_mode = VK_PRESENT_MODE_FIFO_KHR;

		for(const auto& present_mode : swapchain_details.present_modes)
			if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				chosen_present_mode = present_mode;

				break;
			}

		VkExtent2D chosen_extent = swapchain_details.capabilites.currentExtent;

		if (swapchain_details.capabilites.currentExtent.width == ~0u)
		{
			uint32_t width, height;

			glfwGetFramebufferSize(window, reinterpret_cast<int*>(&width), reinterpret_cast<int*>(&height));

			if (width > swapchain_details.capabilites.maxImageExtent.width)
				width = swapchain_details.capabilites.maxImageExtent.width;
			else if (width < swapchain_details.capabilites.minImageExtent.width)
				width = swapchain_details.capabilites.minImageExtent.width;

			if (height > swapchain_details.capabilites.maxImageExtent.height)
				height = swapchain_details.capabilites.maxImageExtent.height;
			else if (height < swapchain_details.capabilites.minImageExtent.height)
				height = swapchain_details.capabilites.minImageExtent.height;

			chosen_extent.width = static_cast<uint32_t>(width);
			chosen_extent.height = static_cast<uint32_t>(height);
		}

		uint32_t chosen_image_count = swapchain_details.capabilites.minImageCount + 1;

		if (swapchain_details.capabilites.maxImageCount && chosen_image_count > swapchain_details.capabilites.maxImageCount)
			chosen_image_count = swapchain_details.capabilites.maxImageCount;

		VkSwapchainCreateInfoKHR info{};

		info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		info.imageFormat = chosen_format.format;
		info.imageColorSpace = chosen_format.colorSpace;
		info.presentMode = chosen_present_mode;
		info.imageExtent = chosen_extent;
		info.minImageCount = chosen_image_count;
		info.surface = vk_surface;
		info.imageArrayLayers = 1;
		info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		info.preTransform = swapchain_details.capabilites.currentTransform;
		info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		info.clipped = VK_TRUE;
		info.oldSwapchain = nullptr;

		queue_family_indices family_indices;

		check(query_queue_families(vk_physical_device, vk_surface, family_indices));

		if (family_indices.discrete_present_family())
		{
			uint32_t queue_indices[]{ family_indices.graphics_idx, family_indices.present_idx };

			info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			info.pQueueFamilyIndices = queue_indices;
			info.queueFamilyIndexCount = sizeof(queue_indices) / sizeof(*queue_indices);
		}
		else
			info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		
		vkCreateSwapchainKHR(vk_device, &info, nullptr, &vk_swapchain);

		uint32_t swapchain_image_cnt;

		check(vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &swapchain_image_cnt, nullptr));

		vk_swapchain_images.resize(swapchain_image_cnt);

		check(vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &swapchain_image_cnt, vk_swapchain_images.data()));

		vk_swapchain_format = chosen_format.format;

		vk_swapchain_extent = chosen_extent;

		return {};
	}

	err_info get_vk_swapchain_views()
	{
		vk_swapchain_views.resize(vk_swapchain_images.size());

		for (uint32_t i = 0; i != static_cast<uint32_t>(vk_swapchain_images.size()); ++i)
		{
			VkImageViewCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			create_info.image = vk_swapchain_images[i];
			create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			create_info.format = vk_swapchain_format;
			create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			create_info.subresourceRange.baseMipLevel = 0;
			create_info.subresourceRange.levelCount = 1;
			create_info.subresourceRange.baseArrayLayer = 0;
			create_info.subresourceRange.layerCount = 1;

			check(vkCreateImageView(vk_device, &create_info, nullptr, &vk_swapchain_views[i]));
		}

		return {};
	}

	err_info create_vk_render_pass()
	{
		VkAttachmentDescription color_attachment{};
		color_attachment.format = vk_swapchain_format;
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_ref{};
		color_ref.attachment = 0;
		color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_ref;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo pass_info{};
		pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		pass_info.attachmentCount = 1;
		pass_info.pAttachments = &color_attachment;
		pass_info.subpassCount = 1;
		pass_info.pSubpasses = &subpass;
		pass_info.dependencyCount = 1;
		pass_info.pDependencies = &dependency;

		check(vkCreateRenderPass(vk_device, &pass_info, nullptr, &vk_render_pass));

		return {};
	}

	err_info create_vk_graphics_pipeline()
	{
		VkShaderModule vert_shader_module;

		check(create_shader_module_from_file("vert.spv", vert_shader_module));

		VkShaderModule frag_shader_module;

		check(create_shader_module_from_file("frag.spv", frag_shader_module));

		VkPipelineShaderStageCreateInfo shader_info[2]{};
		shader_info[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_info[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shader_info[0].module = vert_shader_module;
		shader_info[0].pName = "main";

		shader_info[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_info[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shader_info[1].module = frag_shader_module;
		shader_info[1].pName = "main";
		
		VkPipelineVertexInputStateCreateInfo vert_input_info{};
		vert_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vert_input_info.vertexBindingDescriptionCount = 0;
		vert_input_info.pVertexBindingDescriptions = nullptr;
		vert_input_info.vertexAttributeDescriptionCount = 0;
		vert_input_info.pVertexAttributeDescriptions = nullptr;

		VkPipelineInputAssemblyStateCreateInfo input_asm_info{};
		input_asm_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_asm_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_asm_info.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0F;
		viewport.y = 0.0F;
		viewport.width = static_cast<float>(vk_swapchain_extent.width);
		viewport.height = static_cast<float>(vk_swapchain_extent.height);
		viewport.minDepth = 0.0F;
		viewport.maxDepth = 1.0F;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = vk_swapchain_extent;

		VkPipelineViewportStateCreateInfo view_info{};
		view_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		view_info.viewportCount = 1;
		view_info.pViewports = &viewport;
		view_info.scissorCount = 1;
		view_info.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo raster_info{};
		raster_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		raster_info.depthClampEnable = VK_FALSE;
		raster_info.rasterizerDiscardEnable = VK_FALSE;
		raster_info.polygonMode = VK_POLYGON_MODE_FILL;
		raster_info.lineWidth = 1.0F;
		raster_info.cullMode = VK_CULL_MODE_BACK_BIT;
		raster_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
		raster_info.depthBiasEnable = VK_FALSE;
		raster_info.depthBiasConstantFactor = 0.0F;
		raster_info.depthBiasClamp = 0.0F;
		raster_info.depthBiasSlopeFactor = 0.0F;

		VkPipelineMultisampleStateCreateInfo multisample_info{};
		multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_info.sampleShadingEnable = VK_FALSE;
		multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_info.minSampleShading = 1.0F;
		multisample_info.pSampleMask = nullptr;
		multisample_info.alphaToCoverageEnable = VK_FALSE;
		multisample_info.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState blend_attachment{};
		blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blend_attachment.blendEnable = VK_FALSE;
		blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo blend_info{};
		blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		blend_info.logicOpEnable = VK_FALSE;
		blend_info.logicOp = VK_LOGIC_OP_COPY;
		blend_info.attachmentCount = 1;
		blend_info.pAttachments = &blend_attachment;
		blend_info.blendConstants[0] = 0.0F;
		blend_info.blendConstants[1] = 0.0F;
		blend_info.blendConstants[2] = 0.0F;
		blend_info.blendConstants[3] = 0.0F;

		VkDynamicState dynamic_states[]{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH };
		VkPipelineDynamicStateCreateInfo dynamic_info{};
		dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_info.dynamicStateCount = sizeof(dynamic_states) / sizeof(*dynamic_states);
		dynamic_info.pDynamicStates = dynamic_states;

		VkPipelineLayoutCreateInfo layout_info{};
		layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layout_info.setLayoutCount = 0;
		layout_info.pSetLayouts = nullptr;
		layout_info.pushConstantRangeCount = 0;
		layout_info.pPushConstantRanges = nullptr;

		check(vkCreatePipelineLayout(vk_device, &layout_info, nullptr, &vk_pipeline_layout));

		VkGraphicsPipelineCreateInfo pipeline_info{};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = 2;
		pipeline_info.pStages = shader_info;
		pipeline_info.pVertexInputState = &vert_input_info;
		pipeline_info.pInputAssemblyState = &input_asm_info;
		pipeline_info.pTessellationState = nullptr;
		pipeline_info.pViewportState = &view_info;
		pipeline_info.pRasterizationState = &raster_info;
		pipeline_info.pMultisampleState = &multisample_info;
		pipeline_info.pDepthStencilState = nullptr;
		pipeline_info.pColorBlendState = &blend_info;
		pipeline_info.pDynamicState = nullptr;
		pipeline_info.layout = vk_pipeline_layout;
		pipeline_info.renderPass = vk_render_pass;
		pipeline_info.subpass = 0;
		pipeline_info.basePipelineHandle = nullptr;
		pipeline_info.basePipelineIndex = -1;

		check(vkCreateGraphicsPipelines(vk_device, nullptr, 1, &pipeline_info, nullptr, &vk_graphics_pipeline));

		vkDestroyShaderModule(vk_device, vert_shader_module, nullptr);

		vkDestroyShaderModule(vk_device, frag_shader_module, nullptr);

		return {};
	}
	
	err_info create_vk_swapchain_framebuffers()
	{
		vk_swapchain_framebuffers.resize(vk_swapchain_views.size());

		for (size_t i = 0; i != vk_swapchain_views.size(); ++i)
		{
			VkImageView attachments[]{ vk_swapchain_views[i] };

			VkFramebufferCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			create_info.renderPass = vk_render_pass;
			create_info.attachmentCount = 1;
			create_info.pAttachments = attachments;
			create_info.width = vk_swapchain_extent.width;
			create_info.height = vk_swapchain_extent.height;
			create_info.layers = 1;

			check(vkCreateFramebuffer(vk_device, &create_info, nullptr, &vk_swapchain_framebuffers[i]));


		}

		return {};
	}

	err_info create_vk_command_pool()
	{
		queue_family_indices family_indices;

		check(query_queue_families(vk_physical_device, vk_surface, family_indices));

		VkCommandPoolCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		create_info.queueFamilyIndex = family_indices.graphics_idx;
		
		check(vkCreateCommandPool(vk_device, &create_info, nullptr, &vk_command_pool));

		return {};
	}

	err_info create_vk_command_buffers()
	{
		vk_command_buffers.resize(vk_swapchain_views.size());

		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool = vk_command_pool;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = static_cast<uint32_t>(vk_command_buffers.size());

		check(vkAllocateCommandBuffers(vk_device, &alloc_info, vk_command_buffers.data()));

		for (size_t i = 0; i != vk_command_buffers.size(); ++i)
		{
			VkCommandBufferBeginInfo buffer_beg_info{};
			buffer_beg_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			buffer_beg_info.flags = 0;
			buffer_beg_info.pInheritanceInfo = nullptr;
			
			check(vkBeginCommandBuffer(vk_command_buffers[i], &buffer_beg_info));

			VkClearValue clear_color{ 0.0F, 0.0F, 0.0F, 1.0F };

			VkRenderPassBeginInfo pass_beg_info{};
			pass_beg_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			pass_beg_info.renderPass = vk_render_pass;
			pass_beg_info.framebuffer = vk_swapchain_framebuffers[i];
			pass_beg_info.renderArea.offset = { 0, 0 };
			pass_beg_info.renderArea.extent = vk_swapchain_extent;
			pass_beg_info.clearValueCount = 1;
			pass_beg_info.pClearValues = &clear_color;

			vkCmdBeginRenderPass(vk_command_buffers[i], &pass_beg_info, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(vk_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vk_graphics_pipeline);

			vkCmdDraw(vk_command_buffers[i], 3, 1, 0, 0);

			vkCmdEndRenderPass(vk_command_buffers[i]);

			check(vkEndCommandBuffer(vk_command_buffers[i]));
		}

		return {};
	}

	err_info create_vk_sync_objects()
	{
		vk_images_inflight_fences.resize(vk_swapchain_images.size(), nullptr);

		VkSemaphoreCreateInfo semaphore_info{};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_info{};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (uint32_t i = 0; i != max_frames_in_flight; ++i)
		{
			check(vkCreateSemaphore(vk_device, &semaphore_info, nullptr, &vk_image_available_semaphores[i]));

			check(vkCreateSemaphore(vk_device, &semaphore_info, nullptr, &vk_render_complete_semaphores[i]));

			check(vkCreateFence(vk_device, &fence_info, nullptr, &vk_inflight_fences[i]));
		}

		return {};
	}

	err_info main_loop()
	{
		while (!glfwWindowShouldClose(window))
		{
			check(draw_frame());

			glfwPollEvents();
		}

		check(vkDeviceWaitIdle(vk_device));

		return {};
	}

	err_info draw_frame()
	{
		check(vkWaitForFences(vk_device, 1, &vk_inflight_fences[curr_frame], VK_FALSE, UINT64_MAX));

		uint32_t image_idx;

		if (VkResult acquire_rst = vkAcquireNextImageKHR(vk_device, vk_swapchain, UINT64_MAX, vk_image_available_semaphores[curr_frame], nullptr, &image_idx); acquire_rst == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreate_swapchain();
			return {};
		}
		else if(acquire_rst != VK_SUBOPTIMAL_KHR)
			check(acquire_rst);

		if (vk_images_inflight_fences[image_idx])
			check(vkWaitForFences(vk_device, 1, &vk_images_inflight_fences[image_idx], VK_FALSE, UINT64_MAX));

		vk_images_inflight_fences[image_idx] = vk_inflight_fences[curr_frame];

		VkSemaphore wait_semaphores[]{ vk_image_available_semaphores[curr_frame] };

		VkSemaphore signal_semaphores[]{ vk_render_complete_semaphores[curr_frame] };

		VkPipelineStageFlags wait_stages[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = wait_semaphores;
		submit_info.pWaitDstStageMask = wait_stages;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &vk_command_buffers[image_idx];
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = signal_semaphores;

		check(vkResetFences(vk_device, 1, &vk_inflight_fences[curr_frame]));

		check(vkQueueSubmit(vk_graphics_queue, 1, &submit_info, vk_inflight_fences[curr_frame]));

		VkSwapchainKHR present_swapchains[]{ vk_swapchain };

		VkPresentInfoKHR present_info{};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = signal_semaphores;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = present_swapchains;
		present_info.pImageIndices = &image_idx;
		present_info.pResults = nullptr;

		if (VkResult present_rst = vkQueuePresentKHR(vk_present_queue, &present_info); present_rst == VK_ERROR_OUT_OF_DATE_KHR || present_rst == VK_SUBOPTIMAL_KHR || framebuffer_resized)
		{
			framebuffer_resized = false;
			recreate_swapchain();
		}
		else
			check(present_rst);

		curr_frame = (curr_frame + 1) % max_frames_in_flight;

		return {};
	}

	err_info recreate_swapchain()
	{
		cleanup_swapchain();

		check(vkDeviceWaitIdle(vk_device));

		check(create_vk_swapchain());

		check(get_vk_swapchain_views());

		check(create_vk_render_pass());

		check(create_vk_graphics_pipeline());

		check(create_vk_swapchain_framebuffers());

		check(create_vk_command_buffers());

		return {};
	}

	void cleanup_swapchain()
	{
		int width, height;
		
		glfwGetFramebufferSize(window, &width, &height);
		
		while (!width || !height)
		{
			glfwWaitEvents();

			glfwGetFramebufferSize(window, &width, &height);
		}

		vkDeviceWaitIdle(vk_device);

		for (auto& framebuffer : vk_swapchain_framebuffers)
			vkDestroyFramebuffer(vk_device, framebuffer, nullptr);

		vkFreeCommandBuffers(vk_device, vk_command_pool, static_cast<uint32_t>(vk_command_buffers.size()), vk_command_buffers.data());

		vkDestroyPipeline(vk_device, vk_graphics_pipeline, nullptr);

		vkDestroyPipelineLayout(vk_device, vk_pipeline_layout, nullptr);

		vkDestroyRenderPass(vk_device, vk_render_pass, nullptr);

		for (auto& view : vk_swapchain_views)
			vkDestroyImageView(vk_device, view, nullptr);

		vkDestroySwapchainKHR(vk_device, vk_swapchain, nullptr);
	}

	void cleanup()
	{
		cleanup_swapchain();

		for(auto& sem : vk_render_complete_semaphores)
			vkDestroySemaphore(vk_device, sem, nullptr);

		for (auto& sem : vk_image_available_semaphores)
			vkDestroySemaphore(vk_device, sem, nullptr);

		for (auto& fence : vk_inflight_fences)
			vkDestroyFence(vk_device, fence, nullptr);

		vkDestroyCommandPool(vk_device, vk_command_pool, nullptr);

		vkDestroyDevice(vk_device, nullptr);

		vkDestroySurfaceKHR(vk_instance, vk_surface, nullptr);

		// Destroy Vulkan debug utils messenger
		{
#ifdef OCH_VALIDATE

			auto destroy_fn = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(vk_instance, "vkDestroyDebugUtilsMessengerEXT"));

			if (destroy_fn)
				destroy_fn(vk_instance, vk_debug_messenger, nullptr);
			else
				och::print("\nERROR DURING CLEANUP: Could not load vkDestroyDebugUtilsMessengerEXT\n");

#endif // OCH_VALIDATE
		}

		vkDestroyInstance(vk_instance, nullptr);

		glfwDestroyWindow(window);

		glfwTerminate();
	}

	err_info create_shader_module_from_file(const char* filename, VkShaderModule& shader_module)
	{
		och::mapped_file<uint8_t> shader_file(filename, och::fio::access_read, och::fio::open_normal, och::fio::open_fail);

		if (!shader_file)
			return ERROR(1);

		VkShaderModuleCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.codeSize = shader_file.bytes;
		create_info.pCode = reinterpret_cast<const uint32_t*>(shader_file.get_data().beg);

		check(vkCreateShaderModule(vk_device, &create_info, nullptr, &shader_module));

		return {};
	}

	 err_info query_queue_families(VkPhysicalDevice physical_dev, VkSurfaceKHR surface, queue_family_indices& indices)
	{
		uint32_t queue_family_cnt;

		vkGetPhysicalDeviceQueueFamilyProperties(physical_dev, &queue_family_cnt, nullptr);

		std::vector<VkQueueFamilyProperties> available(queue_family_cnt);

		vkGetPhysicalDeviceQueueFamilyProperties(physical_dev, &queue_family_cnt, available.data());

		bool has_graphics_and_present_in_one = false;

		for (uint32_t i = 0; i != queue_family_cnt; ++i)
		{
			VkQueueFamilyProperties& avl = available[i];

			VkBool32 supports_present;

			check(vkGetPhysicalDeviceSurfaceSupportKHR(physical_dev, i, surface, &supports_present));

			if (avl.queueFlags & VK_QUEUE_GRAPHICS_BIT && !has_graphics_and_present_in_one)
			{
				indices.graphics_idx = i;

				if (supports_present)
				{
					indices.present_idx = i;

					has_graphics_and_present_in_one = true;
				}
			}

			if (avl.queueFlags & VK_QUEUE_COMPUTE_BIT)
				indices.compute_idx = i;

			if (avl.queueFlags & VK_QUEUE_TRANSFER_BIT)
				indices.transfer_idx = i;

			if (supports_present && !has_graphics_and_present_in_one)
				indices.present_idx = i;
		}

		return {};
	}

	 err_info query_swapchain_support(VkPhysicalDevice physical_dev, VkSurfaceKHR surface, swapchain_support_details& support)
	 {
		 check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_dev, surface, &support.capabilites));

		 uint32_t format_cnt;

		 check(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_dev, surface, &format_cnt, nullptr));

		 support.formats.resize(format_cnt);

		 check(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_dev, surface, &format_cnt, support.formats.data()));

		 uint32_t present_mode_cnt;

		 check(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_dev, surface, &present_mode_cnt, nullptr));

		 support.present_modes.resize(present_mode_cnt);

		 check(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_dev, surface, &present_mode_cnt, support.present_modes.data()));

		 return {};
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

void framebuffer_resize_callback_fn(GLFWwindow* window, int width, int height)
{
	width, height;

	reinterpret_cast<hello_vulkan*>(glfwGetWindowUserPointer(window))->framebuffer_resized = true;
}
