#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			// ray sphere intersection 2d

			const Vector3 tc = sphere.origin - ray.origin;
			float dp = Vector3::Dot(tc, ray.direction);
			const Vector3 tp = dp * ray.direction - ray.origin;
			float odSquare = tc.SqrMagnitude() - (dp * dp);

			if (odSquare > (sphere.radius * sphere.radius) )
			{
				return false;
			}
			const float tca = sqrtf((sphere.radius * sphere.radius) - odSquare);
			const float t0 = dp - tca;
			if (t0 <= ray.min || t0 > ray.max)
			{
				return false;
			}
			if (ignoreHitRecord == true)
			{
				return true;
			}
			 Vector3 l1 = ray.origin + t0 * ray.direction;
			 hitRecord.t = t0;
			 hitRecord.didHit = true;
			 hitRecord.origin = l1;
			 hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
			 hitRecord.materialIndex = sphere.materialIndex;
			 return true;


			//--------------------------------------------------------------------------------------------------------------------


			//float A = (Vector3::Dot(ray.direction, ray.direction));
			//float B = (Vector3::Dot((2 * ray.direction), ray.origin - sphere.origin));
			//float C = (Vector3::Dot((ray.origin - sphere.origin), (ray.origin - sphere.origin)) - (sphere.radius * sphere.radius));
			//float D = ((B * B) - (4 * A * C));
			//// D == 0 means 1 hit 
			//if (AreEqual(D, 0.f))
			//{
			//	hitRecord.t = -B / (2 * A);
			//	if (ray.min < hitRecord.t && hitRecord.t < ray.max)
			//	{
			//		if (ignoreHitRecord == true)
			//		{
			//			return true;
			//		}
			//		hitRecord.didHit = true;
			//		hitRecord.origin = ray.origin + (ray.direction * hitRecord.t);
			//		hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
			//		hitRecord.materialIndex = sphere.materialIndex;
			//		return true;
			//	}


			//}
			//else if (D > 0.f) // check if hit 
			//{
			//	D = sqrt(D);
			//	float t1, t2;
			//	t1 = (-B + D) / (2 * A);
			//	t2 = (-B - D) / (2 * A);

			//	if (t1 < 0.f && t2 < 0.f)
			//	{
			//		return false;
			//	}
			//	if ((t1 > t2 || t1 < 0) && t2 > 0)
			//	{
			//		hitRecord.t = t2; // take closest hit
			//	}
			//	else
			//	{
			//		hitRecord.t = t1;
			//	}
			//	if (ray.min < hitRecord.t && hitRecord.t < ray.max)
			//	{
			//		if (ignoreHitRecord == true)
			//		{
			//			return true;
			//		}
			//		hitRecord.didHit = true;
			//		hitRecord.origin = ray.origin + (ray.direction * hitRecord.t);
			//		hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
			//		hitRecord.materialIndex = sphere.materialIndex;
			//		return true;
			//	}
			//}
			//return false;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W1
			float temp = Vector3::Dot(plane.origin - ray.origin, plane.normal) / Vector3::Dot(ray.direction, plane.normal);
			if (ray.min <= temp && temp <= ray.max)
			{
				if (ignoreHitRecord == true)
				{
					return true;
				}
				hitRecord.t = temp;
				hitRecord.origin = ray.origin + (ray.direction * temp);
				hitRecord.normal = plane.normal;
				hitRecord.materialIndex = plane.materialIndex;
				hitRecord.didHit = true;

			}
			return hitRecord.didHit;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			Vector3 edge1 = triangle.v1 - triangle.v0;
			Vector3 edge2 = triangle.v2 - triangle.v0;
			Vector3 normal = Vector3::Cross(edge1, edge2);
			float viewDot = Vector3::Dot(normal, ray.direction);
			if (-FLT_EPSILON < viewDot && viewDot < FLT_EPSILON)
			{
				return false;
			}

			TriangleCullMode currentCullMode = triangle.cullMode;

			// Shadows have reversed culling mode
			if (ignoreHitRecord == true)
			{
				switch (currentCullMode)
				{
				case dae::TriangleCullMode::FrontFaceCulling:
					currentCullMode = TriangleCullMode::BackFaceCulling;
					break;
				case dae::TriangleCullMode::BackFaceCulling:
					currentCullMode = TriangleCullMode::FrontFaceCulling;
					break;
				case dae::TriangleCullMode::NoCulling:
					break;
				default:
					break;
				}
			}

			switch (currentCullMode)
			case dae::TriangleCullMode::FrontFaceCulling:
			{
				if (viewDot < 0)
				{
					return false;
				}

				break;
			case dae::TriangleCullMode::BackFaceCulling:

				if (viewDot > 0)
				{
					return false;
				}
				break;
			case dae::TriangleCullMode::NoCulling:
				break;

			}
			//https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
			const Vector3 h = Vector3::Cross(ray.direction, edge2);
			float x = Vector3::Dot(edge1, h);
			if (x > -FLT_EPSILON && x < FLT_EPSILON)
			{
				return false;
			}
			const float f = 1.0f / x;
			const Vector3 s = ray.origin - triangle.v0;
			const float u = f * Vector3::Dot(s, h);
			if (u < 0.f || u > 1.f)
			{
				return false;
			}
			const Vector3 q = Vector3::Cross(s, edge1);
			const float v = f * Vector3::Dot(ray.direction, q);
			if (v < 0.f || v + u > 1.f)
			{
				return false;
			}
			const float t = f * Vector3::Dot(edge2, q);
			if (t < ray.min || t >= ray.max)
			{
				return false;
			}
			if (ignoreHitRecord == true)
			{
				return true;
			}
			Vector3 intersectPoint = ray.origin + ray.direction * t;
			hitRecord.t = t;
			hitRecord.origin = intersectPoint;
			hitRecord.normal = triangle.normal;
			hitRecord.materialIndex = triangle.materialIndex;
			hitRecord.didHit = true;
			return true;
			// --------------------------------------------------------------------------------------------
			// --------------------------------------------------------------------------------------------
			// --------------------------------------------------------------------------------------------
			// --------------------------------------------------------------------------------------------
			// --------------------------------------------------------------------------------------------
			// --------------------------------------------------------------------------------------------
			// --------------------------------------------------------------------------------------------


			//Vector3 center = (triangle.v0 + triangle.v1 + triangle.v2) / 3.f;
			//Vector3 L = center - ray.origin;
			//float t = Vector3::Dot(L, normal) / Vector3::Dot(ray.direction, normal);
			//if (ray.min >= t || t >= ray.max)
			//{
			//	return false;
			//}
			//Vector3 p = ray.origin + t * ray.direction;
			//Vector3 pointToSide = p - triangle.v0;
			//Vector3 edgeA = a;
			//if (Vector3::Dot(normal, Vector3::Cross(edgeA, pointToSide)) < 0)
			//{
			//	return false;
			//}
			//Vector3 edgeB = triangle.v2 - triangle.v1;
			//pointToSide = p - triangle.v1;
			//if (Vector3::Dot(normal, Vector3::Cross(edgeB, pointToSide)) < 0)
			//{
			//	return false;
			//}
			//Vector3 edgeC = triangle.v0 - triangle.v2;
			//pointToSide = p - triangle.v2;
			//if (Vector3::Dot(normal, Vector3::Cross(edgeC, pointToSide)) < 0)
			//{
			//	return false;
			//}
			//if (ignoreHitRecord)
			//{
			//	return true;
			//}
			//hitRecord.didHit = true;
			//hitRecord.origin = p;
			//hitRecord.normal = triangle.normal;
			//hitRecord.materialIndex = triangle.materialIndex;
			//hitRecord.t = t;
			//return true;

		}
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool SlabTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			float tx1 = (mesh.transformedMinAABB.x - ray.origin.x) / ray.direction.x;
			float tx2 = (mesh.transformedMaxAABB.x - ray.origin.x) / ray.direction.x;

			float tmin = std::min(tx1, tx2);
			float tmax = std::max(tx1, tx2);

			float ty1 = (mesh.transformedMinAABB.y - ray.origin.y) / ray.direction.y;
			float ty2 = (mesh.transformedMaxAABB.y - ray.origin.y) / ray.direction.y;

			tmin = std::max(tmin, std::min(ty1, ty2));
			tmax = std::min(tmin, std::max(ty1, ty2));

			float tz1 = (mesh.transformedMinAABB.z - ray.origin.z) / ray.direction.z;
			float tz2 = (mesh.transformedMaxAABB.z - ray.origin.z) / ray.direction.z;

			tmin = std::max(tmin, std::min(tz1, tz2));
			tmax = std::min(tmin, std::max(tz1, tz2));

			return tmax > 0 && tmax >= tmin;
		}
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{

			//todo W5
			// slab test
			if (!SlabTest_TriangleMesh(mesh, ray))
			{
				return false;
			}
			HitRecord currentHit;
			Triangle t;
			for (size_t i = 0; i < mesh.indices.size(); i += 3)
			{
				t = Triangle{ mesh.transformedPositions[mesh.indices[i]],mesh.transformedPositions[mesh.indices[i + 1]],mesh.transformedPositions[mesh.indices[i + 2]] };
				t.cullMode = mesh.cullMode;
				t.normal = mesh.transformedNormals[i / 3];
				t.materialIndex = mesh.materialIndex;
				if (HitTest_Triangle(t, ray, currentHit))
				{
					if (ignoreHitRecord == true)
					{
						return true;
					}
					if (currentHit.t < hitRecord.t)
					{
						hitRecord = currentHit;
					}
				}

			}
			return hitRecord.didHit;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}


#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			if (light.type == LightType::Point)
			{
				return { light.origin - origin };
			}
			else
			{
				return{};
			}

		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			ColorRGB radiance;
			//todo W3
			switch (light.type)
			{
			case LightType::Point:
				radiance = light.color * (light.intensity / (light.origin - target).SqrMagnitude());
				break;
			case LightType::Directional:
				radiance = light.color * light.intensity;
				break;
			default:
				break;
			}
			return radiance;
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof())
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if (isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}