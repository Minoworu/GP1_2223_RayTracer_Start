#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle) :
			origin{ _origin },
			fovAngle{ _fovAngle }
		{
		}


		Vector3 origin{};
		float fovAngle{ 90.f };

		Vector3 forward{ 0.266f,-0.453f,0.860f };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };
		Vector3 direction{};
		Vector3 transformedVector{};
		const float speed{ 10.f };
		const float rotationSpeed{ 5.f * TO_RADIANS };
		float totalPitch{ 0.f };
		float totalYaw{ 0.f };
		Matrix cameraToWorld{};


		Matrix CalculateCameraToWorld()
		{
			//todo: W2
			right = Vector3::Cross(Vector3::UnitY, forward);
			right = right.Normalized();
			up = Vector3::Cross(forward, right);
			up = up.Normalized();

			cameraToWorld = Matrix{ right,up,forward,origin };

			return cameraToWorld ;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			// FORWARD / BACKWARD INPUT 
			origin += forward * speed * pKeyboardState[SDL_SCANCODE_W] * deltaTime;
			origin += forward * -speed * pKeyboardState[SDL_SCANCODE_S] * deltaTime;

			// SIDEWAY INPUT
			origin += right * speed * pKeyboardState[SDL_SCANCODE_D] * deltaTime;
			origin += right * -speed * pKeyboardState[SDL_SCANCODE_A] * deltaTime;

			// VERTICAL INPUT
			if (mouseState == (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))
			{
				origin += up * speed * float(mouseX) * 0.05f;
				origin += up * -speed * float(mouseY) * 0.05f;

			}
			// ROTATION
			//https://stackoverflow.com/questions/71030102/how-to-detect-if-left-mousebutton-is-being-held-down-with-sdl2
			if (mouseState == SDL_BUTTON_LMASK)
			{
				totalPitch += -mouseY * (rotationSpeed) * 0.05f;
				totalYaw += mouseX * (rotationSpeed) * 0.05f;
			}
		



			Matrix totalRot = Matrix::CreateRotation(totalPitch, totalYaw, 0.f);

			forward = totalRot.TransformVector(Vector3::UnitZ);
			forward.Normalize();

			//todo: W2
			//assert(false && "Not Implemented Yet");
		}
	};
}
