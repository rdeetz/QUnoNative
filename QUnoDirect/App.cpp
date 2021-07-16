// QUnoDirect - App.cpp
// 2021 Roger Deetz

#include "pch.h"
#include "App.h"

using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::ApplicationModel::Activation;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics::Display;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Input;
using namespace winrt::Windows::UI::ViewManagement;
using namespace winrt::Windows::System;
using namespace DirectX;
using namespace Mooville::QUno::Direct;

IFrameworkView FrameworkViewSource::CreateView()
{
    return winrt::make<App>();
}

void App::Initialize(CoreApplicationView const& applicationView)
{
    applicationView.Activated({ this, &App::OnActivated });
    CoreApplication::Suspending({ this, &App::OnSuspending });
    CoreApplication::Resuming({ this, &App::OnResuming });

    _game = std::make_unique<Game>();

    return;
}

void App::Uninitialize() noexcept
{
    _game.reset();

    return;
}

void App::SetWindow(CoreWindow const& window)
{
    window.SizeChanged({ this, &App::OnWindowSizeChanged });

    try
    {
        window.ResizeStarted([this](auto&&, auto&&) { _sizemove = true; });
        window.ResizeCompleted([this](auto&&, auto&&) { _sizemove = false; HandleWindowSizeChanged(); });
    }
    catch (...)
    {
        // Requires Windows 10 Creators Update (10.0.15063) or later.
    }

    window.VisibilityChanged({ this, &App::OnVisibilityChanged });
    window.Closed([this](auto&&, auto&&) { _exit = true; });

    auto dispatcher = CoreWindow::GetForCurrentThread().Dispatcher();
    dispatcher.AcceleratorKeyActivated({ this, &App::OnAcceleratorKeyActivated });

    auto navigation = SystemNavigationManager::GetForCurrentView();

    // UWP on Xbox One triggers a back request whenever the B button is pressed
    // which can result in the app being suspended if unhandled.
    navigation.BackRequested([](const IInspectable&, const BackRequestedEventArgs& args) { args.Handled(true); });

    auto currentDisplayInformation = DisplayInformation::GetForCurrentView();
    currentDisplayInformation.DpiChanged({ this, &App::OnDpiChanged });
    currentDisplayInformation.OrientationChanged({ this, &App::OnOrientationChanged });
    DisplayInformation::DisplayContentsInvalidated({ this, &App::OnDisplayContentsInvalidated });

    _dpi = currentDisplayInformation.LogicalDpi();
    _logicalWidth = window.Bounds().Width;
    _logicalHeight = window.Bounds().Height;
    _nativeOrientation = currentDisplayInformation.NativeOrientation();
    _currentOrientation = currentDisplayInformation.CurrentOrientation();

    int outputWidth = ConvertDipsToPixels(_logicalWidth);
    int outputHeight = ConvertDipsToPixels(_logicalHeight);
    DXGI_MODE_ROTATION rotation = ComputeDisplayRotation();

    if (rotation == DXGI_MODE_ROTATION_ROTATE90 || rotation == DXGI_MODE_ROTATION_ROTATE270)
    {
        std::swap(outputWidth, outputHeight);
    }

    auto windowPtr = static_cast<::IUnknown*>(winrt::get_abi(window));
    _game->Initialize(windowPtr, outputWidth, outputHeight, rotation);

    return;
}

void App::Load(winrt::hstring const&) noexcept
{
    return;
}

void App::Run()
{
    while (!_exit)
    {
        if (_visible)
        {
            _game->Tick();
            CoreWindow::GetForCurrentThread().Dispatcher().ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
        }
        else
        {
            CoreWindow::GetForCurrentThread().Dispatcher().ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
        }
    }

    return;
}

void App::OnActivated(CoreApplicationView const&, IActivatedEventArgs const& args)
{
    if (args.Kind() == ActivationKind::Launch)
    {
        auto launchArgs = (const LaunchActivatedEventArgs*)(&args);

        if (launchArgs->PrelaunchActivated())
        {
            // Opt-out of prelaunch.
            CoreApplication::Exit();
            return;
        }
    }

    int w;
    int h;
    _game->GetDefaultSize(w, h);
    _dpi = DisplayInformation::GetForCurrentView().LogicalDpi();

    // Change to ApplicationViewWindowingMode::FullScreen to default to full screen.
    ApplicationView::PreferredLaunchWindowingMode(ApplicationViewWindowingMode::PreferredLaunchViewSize);
    auto desiredSize = Size(ConvertPixelsToDips(w), ConvertPixelsToDips(h));
    ApplicationView::PreferredLaunchViewSize(desiredSize);
    auto view = ApplicationView::GetForCurrentView();
    auto minSize = Size(ConvertPixelsToDips(640), ConvertPixelsToDips(480));
    view.SetPreferredMinSize(minSize);

    CoreWindow::GetForCurrentThread().Activate();

    view.FullScreenSystemOverlayMode(FullScreenSystemOverlayMode::Minimal);
    view.TryResizeView(desiredSize);

    return;
}

