#pragma once

struct mat4
{
	float f[16];
};

struct vec4
{
	float f[4];
};

struct mat3
{
	float f[3][3];
};

struct vec3
{
	float f[3];
};

struct mat2
{
	float f[2][2];
};

struct vec2
{
	float f[2];
};

mat4 operator*(const mat4& l, const mat4& r)
{
	mat4 product;

	product.f[ 0] = l.f[ 0] * r.f[ 0] + l.f[ 1] * r.f[ 4] + l.f[ 2] * r.f[ 8] + l.f[ 3] * r.f[12];
	product.f[ 1] = l.f[ 0] * r.f[ 1] + l.f[ 1] * r.f[ 5] + l.f[ 2] * r.f[ 9] + l.f[ 3] * r.f[13];
	product.f[ 2] = l.f[ 0] * r.f[ 2] + l.f[ 1] * r.f[ 6] + l.f[ 2] * r.f[10] + l.f[ 3] * r.f[14];
	product.f[ 3] = l.f[ 0] * r.f[ 3] + l.f[ 1] * r.f[ 7] + l.f[ 2] * r.f[11] + l.f[ 3] * r.f[15];

	product.f[ 4] = l.f[ 4] * r.f[ 0] + l.f[ 5] * r.f[ 4] + l.f[ 6] * r.f[ 8] + l.f[ 7] * r.f[12];
	product.f[ 5] = l.f[ 4] * r.f[ 1] + l.f[ 5] * r.f[ 5] + l.f[ 6] * r.f[ 9] + l.f[ 7] * r.f[13];
	product.f[ 6] = l.f[ 4] * r.f[ 2] + l.f[ 5] * r.f[ 6] + l.f[ 6] * r.f[10] + l.f[ 7] * r.f[14];
	product.f[ 7] = l.f[ 4] * r.f[ 3] + l.f[ 5] * r.f[ 7] + l.f[ 6] * r.f[11] + l.f[ 7] * r.f[15];

	product.f[ 8] = l.f[ 8] * r.f[ 0] + l.f[ 9] * r.f[ 4] + l.f[10] * r.f[ 8] + l.f[11] * r.f[12];
	product.f[ 9] = l.f[ 8] * r.f[ 1] + l.f[ 9] * r.f[ 5] + l.f[10] * r.f[ 9] + l.f[11] * r.f[13];
	product.f[10] = l.f[ 8] * r.f[ 2] + l.f[ 9] * r.f[ 6] + l.f[10] * r.f[10] + l.f[11] * r.f[14];
	product.f[11] = l.f[ 8] * r.f[ 3] + l.f[ 9] * r.f[ 7] + l.f[10] * r.f[11] + l.f[11] * r.f[15];

	product.f[12] = l.f[12] * r.f[ 0] + l.f[13] * r.f[ 4] + l.f[14] * r.f[ 8] + l.f[15] * r.f[12];
	product.f[13] = l.f[12] * r.f[ 1] + l.f[13] * r.f[ 5] + l.f[14] * r.f[ 9] + l.f[15] * r.f[13];
	product.f[14] = l.f[12] * r.f[ 2] + l.f[13] * r.f[ 6] + l.f[14] * r.f[10] + l.f[15] * r.f[14];
	product.f[15] = l.f[12] * r.f[ 3] + l.f[13] * r.f[ 7] + l.f[14] * r.f[11] + l.f[15] * r.f[15];
	
	return product;
}

vec4 operator*(const mat4& l, const vec4& r)
{
	vec4 product;

	product.f[0] = l.f[ 0] * r.f[0] + l.f[ 1] * r.f[1] + l.f[ 2] * r.f[2] + l.f[ 3] * r.f[3];

	product.f[1] = l.f[ 4] * r.f[0] + l.f[ 5] * r.f[1] + l.f[ 6] * r.f[2] + l.f[ 7] * r.f[3];

	product.f[2] = l.f[ 8] * r.f[0] + l.f[ 9] * r.f[1] + l.f[10] * r.f[2] + l.f[11] * r.f[3];

	product.f[3] = l.f[12] * r.f[0] + l.f[13] * r.f[1] + l.f[14] * r.f[2] + l.f[15] * r.f[3];

	return product;
}
