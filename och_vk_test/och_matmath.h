#pragma once

#include <cmath>

namespace och
{
	struct mat2
	{
		float f[4];
	};

	struct vec2
	{
		float f[2];
	};

	struct mat3
	{
		float f[9];
	};

	struct vec3
	{
		float f[3];

		vec3() = default;

		constexpr vec3(float all) : f{ all, all, all } {}

		constexpr vec3(float x, float y, float z) noexcept : f{ x, y, z } {}

		float& operator[](size_t i) noexcept { return f[i]; }

		float operator[](size_t i) const noexcept { return f[i]; }
	};

	struct mat4
	{
		float f[16];

		mat4() = default;

		mat4(float all) : f{ all, all, all, all, all, all, all, all, all, all, all, all, all, all, all, all } {}

		mat4(float xx, float xy, float xz, float xw, float yx, float yy, float yz, float yw, float zx, float zy, float zz, float zw, float wx, float wy, float wz, float ww)
			: f{ xx, xy, xz, xw, yx, yy, yz, yw, zx, zy, zz, zw, wx, wy, wz, ww } {}

		static mat4 identity() noexcept
		{
			return
			{
				1.0F, 0.0F, 0.0F, 0.0F,
				0.0F, 1.0F, 0.0F, 0.0F,
				0.0F, 0.0F, 1.0F, 0.0F,
				0.0F, 0.0F, 0.0F, 1.0F
			};
		}

		static mat4 scale(float sx, float sy, float sz) noexcept
		{
			return
			{
				  sx, 0.0F, 0.0F, 0.0F,
				0.0F,   sy, 0.0F, 0.0F,
				0.0F, 0.0F,   sz, 0.0F,
				0.0F, 0.0F, 0.0F, 1.0F
			};
		}

		static mat4 translate(float tx, float ty, float tz) noexcept
		{
			return
			{
				1.0F, 0.0F, 0.0F,   tx,
				0.0F, 1.0F, 0.0F,   ty,
				0.0F, 0.0F, 1.0F,   tz,
				0.0F, 0.0F, 0.0F, 1.0F
			};
		}

		static mat4 translate(const vec3& t)
		{
			return translate(t.f[0], t.f[1], t.f[2]);
		}

		static mat4 rotate_x(float angle) noexcept
		{
			float cv = cosf(angle);

			float sv = sinf(angle);

			return
			{
				1.0F, 0.0F, 0.0F, 0.0F,
				0.0F,   cv,  -sv, 0.0F,
				0.0F,   sv,   cv, 0.0F,
				0.0F, 0.0F, 0.0F, 1.0F
			};
		}

		static mat4 rotate_y(float angle) noexcept
		{
			float cv = cosf(angle);

			float sv = sinf(angle);

			return
			{
				  cv, 0.0F,   sv, 0.0F,
				0.0F, 1.0F, 0.0F, 0.0F,
				 -sv, 0.0F,   cv, 0.0F,
				0.0F, 0.0F, 0.0F, 1.0F
			};
		}

		static mat4 rotate_z(float angle) noexcept
		{
			float cv = cosf(angle);

			float sv = sinf(angle);

			return
			{
				  cv,  -sv, 0.0F, 0.0F,
				  sv,   cv, 0.0F, 0.0F,
				0.0F, 0.0F, 1.0F, 0.0F,
				0.0F, 0.0F, 0.0F, 1.0F
			};
		}

		static mat4 mirror_yz() noexcept
		{
			return
			{
				-1.0F, 0.0F, 0.0F, 0.0F,
				 0.0F, 1.0F, 0.0F, 0.0F,
				 0.0F, 0.0F, 1.0F, 0.0F,
				 0.0F, 0.0F, 0.0F, 1.0F
			};
		}

		static mat4 mirror_xz() noexcept
		{
			return
			{
				1.0F,  0.0F, 0.0F, 0.0F,
				0.0F, -1.0F, 0.0F, 0.0F,
				0.0F,  0.0F, 1.0F, 0.0F,
				0.0F,  0.0F, 0.0F, 1.0F
			};
		}

		static mat4 mirror_xy() noexcept
		{
			return
			{
				1.0F, 0.0F,  0.0F, 0.0F,
				0.0F, 1.0F,  0.0F, 0.0F,
				0.0F, 0.0F, -1.0F, 0.0F,
				0.0F, 0.0F,  0.0F, 1.0F
			};
		}
	};

	struct vec4
	{
		float f[4];

