// QUnoDirect - DeviceResources.cpp
// 2021 Roger Deetz

#include "pch.h"
#include "DeviceResources.h"

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace DirectX;
using namespace DX;

#ifdef __clang__
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#pragma clang diagnostic ignored "-Wswitch-enum"
#endif

#pragma warning(disable : 4061)

DeviceResources::DeviceResources(
    DXGI_FORMAT backBufferFormat,
    DXGI_FORMAT depthBufferFormat,
    UINT backBufferCount,
    D3D_FEATURE_LEVEL minFeatureLevel,
    unsigned int flags) noexcept(false) :
    _backBufferIndex(0),
    _fenceValues{},
    _rtvDescriptorSize(0),
    _screenViewport{},
    _scissorRect{},
    _backBufferFormat(backBufferFormat),
    _depthBufferFormat(depthBufferFormat),
    _backBufferCount(backBufferCount),
    _d3dMinFeatureLevel(minFeatureLevel),
    _window(nullptr),
    _d3dFeatureLevel(D3D_FEATURE_LEVEL_11_0),
    _rotation(DXGI_MODE_ROTATION_IDENTITY),
    _dxgiFactoryFlags(0),
    _outputSize{ 0, 0, 1, 1 },
    _orientationTransform3D(DX::Rotation0),
    _colorSpace(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709),
    _options(flags),
    _deviceNotify(nullptr)
{
    if (backBufferCount < 2 || backBufferCount > MAX_BACK_BUFFER_COUNT)
    {
        throw std::out_of_range("invalid backBufferCount");
    }

    if (minFeatureLevel < D3D_FEATURE_LEVEL_11_0)
    {
        throw std::out_of_range("minFeatureLevel too low");
    }
}

DeviceResources::~DeviceResources()
{
    // Ensure that the GPU is no longer referencing resources that are about to be destroyed.
    WaitForGpu();
}

// Configures the Direct3D device, and stores handles to it and the device context.
void DeviceResources::CreateDeviceDependentResources()
{
#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug> debugController;

        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()))))
        {
            debugController->EnableDebugLayer();
        }
        else
        {
            OutputDebugStringA("WARNING: Direct3D Debug Device is not available\n");
        }

        ComPtr<IDXGIInfoQueue> dxgiInfoQueue;

        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
        {
            _dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

            DXGI_INFO_QUEUE_MESSAGE_ID hide[] =
            {
                80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
            };
            DXGI_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
            filter.DenyList.pIDList = hide;
            dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
        }
    }
#endif

    ThrowIfFailed(CreateDXGIFactory2(_dxgiFactoryFlags, IID_PPV_ARGS(_dxgiFactory.ReleaseAndGetAddressOf())));

    // Determines whether tearing support is available for fullscreen borderless windows.
    if (_options & c_AllowTearing)
    {
        BOOL allowTearing = FALSE;

        ComPtr<IDXGIFactory5> factory5;
        HRESULT hr = _dxgiFactory.As(&factory5);

        if (SUCCEEDED(hr))
        {
            hr = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
        }

        if (FAILED(hr) || !allowTearing)
        {
            _options &= ~c_AllowTearing;
#ifdef _DEBUG
            OutputDebugStringA("WARNING: Variable refresh rate displays not supported");
#endif
        }
    }

    ComPtr<IDXGIAdapter1> adapter;
    GetAdapter(adapter.GetAddressOf());

    // Create the DX12 API device object.
    ThrowIfFailed(D3D12CreateDevice(adapter.Get(), _d3dMinFeatureLevel, IID_PPV_ARGS(_d3dDevice.ReleaseAndGetAddressOf())));
    _d3dDevice->SetName(L"DeviceResources");

#ifndef NDEBUG
    // Configure debug device (if active).
    ComPtr<ID3D12InfoQueue> d3dInfoQueue;

    if (SUCCEEDED(_d3dDevice.As(&d3dInfoQueue)))
    {
#ifdef _DEBUG
        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
#endif
        D3D12_MESSAGE_ID hide[] =
        {
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
            D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE
        };
        D3D12_INFO_QUEUE_FILTER filter = {};
        filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
        filter.DenyList.pIDList = hide;
        d3dInfoQueue->AddStorageFilterEntries(&filter);
    }
