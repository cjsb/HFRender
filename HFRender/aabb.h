#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// 轴对齐包围盒
struct AABB
{
	glm::vec3 ma, mi, half;// 最大，最小，一半大小

	AABB(const glm::vec3& _ma, const glm::vec3& _mi) :
		ma(_ma), mi(_mi), half((_ma - _mi) * 0.5f)
	{}
	AABB(const AABB& other) : ma(other.ma), mi(other.mi), half(other.half)
	{}
	AABB() : ma(-FLT_MAX * 0.5f), mi(FLT_MAX * 0.5f), half(FLT_MAX * 0.5f) {}

	void Reset()
	{
		ma = glm::vec3(-FLT_MAX * 0.5f);
		mi = glm::vec3(FLT_MAX * 0.5f);
		half = glm::vec3(FLT_MAX * 0.5f);
	}

	void Set(const glm::vec3& _center, const glm::vec3& _half)
	{
		ma = _center + _half;
		mi = _center - _half;
		half = _half;
	}

	glm::vec3 GetCenter() const
	{
		return glm::vec3(mi + half);
	}

	glm::vec3 GetPoint(int i) const
	{
		return glm::vec3((i & 1) ? mi.x : ma.x, (i & 2) ? mi.y : ma.y, (i & 4) ? mi.z : ma.z);
	}

	const AABB& operator = (const AABB& other)
	{
		ma = other.ma; mi = other.mi; half = other.half;
		return *this;
	}

	AABB& operator *= (const glm::mat4& mat)
	{
		glm::vec3 points[] =
		{
			glm::vec3(ma.x, ma.y, mi.z), glm::vec3(mi.x, ma.y, mi.z), glm::vec3(mi.x, ma.y, ma.z),
			mi, glm::vec3(ma.x, mi.y, mi.z), glm::vec3(ma.x, mi.y, ma.z), glm::vec3(mi.x, mi.y, ma.z)
		};

		ma = mat * glm::vec4(ma, 1.0f);
		mi = ma;

		for (int i = 0; i < 7; ++i)
		{
			points[i] = mat * glm::vec4(points[i], 1.0f);
			mi = glm::min(mi, points[i]);
			ma = glm::max(ma, points[i]);
		}
		half = (ma - mi) * 0.5f;
		return *this;
	}

	AABB& operator *= (const glm::vec3& scale)
	{
		glm::vec3 center = ma - half;
		half.x *= scale.x;
		half.y *= scale.y;
		half.z *= scale.z;
		ma = center + half;
		mi = center - half;
		return *this;
	}

	void Merge(const AABB& other)
	{
		ma = glm::max(ma, other.ma);
		mi = glm::min(mi, other.mi);
		half = (ma - mi) * 0.5f;
	}

	void Merge(const glm::vec3& point)
	{
		ma = glm::max(ma, point);
		mi = glm::min(mi, point);
		half = (ma - mi) * 0.5f;
	}

	// 相交有2种情况，1.全包围, 2.部分相交
	bool Intersect(const AABB& other) const// 是否相交
	{
		bool result = glm::any(glm::lessThan(ma, other.mi)) || glm::any(glm::greaterThan(mi, other.ma));
		return !result;
	}

	bool Contain(const AABB& other) const // 是否全包围
	{
		bool result = glm::all(glm::greaterThan(ma, other.ma)) && glm::all(glm::lessThan(mi, other.mi));
		return result;
	}

	bool Contain(const glm::vec3& point) const
	{
		bool result = glm::all(glm::greaterThan(ma, point)) && glm::all(glm::lessThan(mi, point));
		return result;
	}

	void GetSizeCenter(glm::vec3& half_size, glm::vec3& center)
	{
		half_size = half;
		center = ma - half;
	}

	const glm::vec3& GetSize() const
	{
		return half;
	}
};