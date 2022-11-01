//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"
#include <future>
#include <iostream>
#include <thread>
#include <ppl.h>



using namespace dae;

//#define ASYNC

#define PARALLEL_FOR

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	camera.CalculateCameraToWorld();
	
	auto& materials = pScene->GetMaterials();

	float aspectRatio = m_Width / (float)m_Height;
	auto& lights = pScene->GetLights();



	const uint32_t numPixels = m_Width * m_Height;

	//float dotResult{};

	//dotResult = Vector3::Dot(Vector3::UnitX, Vector3::UnitX);
	//dotResult = Vector3::Dot(Vector3::UnitX, -Vector3::UnitX);
	//dotResult = Vector3::Dot(Vector3::UnitX, Vector3::UnitY);


	//Vector3 crossResult{};
	//crossResult = Vector3::Cross(Vector3::UnitZ, Vector3::UnitX);
	//crossResult = Vector3::Cross(Vector3::UnitX, Vector3::UnitZ); 

	//@END
	//Update SDL Surface
#if defined(ASYNC)
//ASYNC
	const uint32_t numCores = std::thread::hardware_concurrency();
	std::vector<std::future<void>> async_futures{};

	const uint32_t numPixelsPerTask = numPixels / numCores; //Int division can skip pixels
	uint32_t numUnassignedPixels = numPixels % numCores; //Rest of division
	uint32_t currentPixelIndex = 0;

	//Create task
	for (uint32_t index{ 0 }; index < numCores; ++index)
	{
		uint32_t taskSize = numPixelsPerTask;
		if (numUnassignedPixels > 0)
		{
			++taskSize;
			--numUnassignedPixels;
		}

		async_futures.push_back(
			std::async(std::launch::async, [=, this]
				{
					const uint32_t pixelIndexEnd = currentPixelIndex + taskSize;
					for (uint32_t pixelIndex = currentPixelIndex; pixelIndex < pixelIndexEnd; ++pixelIndex)
					{
						RenderPixel(pScene, pixelIndex, fov, aspectRatio, camera, lights, materials);
					}
				})
		);

		currentPixelIndex += taskSize;
	}

	//Wait for all task
	for (const std::future<void>& f : async_futures)
	{
		f.wait();
	}

#elif defined(PARALLEL_FOR)
	//PARALLEL
	concurrency::parallel_for(0u, numPixels, [=, this](int i)
		{
			RenderPixel(pScene, i, aspectRatio, camera, lights, materials);
		});
#else
	//SYNCHRONOUS
	for (uint32_t index = 0; index < numPixels; index++)
	{
		RenderPixel(pScene, index,  aspectRatio, camera, lights, materials);
	}
#endif
	SDL_UpdateWindowSurface(m_pWindow);
}

void dae::Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float aspectRatio, const Camera& camera, const std::vector<Light>& lights, const std::vector<Material*>& materials) const
{
	const int px = pixelIndex % m_Width;
	const int py = pixelIndex / m_Width;

	/*const Matrix cameraToWorld = camera.cameraToWorld;*/

	Vector3 rayDirection;
	float offset{ 0.5f };
	rayDirection.x = ((2.f * (px + offset) / float(m_Width)) - 1) * aspectRatio * camera.tangent;
	rayDirection.y = (1.f - 2.f * (py + offset) / m_Height) * camera.tangent;
	rayDirection.z = 1;

	rayDirection = camera.cameraToWorld.TransformVector(rayDirection);
	rayDirection.Normalize();

	Ray viewRay{ camera.origin,rayDirection };

	ColorRGB finalColor{};

	HitRecord closestHit{};
	pScene->GetClosestHit(viewRay, closestHit);

	if (closestHit.didHit)
	{
		// Not Shadowed
		/*finalColor = materials[closestHit.materialIndex]->Shade();*/
		closestHit.origin += closestHit.normal * 0.001f;
		Ray shadowRay;


		// Shadowed

		for (int i = 0; i < lights.size(); i++)
		{
			Vector3 lightHit = LightUtils::GetDirectionToLight(lights[i], closestHit.origin);
			float lightNormalize = lightHit.Normalize();

			if (m_ShadowsEnabled)
			{
			
				shadowRay.origin = closestHit.origin;
				shadowRay.direction = lightHit;
				shadowRay.max = lightNormalize;
				if (pScene->DoesHit(shadowRay))
				{
					/*shadowFactor *= 0.95f;*/
					continue;
				}
			}
			switch (m_CurrentLightingMode)
			{
			case dae::Renderer::LightingMode::OberservedArea:
			{
				float observedArea = std::max(Vector3::Dot(closestHit.normal, lightHit), 0.f);
				finalColor += ColorRGB{ observedArea,observedArea,observedArea };

			}
			break;

			case dae::Renderer::LightingMode::Radiance:
			{
				finalColor += LightUtils::GetRadiance(lights[i], closestHit.origin);
			}
			break;
			case dae::Renderer::LightingMode::BRDF:
			{

				finalColor += materials[closestHit.materialIndex]->Shade(closestHit, lightHit, rayDirection);
			}
			break;
			case dae::Renderer::LightingMode::Combined:
			{
				float observedArea = std::max(Vector3::Dot(closestHit.normal, lightHit), 0.f);
				finalColor += LightUtils::GetRadiance(lights[i], closestHit.origin) * observedArea * materials[closestHit.materialIndex]->Shade(closestHit, lightHit, rayDirection);


			}
			break;
			}

		}

	}

	//Update Color in Buffer
	finalColor.MaxToOne();
	// Apply shadowing to final color 
	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));

}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void dae::Renderer::ToggleShadows()
{
	m_ShadowsEnabled = !m_ShadowsEnabled;
}

void dae::Renderer::CycleLightingMode()
{
	m_CurrentLightingMode = static_cast<LightingMode>((static_cast<int>(m_CurrentLightingMode) + 1) % 4);
	std::cout << "Current light mode is " << std::to_string(int(m_CurrentLightingMode)) << '\n';
}