#endif

    // Determine maximum supported feature level for this device
    static const D3D_FEATURE_LEVEL s_featureLevels[] =
    {
#if defined(NTDDI_WIN10_FE) && (NTDDI_VERSION >= NTDDI_WIN10_FE)
        D3D_FEATURE_LEVEL_12_2,
#endif
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    D3D12_FEATURE_DATA_FEATURE_LEVELS featLevels =
    {
        static_cast<UINT>(std::size(s_featureLevels)), s_featureLevels, D3D_FEATURE_LEVEL_11_0
    };

    HRESULT hr = _d3dDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featLevels, sizeof(featLevels));

    if (SUCCEEDED(hr))
    {
        _d3dFeatureLevel = featLevels.MaxSupportedFeatureLevel;
    }
    else
    {
        _d3dFeatureLevel = _d3dMinFeatureLevel;
    }

    // Create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(_commandQueue.ReleaseAndGetAddressOf())));
    _commandQueue->SetName(L"DeviceResources");

    // Create descriptor heaps for render target views and depth stencil views.
    D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
    rtvDescriptorHeapDesc.NumDescriptors = _backBufferCount;
    rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

    ThrowIfFailed(_d3dDevice->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(_rtvDescriptorHeap.ReleaseAndGetAddressOf())));
    _rtvDescriptorHeap->SetName(L"DeviceResources");

    _rtvDescriptorSize = _d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    if (_depthBufferFormat != DXGI_FORMAT_UNKNOWN)
    {
        D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
        dsvDescriptorHeapDesc.NumDescriptors = 1;
        dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

        ThrowIfFailed(_d3dDevice->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(_dsvDescriptorHeap.ReleaseAndGetAddressOf())));
        _dsvDescriptorHeap->SetName(L"DeviceResources");
    }

    // Create a command allocator for each back buffer that will be rendered to.
    for (UINT n = 0; n < _backBufferCount; n++)
    {
        ThrowIfFailed(_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(_commandAllocators[n].ReleaseAndGetAddressOf())));
        wchar_t name[25] = {};
        swprintf_s(name, L"Render target %u", n);
        _commandAllocators[n]->SetName(name);
    }

    // Create a command list for recording graphics commands.
    ThrowIfFailed(_d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(_commandList.ReleaseAndGetAddressOf())));
    ThrowIfFailed(_commandList->Close());
    _commandList->SetName(L"DeviceResources");

    // Create a fence for tracking GPU execution progress.
    ThrowIfFailed(_d3dDevice->CreateFence(_fenceValues[_backBufferIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_fence.ReleaseAndGetAddressOf())));
    _fenceValues[_backBufferIndex]++;
    _fence->SetName(L"DeviceResources");

    _fenceEvent.Attach(CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE));

    if (!_fenceEvent.IsValid())
    {
        throw std::system_error(std::error_code(static_cast<int>(GetLastError()), std::system_category()), "CreateEventEx");
    }

    return;
}

