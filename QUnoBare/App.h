// QUnoBare - App.h
// 2021 Roger Deetz

#pragma once

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
    struct FrameworkViewSource : implements<FrameworkViewSource, IFrameworkViewSource>
    {
        // IFrameworkViewSource
        IFrameworkView CreateView();
    };

    struct FrameworkView : implements<FrameworkView, IFrameworkView>
    {
        // IFrameworkView
        void Initialize(CoreApplicationView const&);
        void Uninitialize();
        void Load(hstring const&);
        void Run();
        void SetWindow(CoreWindow const&);

        void OnPointerPressed(IInspectable const&, PointerEventArgs const&);
        void OnPointerMoved(IInspectable const&, PointerEventArgs const&);
        void OnPointerReleased(IInspectable const&, PointerEventArgs const&);
        void AddVisual(float2 const);

        CompositionTarget _target { nullptr };
        VisualCollection _visuals { nullptr };
        Visual _selected { nullptr };
        float2 _offset {};
    };
};
