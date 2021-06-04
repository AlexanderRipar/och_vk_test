#pragma once

#include <cstdint>

#include <vulkan/vulkan.h>

struct err_info
{
	enum class error_type : uint32_t
	{
		custom,
		vkresult,
		hresult,
		win32,
	};

	uint64_t errcode;
	error_type errtype;
	uint32_t line_number;

	err_info(VkResult rst, uint32_t line) noexcept : errcode{ static_cast<uint64_t>(rst) }, errtype{ error_type::vkresult }, line_number{ line } {};

	err_info(uint64_t rst, uint32_t line) noexcept : errcode{ rst }, errtype{ error_type::custom }, line_number{ line } {}

	err_info() noexcept : errcode{}, errtype{ error_type::custom }, line_number{} {}

	operator bool() const noexcept { return errcode != 0; }
};

err_info check_(VkResult error, uint32_t line) noexcept { return { error, line }; }

err_info check_(err_info error, uint32_t line) noexcept { return error; line; }

#define check(macro_error_cause) { if (err_info macro_defined_result = check_((macro_error_cause), static_cast<uint32_t>(__LINE__))) return macro_defined_result; };

#define ERROR(num) err_info(static_cast<uint64_t>(num), __LINE__)