// These resources need to be recreated every time the window size is changed.
void DeviceResources::CreateWindowSizeDependentResources()
{
    if (!_window)
    {
        throw std::logic_error("Call SetWindow with a valid CoreWindow pointer");
    }

    // Wait until all previous GPU work is complete.
    WaitForGpu();

    // Release resources that are tied to the swap chain and update fence values.
    for (UINT n = 0; n < _backBufferCount; n++)
    {
        _renderTargets[n].Reset();
        _fenceValues[n] = _fenceValues[_backBufferIndex];
    }

    // Determine the render target size in pixels.
    const UINT backBufferWidth = std::max<UINT>(static_cast<UINT>(_outputSize.right - _outputSize.left), 1u);
    const UINT backBufferHeight = std::max<UINT>(static_cast<UINT>(_outputSize.bottom - _outputSize.top), 1u);
    const DXGI_FORMAT backBufferFormat = NoSRGB(_backBufferFormat);

    // If the swap chain already exists, resize it, otherwise create one.
    if (_swapChain)
    {
        // If the swap chain already exists, resize it.
        HRESULT hr = _swapChain->ResizeBuffers(
            _backBufferCount,
            backBufferWidth,
            backBufferHeight,
            backBufferFormat,
            (_options & c_AllowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u
        );

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
#ifdef _DEBUG
            char buff[64] = {};
            sprintf_s(buff, "Device Lost on ResizeBuffers: Reason code 0x%08X\n",
                static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? _d3dDevice->GetDeviceRemovedReason() : hr));
            OutputDebugStringA(buff);
#endif
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            HandleDeviceLost();

            // Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method
            // and correctly set up the new device.
            return;
        }
        else
        {
            ThrowIfFailed(hr);
        }
    }
    else
    {
        // Create a descriptor for the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = backBufferWidth;
        swapChainDesc.Height = backBufferHeight;
        swapChainDesc.Format = backBufferFormat;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = _backBufferCount;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.Scaling = DXGI_SCALING_ASPECT_RATIO_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        swapChainDesc.Flags = (_options & c_AllowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u;

        // Create a swap chain for the window.
        ComPtr<IDXGISwapChain1> swapChain;
        ThrowIfFailed(_dxgiFactory->CreateSwapChainForCoreWindow(
            _commandQueue.Get(),
            _window,
            &swapChainDesc,
            nullptr,
            swapChain.GetAddressOf()
        ));

        ThrowIfFailed(swapChain.As(&_swapChain));
    }

    // Handle color space settings for HDR
    UpdateColorSpace();

    // Set the proper orientation for the swap chain, and generate
    // matrix transformations for rendering to the rotated swap chain.
    switch (_rotation)
    {
        default:
        case DXGI_MODE_ROTATION_IDENTITY:
            _orientationTransform3D = DX::Rotation0;
            break;

        case DXGI_MODE_ROTATION_ROTATE90:
            _orientationTransform3D = DX::Rotation270;
            break;

        case DXGI_MODE_ROTATION_ROTATE180:
            _orientationTransform3D = DX::Rotation180;
            break;

        case DXGI_MODE_ROTATION_ROTATE270:
            _orientationTransform3D = DX::Rotation90;
            break;
    }

    ThrowIfFailed(_swapChain->SetRotation(_rotation));

    // Obtain the back buffers for this window which will be the final render targets
    // and create render target views for each of them.
    for (UINT n = 0; n < _backBufferCount; n++)
    {
        ThrowIfFailed(_swapChain->GetBuffer(n, IID_PPV_ARGS(_renderTargets[n].GetAddressOf())));
        wchar_t name[25] = {};
        swprintf_s(name, L"Render target %u", n);
        _renderTargets[n]->SetName(name);

        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = _backBufferFormat;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(
            _rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
            static_cast<INT>(n), _rtvDescriptorSize);
        _d3dDevice->CreateRenderTargetView(_renderTargets[n].Get(), &rtvDesc, rtvDescriptor);
    }

    // Reset the index to the current back buffer.
    _backBufferIndex = _swapChain->GetCurrentBackBufferIndex();

    if (_depthBufferFormat != DXGI_FORMAT_UNKNOWN)
    {
        // Allocate a 2-D surface as the depth/stencil buffer and create a depth/stencil view
        // on this surface.
        CD3DX12_HEAP_PROPERTIES depthHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

        D3D12_RESOURCE_DESC depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            _depthBufferFormat,
            backBufferWidth,
            backBufferHeight,
            1, // This depth stencil view has only one texture.
            1  // Use a single mipmap level.
        );
        depthStencilDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
        depthOptimizedClearValue.Format = _depthBufferFormat;
        depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
        depthOptimizedClearValue.DepthStencil.Stencil = 0;

        ThrowIfFailed(_d3dDevice->CreateCommittedResource(
            &depthHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &depthStencilDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &depthOptimizedClearValue,
            IID_PPV_ARGS(_depthStencil.ReleaseAndGetAddressOf())
        ));

        _depthStencil->SetName(L"Depth stencil");

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = _depthBufferFormat;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

        _d3dDevice->CreateDepthStencilView(_depthStencil.Get(), &dsvDesc, _dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
    }

    // Set the 3D rendering viewport and scissor rectangle to target the entire window.
    _screenViewport.TopLeftX = _screenViewport.TopLeftY = 0.f;
    _screenViewport.Width = static_cast<float>(backBufferWidth);
    _screenViewport.Height = static_cast<float>(backBufferHeight);
    _screenViewport.MinDepth = D3D12_MIN_DEPTH;
    _screenViewport.MaxDepth = D3D12_MAX_DEPTH;

    _scissorRect.left = _scissorRect.top = 0;
    _scissorRect.right = static_cast<LONG>(backBufferWidth);
    _scissorRect.bottom = static_cast<LONG>(backBufferHeight);

    return;
}

