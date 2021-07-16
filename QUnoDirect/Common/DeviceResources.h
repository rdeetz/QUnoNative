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

/*
inline IAsyncOperation<std::vector<byte>> ReadDataAsync(const winrt::param::hstring& filename)
{
    auto folder = Package::Current().InstalledLocation();
    auto file = co_await folder.GetFileAsync(filename);
    auto fileBuffer = co_await FileIO::ReadBufferAsync(file);
    std::vector<byte> returnBuffer;
    returnBuffer.resize(fileBuffer.Length());
    DataReader::FromBuffer(fileBuffer).ReadBytes(returnBuffer.data());
    //auto dataReader = DataReader::FromBuffer(fileBuffer);
    //dataReader.ReadBytes(returnBuffer.data());

    co_return returnBuffer;

    return create_task(
            // get a file via IAsync given a file name from the installed location folder
            folder->GetFileAsync(Platform::StringReference(filename.c_str()))
        ).then([](StorageFile^ file)
        {
            // This returns an IAsync that returns a buffer
            return FileIO::ReadBufferAsync(file);
        }).then([](Streams::IBuffer^ fileBuffer) -> std::vector<byte>
            {
                // This takes a file buffer, reads the bytes and copies them to an array which is then returned
                std::vector<byte> returnBuffer;
                returnBuffer.resize(fileBuffer->Length);
                Streams::DataReader::FromBuffer(fileBuffer)->ReadBytes(Platform::ArrayReference<byte>(returnBuffer.data(), fileBuffer->Length));
                return returnBuffer;
            });
}
*/

/*
namespace DX
{
    // Controls all the DirectX device resources.
    class DeviceResources
    {
    public:
        DeviceResources(DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT);
        void SetWindow(Windows::UI::Core::CoreWindow^ window);
        void SetLogicalSize(Windows::Foundation::Size logicalSize);
        void SetCurrentOrientation(Windows::Graphics::Display::DisplayOrientations currentOrientation);
        void SetDpi(float dpi);
        void ValidateDevice();
        void Present();
        void WaitForGpu();

        // The size of the render target, in pixels.
        Windows::Foundation::Size	GetOutputSize() const				{ return m_outputSize; }

        // The size of the render target, in dips.
        Windows::Foundation::Size	GetLogicalSize() const				{ return m_logicalSize; }

        float						GetDpi() const						{ return m_effectiveDpi; }
        bool						IsDeviceRemoved() const				{ return m_deviceRemoved; }

        // D3D Accessors.
        ID3D12Device*				GetD3DDevice() const				{ return m_d3dDevice.Get(); }
        IDXGISwapChain3*			GetSwapChain() const				{ return m_swapChain.Get(); }
        ID3D12Resource*				GetRenderTarget() const				{ return m_renderTargets[m_currentFrame].Get(); }
        ID3D12Resource*				GetDepthStencil() const				{ return m_depthStencil.Get(); }
        ID3D12CommandQueue*			GetCommandQueue() const				{ return m_commandQueue.Get(); }
        ID3D12CommandAllocator*		GetCommandAllocator() const			{ return m_commandAllocators[m_currentFrame].Get(); }
        DXGI_FORMAT					GetBackBufferFormat() const			{ return m_backBufferFormat; }
        DXGI_FORMAT					GetDepthBufferFormat() const		{ return m_depthBufferFormat; }
        D3D12_VIEWPORT				GetScreenViewport() const			{ return m_screenViewport; }
        DirectX::XMFLOAT4X4			GetOrientationTransform3D() const	{ return m_orientationTransform3D; }
        UINT						GetCurrentFrameIndex() const		{ return m_currentFrame; }

        CD3DX12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const
        {
            return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_currentFrame, m_rtvDescriptorSize);
        }
        CD3DX12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const
        {
            return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
        }

    private:
        void CreateDeviceIndependentResources();
        void CreateDeviceResources();
        void CreateWindowSizeDependentResources();
        void UpdateRenderTargetSize();
        void MoveToNextFrame();
        DXGI_MODE_ROTATION ComputeDisplayRotation();
        void GetHardwareAdapter(IDXGIAdapter1** ppAdapter);

        UINT											m_currentFrame;

        // Direct3D objects.
        Microsoft::WRL::ComPtr<ID3D12Device>			m_d3dDevice;
        Microsoft::WRL::ComPtr<IDXGIFactory4>			m_dxgiFactory;
        Microsoft::WRL::ComPtr<IDXGISwapChain3>			m_swapChain;
        Microsoft::WRL::ComPtr<ID3D12Resource>			m_renderTargets[c_frameCount];
        Microsoft::WRL::ComPtr<ID3D12Resource>			m_depthStencil;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_rtvHeap;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_dsvHeap;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue>		m_commandQueue;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator>	m_commandAllocators[c_frameCount];
        DXGI_FORMAT										m_backBufferFormat;
        DXGI_FORMAT										m_depthBufferFormat;
        D3D12_VIEWPORT									m_screenViewport;
        UINT											m_rtvDescriptorSize;
        bool											m_deviceRemoved;

        // CPU/GPU Synchronization.
        Microsoft::WRL::ComPtr<ID3D12Fence>				m_fence;
        UINT64											m_fenceValues[c_frameCount];
        HANDLE											m_fenceEvent;

        // Cached reference to the Window.
        Platform::Agile<Windows::UI::Core::CoreWindow>	m_window;

        // Cached device properties.
        Windows::Foundation::Size						m_d3dRenderTargetSize;
        Windows::Foundation::Size						m_outputSize;
        Windows::Foundation::Size						m_logicalSize;
        Windows::Graphics::Display::DisplayOrientations	m_nativeOrientation;
        Windows::Graphics::Display::DisplayOrientations	m_currentOrientation;
        float											m_dpi;

        // This is the DPI that will be reported back to the app. It takes into account whether the app supports high resolution screens or not.
        float											m_effectiveDpi;

        // Transforms used for display orientation.
        DirectX::XMFLOAT4X4								m_orientationTransform3D;
    };
}
*/
