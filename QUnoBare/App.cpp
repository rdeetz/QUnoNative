// QUnoBare - App.cpp
// 2021 Roger Deetz

#include "pch.h"
#include "winrt/Mooville.QUno.Model.h"

using namespace winrt;
using namespace Windows;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;
using namespace Mooville::QUno::Model;

namespace winrt::Mooville::QUno::Bare
{
    struct FrameworkView : implements<FrameworkView, IFrameworkView>
    {
        CompositionTarget _target{ nullptr };
        VisualCollection _visuals{ nullptr };
        Visual _selected{ nullptr };
        float2 _offset{};

        void Initialize(CoreApplicationView const&)
        {
            return;
        }

        void Uninitialize()
        {
            return;
        }

        void Load(hstring const&)
        {
            return;
        }

        void Run()
        {
            CoreWindow window = CoreWindow::GetForCurrentThread();
            window.Activate();

            CoreDispatcher dispatcher = window.Dispatcher();
            dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);

            return;
        }

        void SetWindow(CoreWindow const& window)
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

        void OnPointerPressed(IInspectable const&, PointerEventArgs const& args)
        {
            Card card(Mooville::QUno::Model::Color::Red, Value::Five);

            float2 const point = args.CurrentPoint().Position();

            for (Visual visual : _visuals)
            {
                float3 const offset = visual.Offset();
                float2 const size = visual.Size();

                if ((point.x >= offset.x) &&
                    (point.x < offset.x + size.x) &&
                    (point.y >= offset.y) &&
                    (point.y < offset.y) + size.y)
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

        void OnPointerMoved(IInspectable const&, PointerEventArgs const& args)
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

        void OnPointerReleased(IInspectable const&, PointerEventArgs const& args)
        {
            _selected = nullptr;

            return;
        }

        void AddVisual(float2 const point)
        {
            Compositor compositor = _visuals.Compositor();
            SpriteVisual visual = compositor.CreateSpriteVisual();

            static Windows::UI::Color colors[] =
            {
                { 0xDC, 0x5B, 0x9B, 0xD5 },
                { 0xDC, 0xED, 0x7D, 0x31 },
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
    };

    struct FrameworkViewSource : implements<FrameworkViewSource, IFrameworkViewSource>
    {
        IFrameworkView CreateView()
        {
            return make<FrameworkView>();
        }
    };
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    CoreApplication::Run(make<::Mooville::QUno::Bare::FrameworkViewSource>());

    return 0;
}