// This method is called when the CoreWindow is created (or re-created).
void DeviceResources::SetWindow(::IUnknown* window, int width, int height, DXGI_MODE_ROTATION rotation) noexcept
{
    _window = window;

    _outputSize.left = _outputSize.top = 0;
    _outputSize.right = width;
    _outputSize.bottom = height;

    _rotation = rotation;

    return;
}

// This method is called when the window changes size.
bool DeviceResources::WindowSizeChanged(int width, int height, DXGI_MODE_ROTATION rotation)
{
    RECT newRc;
    newRc.left = 0;
    newRc.top = 0;
    newRc.right = width;
    newRc.bottom = height;

    if (newRc.left == _outputSize.left
        && newRc.top == _outputSize.top
        && newRc.right == _outputSize.right
        && newRc.bottom == _outputSize.bottom
        && rotation == _rotation)
    {
        // Handle color space settings for HDR
        UpdateColorSpace();

        return false;
    }

    _outputSize = newRc;
    _rotation = rotation;

    CreateWindowSizeDependentResources();

    return true;
}

// This method is called in the event handler for the DisplayContentsInvalidated event.
void DeviceResources::ValidateDevice()
{
    // The D3D Device is no longer valid if the default adapter changed since the device
    // was created or if the device has been removed.

    DXGI_ADAPTER_DESC previousDesc;
    {
        ComPtr<IDXGIAdapter1> previousDefaultAdapter;
        ThrowIfFailed(_dxgiFactory->EnumAdapters1(0, previousDefaultAdapter.GetAddressOf()));
        ThrowIfFailed(previousDefaultAdapter->GetDesc(&previousDesc));
    }

    DXGI_ADAPTER_DESC currentDesc;
    {
        ComPtr<IDXGIFactory4> currentFactory;
        ThrowIfFailed(CreateDXGIFactory2(_dxgiFactoryFlags, IID_PPV_ARGS(currentFactory.GetAddressOf())));

        ComPtr<IDXGIAdapter1> currentDefaultAdapter;
        ThrowIfFailed(currentFactory->EnumAdapters1(0, currentDefaultAdapter.GetAddressOf()));
        ThrowIfFailed(currentDefaultAdapter->GetDesc(&currentDesc));
    }

    // If the adapter LUIDs don't match, or if the device reports that it has been removed,
    // a new D3D device must be created.

    if (previousDesc.AdapterLuid.LowPart != currentDesc.AdapterLuid.LowPart
        || previousDesc.AdapterLuid.HighPart != currentDesc.AdapterLuid.HighPart
        || FAILED(_d3dDevice->GetDeviceRemovedReason()))
    {
#ifdef _DEBUG
        OutputDebugStringA("Device Lost on ValidateDevice\n");
#endif

        // Create a new device and swap chain.
        HandleDeviceLost();
    }

    return;
}

// Recreate all device resources and set them back to the current state.
void DeviceResources::HandleDeviceLost()
{
    if (_deviceNotify)
    {
        _deviceNotify->OnDeviceLost();
    }

    for (UINT n = 0; n < _backBufferCount; n++)
    {
        _commandAllocators[n].Reset();
        _renderTargets[n].Reset();
    }

    _depthStencil.Reset();
    _commandQueue.Reset();
    _commandList.Reset();
    _fence.Reset();
    _rtvDescriptorHeap.Reset();
    _dsvDescriptorHeap.Reset();
    _swapChain.Reset();
    _d3dDevice.Reset();
    _dxgiFactory.Reset();

#ifdef _DEBUG
    {
        ComPtr<IDXGIDebug1> dxgiDebug;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
        {
            dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
        }
    }
#endif

    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();

    if (_deviceNotify)
    {
        _deviceNotify->OnDeviceRestored();
    }

    return;
}

