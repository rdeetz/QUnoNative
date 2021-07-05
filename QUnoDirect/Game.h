// QUnoDirect - Game.h
// 2021 Roger Deetz

#pragma once

#include "Common/DeviceResources.h"
#include "Common/StepTimer.h"
//#include "Content/Sample3DSceneRenderer.h"

// A basic game implementation that creates a D3D12 device and
// provides a game loop.
namespace Mooville::QUno::Direct
{
    class Game final : public DX::IDeviceNotify
    {
    public:

        Game() noexcept(false);
        ~Game() = default;

        Game(Game&&) = default;
        Game& operator= (Game&&) = default;

        Game(Game const&) = delete;
        Game& operator= (Game const&) = delete;

        // Initialization and management
        void Initialize(::IUnknown* window, int width, int height, DXGI_MODE_ROTATION rotation);

        void CreateRenderers(const std::shared_ptr<DX::DeviceResources>& deviceResources);

        // Basic game loop
        void Tick();

        // IDeviceNotify
        void OnDeviceLost() override;
        void OnDeviceRestored() override;

        // Messages
        void OnActivated();
        void OnDeactivated();
        void OnSuspending();
        void OnResuming();
        void OnWindowSizeChanged(int width, int height, DXGI_MODE_ROTATION rotation);
        void ValidateDevice();

        // Properties
        void GetDefaultSize(int& width, int& height) const noexcept;

    private:

        void Update(DX::StepTimer const& timer);
        void Render();

        void Clear();

        void CreateDeviceDependentResources();
        void CreateWindowSizeDependentResources();

        // Device resources.
        std::unique_ptr<DX::DeviceResources>    m_deviceResources;

        // TODO: Replace with your own content renderers.
        //std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;

        // Rendering loop timer.
        DX::StepTimer                           m_timer;
    };
}
