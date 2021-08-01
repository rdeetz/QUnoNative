// QUnoBare - App.cpp
// 2021 Roger Deetz

#include "pch.h"
#include "App.h"
#include "winrt/Mooville.QUno.Model.h"

using namespace winrt;
using namespace Windows;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;
using namespace Mooville::QUno::Bare;
using namespace Mooville::QUno::Model;

IFrameworkView FrameworkViewSource::CreateView()
{
    return make<FrameworkView>();
}

void FrameworkView::Initialize(CoreApplicationView const& applicationView)
{
    applicationView.Activated({ this, &FrameworkView::OnActivated });

    return;
}

void FrameworkView::Uninitialize()
{
    return;
}

void FrameworkView::Load(hstring const&)
{
    return;
}

void FrameworkView::Run()
{
    CoreWindow window = CoreWindow::GetForCurrentThread();
    window.Activate();

    CoreDispatcher dispatcher = window.Dispatcher();
    dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);

    return;
}

void FrameworkView::SetWindow(CoreWindow const& window)
{
    Compositor compositor;
    ContainerVisual root = compositor.CreateContainerVisual();
    _target = compositor.CreateTargetForCurrentView();
    _target.Root(root);
    _visuals = root.Children();

    window.PointerPressed({ this, &FrameworkView::OnPointerPressed });
    window.PointerMoved({ this, &FrameworkView::OnPointerMoved });
    window.PointerReleased({ this, &FrameworkView::OnPointerReleased });    

    return;
}

void FrameworkView::OnActivated(IInspectable const&, IActivatedEventArgs const&)
{
    return;
}

void FrameworkView::OnPointerPressed(IInspectable const&, PointerEventArgs const& args)
{
    Card card(Mooville::QUno::Model::Color::Red, Value::Five);

    float2 const point = args.CurrentPoint().Position();

    for (Visual visual : _visuals)
    {
        float3 const offset = visual.Offset();
        float2 const size = visual.Size();

        if ((point.x >= offset.x) &&
            (point.x < (offset.x + size.x)) &&
            (point.y >= offset.y) &&
            (point.y < (offset.y + size.y)))
        {
            _selected = visual;
            _offset.x = offset.x - point.x;
            _offset.y = offset.y - point.y;
        }
    }

    if (_selected)
    {
        _visuals.Remove(_selected);
        _visuals.InsertAtTop(_selected);
    }
    else
    {
        AddVisual(point);
    }

    return;
}

void FrameworkView::OnPointerMoved(IInspectable const&, PointerEventArgs const& args)
{
    if (_selected)
    {
        float2 const point = args.CurrentPoint().Position();

        _selected.Offset(
        {
            point.x + _offset.x,
            point.y + _offset.y,
            0.0f
        });
    }

    return;
}

void FrameworkView::OnPointerReleased(IInspectable const&, PointerEventArgs const&)
{
    _selected = nullptr;

    return;
}

void FrameworkView::AddVisual(float2 const point)
{
    Compositor compositor = _visuals.Compositor();
    SpriteVisual visual = compositor.CreateSpriteVisual();

    static Windows::UI::Color colors[] =
    {
        { 0xDC, 0x5B, 0x84, 0xB5 },
        { 0xDC, 0xED, 0x31, 0x31 },
        { 0xDC, 0x70, 0xAD, 0x47 },
        { 0xDC, 0xFF, 0xC0, 0x00 }
    };

    static unsigned last = 0;
    unsigned const next = ++last % _countof(colors);
    visual.Brush(compositor.CreateColorBrush(colors[next]));

    float const BlockSize = 100.0f;

    visual.Size(
    {
        BlockSize,
        BlockSize
    });

    visual.Offset(
    {
        point.x - BlockSize / 2.0f,
        point.y - BlockSize / 2.0f,
        0.0f,
    });

    _visuals.InsertAtTop(visual);

    _selected = visual;
    _offset.x = -BlockSize / 2.0f;
    _offset.y = -BlockSize / 2.0f;

    return;
}

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    auto frameworkViewSource = make<FrameworkViewSource>();
    CoreApplication::Run(frameworkViewSource);

    return 0;
}
