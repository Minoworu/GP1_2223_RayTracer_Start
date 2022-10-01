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

using namespace dae;

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
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();
	float FOV =  tanf( TO_RADIANS *(camera.fovAngle / 2.f));
	const Matrix cameraToWorld{ camera.CalculateCameraToWorld() };

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			float aspectRatio{ float(m_Width) / float(m_Height) };

			Vector3 rayDirection;
			float offset{ 0.5f };
			rayDirection.x = ((2.f * (px + offset) / float(m_Width)) - 1) * aspectRatio * FOV;
			rayDirection.y = (1.f - 2.f * (py + offset) / m_Height) * FOV;
			rayDirection.z = 1;

			rayDirection = cameraToWorld.TransformVector(rayDirection);
			rayDirection.Normalize();

			Ray viewRay{ camera.origin,rayDirection };

			ColorRGB finalColor{};

			HitRecord closestHit{};
			pScene->GetClosestHit(viewRay, closestHit);

			if (closestHit.didHit)
			{
				finalColor = materials[closestHit.materialIndex]->Shade();
				closestHit.origin += closestHit.normal * 0.1f;
				for (int i = 0; i < lights.size(); i++)
				{
					Vector3 lightHit = LightUtils::GetDirectionToLight(lights[i], closestHit.origin);

					Ray shadowRay;
					shadowRay.origin = closestHit.origin;
					shadowRay.direction = lightHit;
					shadowRay.direction.Normalize();
					shadowRay.max = lightHit.Magnitude();
					if (pScene->DoesHit(shadowRay))
					{
						finalColor *= 0.5f;
					}
					
				}
			}
			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	//float dotResult{};

	//dotResult = Vector3::Dot(Vector3::UnitX, Vector3::UnitX);
	//dotResult = Vector3::Dot(Vector3::UnitX, -Vector3::UnitX);
	//dotResult = Vector3::Dot(Vector3::UnitX, Vector3::UnitY);


	//Vector3 crossResult{};
	//crossResult = Vector3::Cross(Vector3::UnitZ, Vector3::UnitX);
	//crossResult = Vector3::Cross(Vector3::UnitX, Vector3::UnitZ); 

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}
