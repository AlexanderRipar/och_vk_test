#pragma once

#include <cstdint>

#include "och_range.h"

#include <vulkan/vulkan.h>

namespace och
{
	struct error_context
	{
		const char* file;
		const char* function;
		const char* call;
		uint32_t line_num;
	};

	struct err_info
	{
		bool is_bad = false;

		err_info(VkResult rst, const error_context& ctx) noexcept;

		err_info(uint64_t rst, const error_context& ctx) noexcept;

		err_info(err_info rst, const error_context& ctx) noexcept;

		err_info() noexcept = default;

		operator bool() const noexcept;
	};

	och::range<const error_context*> get_errors() noexcept;

#define check(macro_error_cause) {static constexpr och::error_context ctx{__FILE__, __FUNCTION__, #macro_error_cause, __LINE__}; if(och::err_info macro_result = och::err_info(macro_error_cause, ctx)) return macro_result; }

#define ERROR(num) och::err_info(static_cast<uint64_t>(num), och::error_context{__FILE__, __FUNCTION__, #num, __LINE__})
}