// Prepare the command list and render target for rendering.
void DeviceResources::Prepare(D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
    // Reset command list and allocator.
    ThrowIfFailed(_commandAllocators[_backBufferIndex]->Reset());
    ThrowIfFailed(_commandList->Reset(_commandAllocators[_backBufferIndex].Get(), nullptr));

    if (beforeState != afterState)
    {
        // Transition the render target into the correct state to allow for drawing into it.
        D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            _renderTargets[_backBufferIndex].Get(),
            beforeState, 
            afterState);
        _commandList->ResourceBarrier(1, &barrier);
    }

    return;
}

// Present the contents of the swap chain to the screen.
void DeviceResources::Present(D3D12_RESOURCE_STATES beforeState)
{
    if (beforeState != D3D12_RESOURCE_STATE_PRESENT)
    {
        // Transition the render target to the state that allows it to be presented to the display.
        D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(_renderTargets[_backBufferIndex].Get(), beforeState, D3D12_RESOURCE_STATE_PRESENT);
        _commandList->ResourceBarrier(1, &barrier);
    }

    // Send the command list off to the GPU for processing.
    ThrowIfFailed(_commandList->Close());
    _commandQueue->ExecuteCommandLists(1, CommandListCast(_commandList.GetAddressOf()));

    HRESULT hr;

    if (_options & c_AllowTearing)
    {
        // Recommended to always use tearing if supported when using a sync interval of 0.
        hr = _swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
    }
    else
    {
        // The first argument instructs DXGI to block until VSync, putting the application
        // to sleep until the next VSync. This ensures we don't waste any cycles rendering
        // frames that will never be displayed to the screen.
        hr = _swapChain->Present(1, 0);
    }

    // If the device was reset we must completely reinitialize the renderer.
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
#ifdef _DEBUG
        char buff[64] = {};
        sprintf_s(buff, "Device Lost on Present: Reason code 0x%08X\n",
            static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? _d3dDevice->GetDeviceRemovedReason() : hr));
        OutputDebugStringA(buff);
#endif
        HandleDeviceLost();
    }
    else
    {
        ThrowIfFailed(hr);

        MoveToNextFrame();

        if (!_dxgiFactory->IsCurrent())
        {
            // Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
            ThrowIfFailed(CreateDXGIFactory2(_dxgiFactoryFlags, IID_PPV_ARGS(_dxgiFactory.ReleaseAndGetAddressOf())));
        }
    }

    return;
}

// Wait for pending GPU work to complete.
void DeviceResources::WaitForGpu() noexcept
{
    if (_commandQueue && _fence && _fenceEvent.IsValid())
    {
        // Schedule a Signal command in the GPU queue.
        UINT64 fenceValue = _fenceValues[_backBufferIndex];

        if (SUCCEEDED(_commandQueue->Signal(_fence.Get(), fenceValue)))
        {
            // Wait until the Signal has been processed.
            if (SUCCEEDED(_fence->SetEventOnCompletion(fenceValue, _fenceEvent.Get())))
            {
                WaitForSingleObjectEx(_fenceEvent.Get(), INFINITE, FALSE);

                // Increment the fence value for the current frame.
                _fenceValues[_backBufferIndex]++;
            }
        }
    }

    return;
}

// Prepare to render the next frame.
void DeviceResources::MoveToNextFrame()
{
    // Schedule a Signal command in the queue.
    const UINT64 currentFenceValue = _fenceValues[_backBufferIndex];
    ThrowIfFailed(_commandQueue->Signal(_fence.Get(), currentFenceValue));

    // Update the back buffer index.
    _backBufferIndex = _swapChain->GetCurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until it is ready.
    if (_fence->GetCompletedValue() < _fenceValues[_backBufferIndex])
    {
        ThrowIfFailed(_fence->SetEventOnCompletion(_fenceValues[_backBufferIndex], _fenceEvent.Get()));
        WaitForSingleObjectEx(_fenceEvent.Get(), INFINITE, FALSE);
    }

    // Set the fence value for the next frame.
    _fenceValues[_backBufferIndex] = currentFenceValue + 1;

    return;
}

