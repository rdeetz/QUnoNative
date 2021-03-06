﻿// QUnoDirect - QUnoDirectMain.h
// 2021 Roger Deetz

#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"

// Renders Direct3D content on the screen.
namespace Mooville
{
	namespace QUno
	{
		namespace Direct
		{
			class QUnoDirectMain
			{
			public:
				QUnoDirectMain();
				void CreateRenderers(const std::shared_ptr<DX::DeviceResources>& deviceResources);
				void Update();
				bool Render();

				void OnWindowSizeChanged();
				void OnSuspending();
				void OnResuming();
				void OnDeviceRemoved();

			private:
				// TODO: Replace with your own content renderers.
				std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;

				// Rendering loop timer.
				DX::StepTimer m_timer;
			};
		};
	};
};
