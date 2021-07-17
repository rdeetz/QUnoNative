// QUnoDirect - DeviceResources.h
// 2021 Roger Deetz

#pragma once

#include "pch.h"
#include "DX.h"

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace DirectX;

namespace DX
{
    class DeviceResources
    {
    public:
        static const unsigned int c_AllowTearing = 0x1;
        static const unsigned int c_EnableHDR = 0x2;

        DeviceResources(
            DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM,
            DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT,
            UINT backBufferCount = 2,
            D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_11_0,
            unsigned int flags = 0) noexcept(false);
        ~DeviceResources();
        DeviceResources(DeviceResources&&) = default;
        DeviceResources& operator= (DeviceResources&&) = default;
        DeviceResources(DeviceResources const&) = delete;
        DeviceResources& operator= (DeviceResources const&) = delete;

        void CreateDeviceDependentResources();
        void CreateWindowSizeDependentResources();
        void SetWindow(::IUnknown* window, int width, int height, DXGI_MODE_ROTATION rotation) noexcept;
        bool WindowSizeChanged(int width, int height, DXGI_MODE_ROTATION rotation);
        void ValidateDevice();
        void HandleDeviceLost();
        void RegisterDeviceNotify(IDeviceNotify* deviceNotify) noexcept { _deviceNotify = deviceNotify; return; }
        void Prepare(
            D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_PRESENT, 
            D3D12_RESOURCE_STATES afterState = D3D12_RESOURCE_STATE_RENDER_TARGET);
        void Present(D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_RENDER_TARGET);
        void WaitForGpu() noexcept;

        RECT GetOutputSize() const noexcept { return _outputSize; }
        DXGI_MODE_ROTATION GetRotation() const noexcept { return _rotation; }
        auto GetD3DDevice() const noexcept { return _d3dDevice.Get(); }
        auto GetSwapChain() const noexcept { return _swapChain.Get(); }
        auto GetDXGIFactory() const noexcept { return _dxgiFactory.Get(); }
        D3D_FEATURE_LEVEL GetDeviceFeatureLevel() const noexcept { return _d3dFeatureLevel; }
        ID3D12Resource* GetRenderTarget() const noexcept { return _renderTargets[_backBufferIndex].Get(); }
        ID3D12Resource* GetDepthStencil() const noexcept { return _depthStencil.Get(); }
        ID3D12CommandQueue* GetCommandQueue() const noexcept { return _commandQueue.Get(); }
        ID3D12CommandAllocator* GetCommandAllocator() const noexcept { return _commandAllocators[_backBufferIndex].Get(); }
        auto GetCommandList() const noexcept { return _commandList.Get(); }
        DXGI_FORMAT GetBackBufferFormat() const noexcept { return _backBufferFormat; }
        DXGI_FORMAT GetDepthBufferFormat() const noexcept { return _depthBufferFormat; }
        D3D12_VIEWPORT GetScreenViewport() const noexcept { return _screenViewport; }
        D3D12_RECT GetScissorRect() const noexcept { return _scissorRect; }
        UINT GetCurrentFrameIndex() const noexcept { return _backBufferIndex; }
        UINT GetBackBufferCount() const noexcept { return _backBufferCount; }
        XMFLOAT4X4 GetOrientationTransform3D() const noexcept { return _orientationTransform3D; }
        DXGI_COLOR_SPACE_TYPE GetColorSpace() const noexcept { return _colorSpace; }
        unsigned int GetDeviceOptions() const noexcept { return _options; }

        CD3DX12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const noexcept
        {
            return CD3DX12_CPU_DESCRIPTOR_HANDLE(
                _rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                static_cast<INT>(_backBufferIndex),
                _rtvDescriptorSize);
        }

        CD3DX12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const noexcept
        {
            return CD3DX12_CPU_DESCRIPTOR_HANDLE(_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
        }

    private:
        static const size_t MAX_BACK_BUFFER_COUNT = 3;

        void MoveToNextFrame();
        void GetAdapter(IDXGIAdapter1** ppAdapter);
        void UpdateColorSpace();

        ComPtr<ID3D12Device> _d3dDevice;
        ComPtr<ID3D12GraphicsCommandList> _commandList;
        ComPtr<ID3D12CommandQueue> _commandQueue;
        ComPtr<ID3D12CommandAllocator> _commandAllocators[MAX_BACK_BUFFER_COUNT];
        ComPtr<IDXGIFactory4> _dxgiFactory;
        ComPtr<IDXGISwapChain3> _swapChain;
        ComPtr<ID3D12Resource> _renderTargets[MAX_BACK_BUFFER_COUNT];
        ComPtr<ID3D12Resource> _depthStencil;
        ComPtr<ID3D12Fence> _fence;
        UINT64 _fenceValues[MAX_BACK_BUFFER_COUNT];
        Event _fenceEvent;
        ComPtr<ID3D12DescriptorHeap> _rtvDescriptorHeap;
        ComPtr<ID3D12DescriptorHeap> _dsvDescriptorHeap;
        UINT _rtvDescriptorSize;
        D3D12_VIEWPORT _screenViewport;
        D3D12_RECT _scissorRect;
        DXGI_FORMAT _backBufferFormat;
        DXGI_FORMAT _depthBufferFormat;
        UINT _backBufferIndex;
        UINT _backBufferCount;
        D3D_FEATURE_LEVEL _d3dMinFeatureLevel;
        ::IUnknown* _window;
        D3D_FEATURE_LEVEL _d3dFeatureLevel;
        DXGI_MODE_ROTATION _rotation;
        DWORD _dxgiFactoryFlags;
        RECT _outputSize;
        DXGI_COLOR_SPACE_TYPE _colorSpace;
        unsigned int _options;
        XMFLOAT4X4 _orientationTransform3D;
        IDeviceNotify* _deviceNotify;
    };
}
