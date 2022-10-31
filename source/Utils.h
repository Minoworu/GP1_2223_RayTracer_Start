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
			//todo W1
			float A = (Vector3::Dot(ray.direction, ray.direction));
			float B = (Vector3::Dot((2 * ray.direction), ray.origin - sphere.origin));
			float C = (Vector3::Dot((ray.origin - sphere.origin), (ray.origin - sphere.origin)) - (sphere.radius * sphere.radius));
			float D = ((B * B) - (4 * A * C));
			// D == 0 means 1 hit 
			if (AreEqual(D, 0.f))
			{
				hitRecord.t = -B / (2 * A);
				if (ray.min < hitRecord.t && hitRecord.t < ray.max)
				{
					if (ignoreHitRecord == true)
					{
						return true;
					}
					hitRecord.origin = ray.origin + (ray.direction * hitRecord.t);
					hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
					hitRecord.materialIndex = sphere.materialIndex;
					hitRecord.didHit = true;
					return true;
				}


			}
			else if (D > 0.f) // check if hit 
			{
				D = sqrt(D);
				float t1, t2;
				t1 = (-B + D) / (2 * A);
				t2 = (-B - D) / (2 * A);

				if (t1 < 0.f && t2 < 0.f)
				{
					return false;
				}
				if ((t1 > t2 || t1 < 0) && t2 > 0)
				{
					hitRecord.t = t2; // take closest hit
				}
				else
				{
					hitRecord.t = t1;
				}
				if (ray.min < hitRecord.t && hitRecord.t < ray.max)
				{
					if (ignoreHitRecord == true)
					{
						return true;
					}
					hitRecord.origin = ray.origin + (ray.direction * hitRecord.t);
					hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
					hitRecord.materialIndex = sphere.materialIndex;
					hitRecord.didHit = true;
					return true;
				}
			}
			return false;
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
				hitRecord.normal = plane.normal;
				hitRecord.origin = ray.origin + (ray.direction * temp);
				hitRecord.t = temp;
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
			Vector3 a = triangle.v1 - triangle.v0;
			Vector3 b = triangle.v2 - triangle.v0;
			Vector3 c = triangle.v2 - triangle.v1;
			Vector3 normal = Vector3::Cross(a, b);
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
			Vector3 center = (triangle.v0 + triangle.v1 + triangle.v2) / 3.f;
			Vector3 L = center - ray.origin;
			float t = Vector3::Dot(L, normal) / Vector3::Dot(ray.direction, normal);
			if (ray.min >= t || t >= ray.max)
			{
				return false;
			}
			Vector3 p = ray.origin + t * ray.direction;
			Vector3 pointToSide = p - triangle.v0;
			Vector3 edgeA = a;
			if (Vector3::Dot(normal, Vector3::Cross(edgeA, pointToSide)) < 0)
			{
				return false;
			}
			Vector3 edgeB = triangle.v2 - triangle.v1;
			pointToSide = p - triangle.v1;
			if (Vector3::Dot(normal, Vector3::Cross(edgeB, pointToSide)) < 0)
			{
				return false;
			}
			Vector3 edgeC = triangle.v0 - triangle.v2;
			pointToSide = p - triangle.v2;
			if (Vector3::Dot(normal, Vector3::Cross(edgeC, pointToSide)) < 0)
			{
				return false;
			}
			if (ignoreHitRecord)
			{
				return true;
			}
			hitRecord.materialIndex = triangle.materialIndex;
			hitRecord.normal = triangle.normal;
			hitRecord.origin = p;
			hitRecord.t = t;
			hitRecord.didHit = true;
			return true;

		}
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			HitRecord currentHit;
			for (size_t i = 0; i < mesh.indices.size(); i += 3)
			{
				Triangle t = Triangle{ mesh.transformedPositions[mesh.indices[i]],mesh.transformedPositions[mesh.indices[i + 1]],mesh.transformedPositions[mesh.indices[i + 2]] };
				t.cullMode = mesh.cullMode;
				t.materialIndex = mesh.materialIndex;
				t.normal = mesh.transformedNormals[i / 3];
				if (HitTest_Triangle(t, ray, currentHit))
				{
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