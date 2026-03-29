// QUnoBare - App.h
// 2021 Roger Deetz

#pragma once

#include "pch.h"
#include "winrt/Mooville.QUno.Model.h"
#include <vector>

using namespace winrt;
using namespace Windows;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;
using namespace Mooville::QUno::Model;

namespace winrt::Mooville::QUno::Bare
{
    enum class HitTargetKind
    {
        None,
        DrawPile,
        HandCard,
        EndTurn,
        WildChoice,
    };

    struct HitTarget
    {
        HitTargetKind Kind { HitTargetKind::None };
        Windows::Foundation::Rect Bounds {};
        uint32_t CardIndex { 0 };
        Mooville::QUno::Model::Color ColorChoice { Mooville::QUno::Model::Color::Wild };
    };

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

        void OnActivated(IInspectable const&, IActivatedEventArgs const&);
        void OnPointerPressed(IInspectable const&, PointerEventArgs const&);
        void OnPointerMoved(IInspectable const&, PointerEventArgs const&);
        void OnPointerReleased(IInspectable const&, PointerEventArgs const&);
        void OnWindowSizeChanged(CoreWindow const&, WindowSizeChangedEventArgs const&);

        void StartGame();
        void AdvanceAITurns();
        void Render();
        void RenderBackground(float width, float height);
        void RenderDrawPile(float width, float height);
        void RenderDiscardPile(float width, float height);
        void RenderHumanHand(float width, float height);
        void RenderOpponentHand(float width, float height);
        void RenderEndTurnButton(float width, float height);
        void RenderWildPicker(float width, float height);
        void RenderCardMarkers(float x, float y, float width, float height, Mooville::QUno::Model::Card const& card, float opacity);
        SpriteVisual CreateRectVisual(float2 const& size, float3 const& offset, Windows::UI::Color const& color, float opacity);
        bool TryHandlePointer(float2 const& point);
        void PlayHumanCard(Mooville::QUno::Model::Card const& card, Mooville::QUno::Model::Color wildColor);
        void BeginHumanTurn();
        void CompleteHumanTurn();
        void UpdateWindowTitle();
        Mooville::QUno::Model::Player HumanPlayer();
        Mooville::QUno::Model::Player OpponentPlayer();
        bool IsHumanTurn() const;
        Mooville::QUno::Model::Color ChooseWildColorForPlayer(Mooville::QUno::Model::Player const& player);
        hstring CardLabel(Mooville::QUno::Model::Card const& card);
        hstring ColorLabel(Mooville::QUno::Model::Color color);
        hstring ValueLabel(Mooville::QUno::Model::Value value);
        hstring WinnerLabel();

        CoreWindow _window { nullptr };
        Compositor _compositor { nullptr };
        CompositionTarget _target { nullptr };
        ContainerVisual _root { nullptr };
        VisualCollection _visuals { nullptr };
        Mooville::QUno::Model::Game _game { nullptr };
        std::vector<HitTarget> _hitTargets;
        hstring _statusMessage;
        bool _hasDrawnThisTurn { false };
        bool _showWildPicker { false };
        Mooville::QUno::Model::Card _pendingWildCard { nullptr };
    };
};
