#pragma once
#include <cassert>
#include "Math.h"

namespace dae
{
	namespace BRDF
	{
		/**
		 * \param kd Diffuse Reflection Coefficient
		 * \param cd Diffuse Color
		 * \return Lambert Diffuse Color
		 */
		static ColorRGB Lambert(float kd, const ColorRGB& cd)
		{
			//todo: W3
			ColorRGB rho = cd * kd;
			ColorRGB final = rho / float(M_PI);
			return {final};
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			//todo: W3
			ColorRGB rho = cd * kd;
			ColorRGB final=  rho / float(M_PI);
			return {final};
		}

		/**
		 * \brief todo
		 * \param ks Specular Reflection Coefficient
		 * \param exp Phong Exponent
		 * \param l Incoming (incident) Light Direction
		 * \param v View Direction
		 * \param n Normal of the Surface
		 * \return Phong Specular Color
		 */
		static ColorRGB Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
		{
			//todo: W3
			// invert light and view 
			Vector3 r = l.Reflect(l,n);
			r.Normalize();
			float alpha = Vector3::Dot(r, v);
			if (alpha > 0 )
			{
				float specularReflection = ks * (powf(alpha, exp));
				return { specularReflection,specularReflection,specularReflection };
			}
			return ColorRGB{};
		}

		/**
		 * \brief BRDF Fresnel Function >> Schlick
		 * \param h Normalized Halfvector between View and Light directions
		 * \param v Normalized View direction
		 * \param f0 Base reflectivity of a surface based on IOR (Indices Of Refrection), this is different for Dielectrics (Non-Metal) and Conductors (Metal)
		 * \return
		 */
		static ColorRGB FresnelFunction_Schlick(const Vector3& h, const Vector3& v, const ColorRGB& f0)
		{
			//todo: W3

			ColorRGB fresnel = f0 + (ColorRGB{ 1,1,1 } - f0) * powf((1 - Vector3::Dot(h,v)), 5);
			return {fresnel};
		}

		/**
		 * \brief BRDF NormalDistribution >> Trowbridge-Reitz GGX (UE4 implemetation - squared(roughness))
		 * \param n Surface normal
		 * \param h Normalized half vector
		 * \param roughness Roughness of the material
		 * \return BRDF Normal Distribution Term using Trowbridge-Reitz GGX
		 */
		static float NormalDistribution_GGX(const Vector3& n, const Vector3& h, float roughness)
		{
			//todo: W3
			float alpha = roughness * roughness;
			float alphaSq = alpha * alpha;
			float dot = Vector3::Dot(n, h);
			float dotSq = dot * dot;
			float normalDist = alphaSq / (float(M_PI) *(dotSq * (alphaSq - 1) + 1) * (dotSq * (alphaSq - 1) + 1));
			return {normalDist};
		}


		/**
		 * \brief BRDF Geometry Function >> Schlick GGX (Direct Lighting + UE4 implementation - squared(roughness))
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using SchlickGGX
		 */
		static float GeometryFunction_SchlickGGX(const Vector3& n, const Vector3& v, float roughness)
		{
			//todo: W3
			float dot = Vector3::Dot(n, v);
			float k = roughness * roughness;
			float schlick = dot / (dot * (1 - k) + k);
			return {schlick};
		}

		/**
		 * \brief BRDF Geometry Function >> Smith (Direct Lighting)
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param l Normalized light direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using Smith (> SchlickGGX(n,v,roughness) * SchlickGGX(n,l,roughness))
		 */
		static float GeometryFunction_Smith(const Vector3& n, const Vector3& v, const Vector3& l, float roughness)
		{
			//todo: W3
			float k = roughness * roughness;
			float smith = GeometryFunction_SchlickGGX(n, v, k) * GeometryFunction_SchlickGGX(n, l, k);
			return {smith};
		}

	}
}