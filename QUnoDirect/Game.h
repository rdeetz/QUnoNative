// QUnoDirect - Game.h
// 2021 Roger Deetz

#pragma once

#include "pch.h"
#include "Common/DX.h"
#include "Common/StepTimer.h"
#include "Common/DeviceResources.h"
#include "Content/Sample3DSceneRenderer.h"

using namespace DX;

namespace Mooville::QUno::Direct
{
    class Game final : public IDeviceNotify
    {
    public:
        Game() noexcept(false);
        ~Game() = default;
        Game(Game&&) = default;
        Game& operator= (Game&&) = default;
        Game(Game const&) = delete;
        Game& operator= (Game const&) = delete;

        void Initialize(::IUnknown* window, int width, int height, DXGI_MODE_ROTATION rotation);
        void Tick();

        // IDeviceNotify
        void OnDeviceLost() override;
        void OnDeviceRestored() override;

        void OnActivated();
        void OnDeactivated();
        void OnSuspending();
        void OnResuming();
        void OnWindowSizeChanged(int width, int height, DXGI_MODE_ROTATION rotation);
        void ValidateDevice();
        void GetDefaultSize(int& width, int& height) const noexcept;

    private:
        void Update(StepTimer const& timer);
        void Render();
        void Clear();
        void CreateDeviceDependentResources();
        void CreateWindowSizeDependentResources();
        void CreateRenderer(const std::shared_ptr<DeviceResources>& deviceResources);

        StepTimer _timer;
        std::unique_ptr<DeviceResources> _deviceResources;
        std::unique_ptr<Sample3DSceneRenderer> _sceneRenderer;
    };
}