void App::OnSuspending(IInspectable const&, SuspendingEventArgs const& args)
{
    auto deferral = args.SuspendingOperation().GetDeferral();

    auto f = std::async(std::launch::async, [this, deferral]()
        {
            _game->OnSuspending();
            deferral.Complete();
        });

    return;
}

void App::OnResuming(IInspectable const&, IInspectable const&)
{
    _game->OnResuming();

    return;
}

void App::OnWindowSizeChanged(CoreWindow const& sender, WindowSizeChangedEventArgs const&)
{
    _logicalWidth = sender.Bounds().Width;
    _logicalHeight = sender.Bounds().Height;

    if (!_sizemove)
    {
        HandleWindowSizeChanged();
    }

    return;
}

void App::OnVisibilityChanged(CoreWindow const&, VisibilityChangedEventArgs const& args)
{
    _visible = args.Visible();

    if (_visible)
    {
        _game->OnActivated();
    }
    else
    {
        _game->OnDeactivated();
    }

    return;
}

void App::OnAcceleratorKeyActivated(CoreDispatcher const&, AcceleratorKeyEventArgs const& args)
{
    if (args.EventType() == CoreAcceleratorKeyEventType::SystemKeyDown && 
        args.VirtualKey() == VirtualKey::Enter && 
        args.KeyStatus().IsMenuKeyDown && 
        !args.KeyStatus().WasKeyDown)
    {
        // Implements the classic Alt+Enter fullscreen toggle.
        auto view = ApplicationView::GetForCurrentView();

        if (view.IsFullScreenMode())
        {
            view.ExitFullScreenMode();
        }
        else
        {
            view.TryEnterFullScreenMode();
        }

        args.Handled(true);
    }

    return;
}

void App::OnDpiChanged(DisplayInformation const& sender, IInspectable const&)
{
    _dpi = sender.LogicalDpi();
    HandleWindowSizeChanged();

    return;
}

void App::OnOrientationChanged(DisplayInformation const& sender, IInspectable const&)
{
    auto resizeManager = CoreWindowResizeManager::GetForCurrentView();
    resizeManager.ShouldWaitForLayoutCompletion(true);
    _currentOrientation = sender.CurrentOrientation();

    HandleWindowSizeChanged();

    resizeManager.NotifyLayoutCompleted();

    return;
}

void App::OnDisplayContentsInvalidated(DisplayInformation const&, IInspectable const&)
{
    _game->ValidateDevice();

    return;
}

DXGI_MODE_ROTATION App::ComputeDisplayRotation() const noexcept
{
    DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;

    switch (_nativeOrientation)
    {
        case DisplayOrientations::Landscape:
            switch (_currentOrientation)
            {
                case DisplayOrientations::Landscape:
                    rotation = DXGI_MODE_ROTATION_IDENTITY;
                    break;

                case DisplayOrientations::Portrait:
                    rotation = DXGI_MODE_ROTATION_ROTATE270;
                    break;

                case DisplayOrientations::LandscapeFlipped:
                    rotation = DXGI_MODE_ROTATION_ROTATE180;
                    break;

                case DisplayOrientations::PortraitFlipped:
                    rotation = DXGI_MODE_ROTATION_ROTATE90;
                    break;
            }

            break;

        case DisplayOrientations::Portrait:
            switch (_currentOrientation)
            {
                case DisplayOrientations::Landscape:
                    rotation = DXGI_MODE_ROTATION_ROTATE90;
                    break;

                case DisplayOrientations::Portrait:
                    rotation = DXGI_MODE_ROTATION_IDENTITY;
                    break;

                case DisplayOrientations::LandscapeFlipped:
                    rotation = DXGI_MODE_ROTATION_ROTATE270;
                    break;

                case DisplayOrientations::PortraitFlipped:
                    rotation = DXGI_MODE_ROTATION_ROTATE180;
                    break;
            }

            break;
    }

    return rotation;
}

void App::HandleWindowSizeChanged()
{
    int outputWidth = ConvertDipsToPixels(_logicalWidth);
    int outputHeight = ConvertDipsToPixels(_logicalHeight);

    DXGI_MODE_ROTATION rotation = ComputeDisplayRotation();

    if (rotation == DXGI_MODE_ROTATION_ROTATE90 || rotation == DXGI_MODE_ROTATION_ROTATE270)
    {
        std::swap(outputWidth, outputHeight);
    }

    _game->OnWindowSizeChanged(outputWidth, outputHeight, rotation);

    return;
}

int WINAPI wWinMain(_In_ HINSTANCE, _In_ HINSTANCE, _In_ LPWSTR, _In_ int)
{
    if (!XMVerifyCPUSupport())
    {
        throw std::runtime_error("XMVerifyCPUSupport");
    }

    auto frameworkViewSource = winrt::make<FrameworkViewSource>();
    CoreApplication::Run(frameworkViewSource);

    return 0;
}

void ExitGame() noexcept
{
    CoreApplication::Exit();

    return;
}