		vec4() = default;

		constexpr vec4(float all) : f{ all, all, all, all } {}

		constexpr vec4(float x, float y, float z, float w) noexcept : f{ x, y, z, w } {}

		float& operator[](size_t i) noexcept { return f[i]; }

		float operator[](size_t i) const noexcept { return f[i]; }
	};

	mat4 operator*(const mat4& l, const mat4& r)
	{
		mat4 product;

		product.f[0] = l.f[0] * r.f[0] + l.f[1] * r.f[4] + l.f[2] * r.f[8] + l.f[3] * r.f[12];
		product.f[1] = l.f[0] * r.f[1] + l.f[1] * r.f[5] + l.f[2] * r.f[9] + l.f[3] * r.f[13];
		product.f[2] = l.f[0] * r.f[2] + l.f[1] * r.f[6] + l.f[2] * r.f[10] + l.f[3] * r.f[14];
		product.f[3] = l.f[0] * r.f[3] + l.f[1] * r.f[7] + l.f[2] * r.f[11] + l.f[3] * r.f[15];

		product.f[4] = l.f[4] * r.f[0] + l.f[5] * r.f[4] + l.f[6] * r.f[8] + l.f[7] * r.f[12];
		product.f[5] = l.f[4] * r.f[1] + l.f[5] * r.f[5] + l.f[6] * r.f[9] + l.f[7] * r.f[13];
		product.f[6] = l.f[4] * r.f[2] + l.f[5] * r.f[6] + l.f[6] * r.f[10] + l.f[7] * r.f[14];
		product.f[7] = l.f[4] * r.f[3] + l.f[5] * r.f[7] + l.f[6] * r.f[11] + l.f[7] * r.f[15];

		product.f[8] = l.f[8] * r.f[0] + l.f[9] * r.f[4] + l.f[10] * r.f[8] + l.f[11] * r.f[12];
		product.f[9] = l.f[8] * r.f[1] + l.f[9] * r.f[5] + l.f[10] * r.f[9] + l.f[11] * r.f[13];
		product.f[10] = l.f[8] * r.f[2] + l.f[9] * r.f[6] + l.f[10] * r.f[10] + l.f[11] * r.f[14];
		product.f[11] = l.f[8] * r.f[3] + l.f[9] * r.f[7] + l.f[10] * r.f[11] + l.f[11] * r.f[15];

		product.f[12] = l.f[12] * r.f[0] + l.f[13] * r.f[4] + l.f[14] * r.f[8] + l.f[15] * r.f[12];
		product.f[13] = l.f[12] * r.f[1] + l.f[13] * r.f[5] + l.f[14] * r.f[9] + l.f[15] * r.f[13];
		product.f[14] = l.f[12] * r.f[2] + l.f[13] * r.f[6] + l.f[14] * r.f[10] + l.f[15] * r.f[14];
		product.f[15] = l.f[12] * r.f[3] + l.f[13] * r.f[7] + l.f[14] * r.f[11] + l.f[15] * r.f[15];

		return product;
	}

	vec4 operator*(const mat4& l, const vec4& r)
	{
		vec4 product;

		product.f[0] = l.f[0] * r.f[0] + l.f[1] * r.f[1] + l.f[2] * r.f[2] + l.f[3] * r.f[3];

		product.f[1] = l.f[4] * r.f[0] + l.f[5] * r.f[1] + l.f[6] * r.f[2] + l.f[7] * r.f[3];

		product.f[2] = l.f[8] * r.f[0] + l.f[9] * r.f[1] + l.f[10] * r.f[2] + l.f[11] * r.f[3];

		product.f[3] = l.f[12] * r.f[0] + l.f[13] * r.f[1] + l.f[14] * r.f[2] + l.f[15] * r.f[3];

		return product;
	}



	vec4 operator+(const vec4& l, const vec4& r) noexcept
	{
		return
		{
			l.f[0] + r.f[0],
			l.f[1] + r.f[1],
			l.f[2] + r.f[2],
			l.f[3] + r.f[3]
		};
	}

	vec4 operator-(const vec4& l, const vec4& r) noexcept
	{
		return
		{
			l.f[0] - r.f[0],
			l.f[1] - r.f[1],
			l.f[2] - r.f[2],
			l.f[3] - r.f[3]
		};
	}

	vec4 operator-(const vec4& v)
	{
		return
		{
			-v.f[0],
			-v.f[1],
			-v.f[2],
			-v.f[3]
		};
	}