// This method acquires the first available hardware adapter that supports Direct3D 12.
// If no such adapter can be found, try WARP. Otherwise throw an exception.
void DeviceResources::GetAdapter(IDXGIAdapter1** ppAdapter)
{
    *ppAdapter = nullptr;

    ComPtr<IDXGIAdapter1> adapter;

#if defined(__dxgi1_6_h__) && defined(NTDDI_WIN10_RS4)
    ComPtr<IDXGIFactory6> factory6;
    HRESULT hr = _dxgiFactory.As(&factory6);

    if (SUCCEEDED(hr))
    {
        for (UINT adapterIndex = 0;
            SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                adapterIndex,
                DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf())));
            adapterIndex++)
        {
            DXGI_ADAPTER_DESC1 desc;
            ThrowIfFailed(adapter->GetDesc1(&desc));

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                continue;
            }

            // Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), _d3dMinFeatureLevel, _uuidof(ID3D12Device), nullptr)))
            {
#ifdef _DEBUG
                wchar_t buff[256] = {};
                swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId, desc.DeviceId, desc.Description);
                OutputDebugStringW(buff);
#endif
                break;
            }
        }
    }
#endif
    if (!adapter)
    {
        for (UINT adapterIndex = 0;
            SUCCEEDED(_dxgiFactory->EnumAdapters1(
                adapterIndex,
                adapter.ReleaseAndGetAddressOf()));
            ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            ThrowIfFailed(adapter->GetDesc1(&desc));

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                continue;
            }

            // Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), _d3dMinFeatureLevel, _uuidof(ID3D12Device), nullptr)))
            {
#ifdef _DEBUG
                wchar_t buff[256] = {};
                swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId, desc.DeviceId, desc.Description);
                OutputDebugStringW(buff);
#endif
                break;
            }
        }
    }

#if !defined(NDEBUG)
    if (!adapter)
    {
        // Try WARP12 instead
        if (FAILED(_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()))))
        {
            throw std::runtime_error("WARP12 not available. Enable the 'Graphics Tools' optional feature");
        }

        OutputDebugStringA("Direct3D Adapter - WARP12\n");
    }
#endif

    if (!adapter)
    {
        throw std::runtime_error("No Direct3D 12 device found");
    }

    *ppAdapter = adapter.Detach();

    return;
}

// Sets the color space for the swap chain in order to handle HDR output.
void DeviceResources::UpdateColorSpace()
{
    DXGI_COLOR_SPACE_TYPE colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
    bool isDisplayHDR10 = false;

    if (_swapChain)
    {
        ComPtr<IDXGIOutput> output;
        if (SUCCEEDED(_swapChain->GetContainingOutput(output.GetAddressOf())))
        {
            ComPtr<IDXGIOutput6> output6;
            if (SUCCEEDED(output.As(&output6)))
            {
                DXGI_OUTPUT_DESC1 desc;
                ThrowIfFailed(output6->GetDesc1(&desc));

                if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)
                {
                    // Display output is HDR10.
                    isDisplayHDR10 = true;
                }
            }
        }
    }

    if ((_options & c_EnableHDR) && isDisplayHDR10)
    {
        switch (_backBufferFormat)
        {
            case DXGI_FORMAT_R10G10B10A2_UNORM:
                // The application creates the HDR10 signal.
                colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
                break;

            case DXGI_FORMAT_R16G16B16A16_FLOAT:
                // The system creates the HDR10 signal; application uses linear values.
                colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
                break;

            default:
                break;
        }
    }

    _colorSpace = colorSpace;

    UINT colorSpaceSupport = 0;

    if (SUCCEEDED(_swapChain->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport))
        && (colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT))
    {
        ThrowIfFailed(_swapChain->SetColorSpace1(colorSpace));
    }

    return;
}
