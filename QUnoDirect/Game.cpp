// QUnoDirect - Game.cpp
// 2021 Roger Deetz

#include "pch.h"
#include "Game.h"

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace DX;
using namespace Mooville::QUno::Direct;

Game::Game() noexcept(false)
{
    _deviceResources = std::make_unique<DeviceResources>();
    _deviceResources->RegisterDeviceNotify(this);
}

void Game::Initialize(::IUnknown* window, int width, int height, DXGI_MODE_ROTATION rotation)
{
    _deviceResources->SetWindow(window, width, height, rotation);

    _deviceResources->CreateDeviceDependentResources();
    CreateDeviceDependentResources();

    _deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // TODO Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    _timer.SetFixedTimeStep(true);
    _timer.SetTargetElapsedSeconds(1.0 / 60);
    */

    return;
}

void Game::Tick()
{
    _timer.Tick([&]()
        {
            Update(_timer);
        });

    Render();

    return;
}

void Game::OnDeviceLost()
{
    // TODO Add Direct3D resource cleanup here.
    return;
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();

    return;
}

void Game::OnActivated()
{
    // TODO Game is becoming the active window.
    return;
}

void Game::OnDeactivated()
{
    // TODO Game is becoming a background window.
    return;
}

void Game::OnSuspending()
{
    // TODO Game is being power-suspended.
    return;
}

void Game::OnResuming()
{
    _timer.ResetElapsedTime();

    // TODO Game is being power-resumed.
    return;
}

void Game::OnWindowSizeChanged(int width, int height, DXGI_MODE_ROTATION rotation)
{
    if (_deviceResources->WindowSizeChanged(width, height, rotation))
    {
        // TODO Game window is being resized.
        CreateWindowSizeDependentResources();
    }
    
    return;
}

void Game::ValidateDevice()
{
    _deviceResources->ValidateDevice();

    return;
}

void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // The minimum size is 320 x 200.
    width = 1024;
    height = 768;

    return;
}

void Game::Update(StepTimer const& timer)
{
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

    float elapsedTime = float(timer.GetElapsedSeconds());

    // TODO Add your game logic here.
    elapsedTime;

    PIXEndEvent();

    return;
}

void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (_timer.GetFrameCount() == 0)
    {
        return;
    }

    // Prepare the command list to render a new frame.
    _deviceResources->Prepare();
    Clear();

    auto commandList = _deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

    // TODO Add your rendering code here.

    PIXEndEvent(commandList);

    // Show the new frame.
    PIXBeginEvent(_deviceResources->GetCommandQueue(), PIX_COLOR_DEFAULT, L"Present");
    _deviceResources->Present();
    PIXEndEvent(_deviceResources->GetCommandQueue());

    return;
}

void Game::Clear()
{
    auto commandList = _deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Clear");

    // Clear the views.
    auto rtvDescriptor = _deviceResources->GetRenderTargetView();
    auto dsvDescriptor = _deviceResources->GetDepthStencilView();

    commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
    commandList->ClearRenderTargetView(rtvDescriptor, Colors::CornflowerBlue, 0, nullptr);
    commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Set the viewport and scissor rect.
    auto viewport = _deviceResources->GetScreenViewport();
    auto scissorRect = _deviceResources->GetScissorRect();
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

    PIXEndEvent(commandList);

    return;
}

void Game::CreateDeviceDependentResources()
{
    auto device = _deviceResources->GetD3DDevice();

    // Check Shader Model 6 support.
    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_0 };

    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)))
        || (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_0))
    {
#ifdef _DEBUG
        OutputDebugStringA("ERROR: Shader Model 6.0 is not supported!\n");
#endif
        throw std::runtime_error("Shader Model 6.0 is not supported!");
    }

    // TODO Initialize device dependent objects here (independent of window size).

    return;
}

void Game::CreateWindowSizeDependentResources()
{
    // TODO Initialize windows-size dependent objects here.
    //      Allocate all memory resources that change on a window SizeChanged event.
    return;
}

/*
void Game::CreateRenderers(const std::shared_ptr<DX::DeviceResources>& deviceResources)
{
    // TODO: Replace this with your app's content initialization.
    m_sceneRenderer = std::unique_ptr<Sample3DSceneRenderer>(new Sample3DSceneRenderer(deviceResources));

    OnWindowSizeChanged();
}
*/