	vec4& operator+=(vec4& l, vec4& r) noexcept
	{
		l.f[0] += r.f[0];
		l.f[1] += r.f[1];
		l.f[2] += r.f[2];
		l.f[3] += r.f[3];

		return l;
	}

	vec4& operator-=(vec4& l, vec4& r) noexcept
	{
		l.f[0] -= r.f[0];
		l.f[1] -= r.f[1];
		l.f[2] -= r.f[2];
		l.f[3] -= r.f[3];

		return l;
	}

	float dot(const vec4& l, const vec4& r) noexcept
	{
		return
			l.f[0] * r.f[0] +
			l.f[1] * r.f[1] +
			l.f[2] * r.f[2] +
			l.f[3] * r.f[3];
	}

	float magnitude(const vec4& v) noexcept
	{
		return sqrtf(v.f[0] * v.f[0] + v.f[1] * v.f[1] + v.f[2] * v.f[2] + v.f[3] * v.f[3]);
	}

	vec4 normalize(const vec4& v) noexcept
	{
		float inv_magnitude = 1.0F / magnitude(v);

		return
		{
			v.f[0] * inv_magnitude,
			v.f[1] * inv_magnitude,
			v.f[2] * inv_magnitude,
			v.f[3] * inv_magnitude
		};
	}



	vec3 operator+(const vec3& l, const vec3& r) noexcept
	{
		return
		{
			l.f[0] + r.f[0],
			l.f[1] + r.f[1],
			l.f[2] + r.f[2]
		};
	}

	vec3 operator-(const vec3& l, const vec3& r) noexcept
	{
		return
		{
			l.f[0] - r.f[0],
			l.f[1] - r.f[1],
			l.f[2] - r.f[2]
		};
	}

	vec3 operator-(const vec3& v)
	{
		return
		{
			-v.f[0],
			-v.f[1],
			-v.f[2]
		};
	}

	vec3& operator+=(vec3& l, vec3& r) noexcept
	{
		l.f[0] += r.f[0];
		l.f[1] += r.f[1];
		l.f[2] += r.f[2];

		return l;
	}

	vec3& operator-=(vec3& l, vec3& r) noexcept
	{
		l.f[0] -= r.f[0];
		l.f[1] -= r.f[1];
		l.f[2] -= r.f[2];

		return l;
	}

	float dot(const vec3& l, const vec3& r) noexcept
	{
		return
			l.f[0] * r.f[0] +
			l.f[1] * r.f[1] +
			l.f[2] * r.f[2];
	}

	float magnitude(const vec3& v) noexcept
	{
		return sqrtf(v.f[0] * v.f[0] + v.f[1] * v.f[1] + v.f[2] * v.f[2]);
	}

	vec3 normalize(const vec3& v) noexcept
	{
		float inv_magnitude = 1.0F / magnitude(v);

		return
		{
			v.f[0] * inv_magnitude,
			v.f[1] * inv_magnitude,
			v.f[2] * inv_magnitude
		};
	}

	vec3 cross(const vec3& l, const vec3& r)
	{
		return
		{
			l.f[1] * r.f[2] - l.f[2] * r.f[1],
			l.f[2] * r.f[0] - l.f[0] * r.f[2],
			l.f[0] * r.f[1] - l.f[1] * r.f[0]
		};
	}






	mat4 perpective(float vert_fov, float aspect_ratio, float n, float f) noexcept
	{
		float tan_fov = tanf(vert_fov);

		return
		{
			1.0F / (aspect_ratio * tan_fov),             0.0F,                0.0F,                      0.0F,
									   0.0F,  -1.0F / tan_fov,                0.0F,                      0.0F,
									   0.0F,             0.0F,  (-n - f) / (n - f),  (2.0F * f * n) / (n - f),
									   0.0F,             0.0F,                1.0F,                      0.0F
		};
	}

	mat4 look_at(vec3 eye_pos, vec3 center_pos, vec3 up)
	{
		vec3 f = normalize(center_pos - eye_pos);

		vec3 u = normalize(up);

		vec3 s = normalize(cross(f, u));

		mat4 m
		{
			 s[0],  s[1],  s[2], 0.0F,
			 u[0],  u[1],  u[2], 0.0F,
			-f[0], -f[1], -f[2], 0.0F,
			 0.0F,  0.0F,  0.0F, 1.0F
		};

		return m * mat4::translate(-eye_pos);
	}
}
