// QUnoBare - App.cpp
// 2021 Roger Deetz

#include "pch.h"
#include "App.h"
#include "winrt/Mooville.QUno.Model.h"
#include <string>

using namespace winrt;
using namespace Windows;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;
using namespace Windows::UI::ViewManagement;
using namespace Mooville::QUno::Bare;
using namespace Mooville::QUno::Model;

namespace
{
    constexpr float CardWidth = 84.0f;
    constexpr float CardHeight = 126.0f;
    constexpr float CardSpacing = 18.0f;
    constexpr float PileSpacing = 110.0f;
    constexpr float HandBottomMargin = 30.0f;
    constexpr float OpponentTopMargin = 24.0f;
    constexpr float StatusButtonWidth = 120.0f;
    constexpr float StatusButtonHeight = 42.0f;
    constexpr float WildButtonSize = 64.0f;
    constexpr float WildButtonGap = 14.0f;

    bool Contains(HitTarget const& target, float2 const& point)
    {
        return (point.x >= target.Bounds.X) &&
            (point.x < (target.Bounds.X + target.Bounds.Width)) &&
            (point.y >= target.Bounds.Y) &&
            (point.y < (target.Bounds.Y + target.Bounds.Height));
    }

    Windows::UI::Color CardColor(Mooville::QUno::Model::Color color)
    {
        switch (color)
        {
        case Mooville::QUno::Model::Color::Red:
            return { 0xFF, 0xC8, 0x39, 0x39 };
        case Mooville::QUno::Model::Color::Blue:
            return { 0xFF, 0x37, 0x72, 0xC3 };
        case Mooville::QUno::Model::Color::Yellow:
            return { 0xFF, 0xEA, 0xBC, 0x2F };
        case Mooville::QUno::Model::Color::Green:
            return { 0xFF, 0x4B, 0x9B, 0x55 };
        default:
            return { 0xFF, 0x38, 0x38, 0x38 };
        }
    }

    Windows::UI::Color Lightened(Windows::UI::Color color, uint8_t amount)
    {
        auto lighten = [amount](uint8_t component)
        {
            uint16_t const result = static_cast<uint16_t>(component) + amount;
            return static_cast<uint8_t>(result > 0xFF ? 0xFF : result);
        };

        return
        {
            color.A,
            lighten(color.R),
            lighten(color.G),
            lighten(color.B),
        };
    }

    bool IsWildCard(Mooville::QUno::Model::Card const& card)
    {
        return card.Color() == Mooville::QUno::Model::Color::Wild;
    }
}

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
    _window = window;
    _compositor = Compositor();
    _root = _compositor.CreateContainerVisual();
    _target = _compositor.CreateTargetForCurrentView();
    _target.Root(_root);
    _visuals = _root.Children();

    window.PointerPressed({ this, &FrameworkView::OnPointerPressed });
    window.PointerMoved({ this, &FrameworkView::OnPointerMoved });
    window.PointerReleased({ this, &FrameworkView::OnPointerReleased });
    window.SizeChanged({ this, &FrameworkView::OnWindowSizeChanged });

    StartGame();

    return;
}

void FrameworkView::OnActivated(IInspectable const&, IActivatedEventArgs const&)
{
    return;
}

void FrameworkView::OnPointerPressed(IInspectable const&, PointerEventArgs const& args)
{
    TryHandlePointer(args.CurrentPoint().Position());

    return;
}

void FrameworkView::OnPointerMoved(IInspectable const&, PointerEventArgs const&)
{
    return;
}

void FrameworkView::OnPointerReleased(IInspectable const&, PointerEventArgs const&)
{
    return;
}

void FrameworkView::OnWindowSizeChanged(CoreWindow const&, WindowSizeChangedEventArgs const&)
{
    Render();

    return;
}

void FrameworkView::StartGame()
{
    _game = Game();
    _statusMessage.clear();

    Player human;
    human.Name(L"You");
    human.IsHuman(true);

    Player opponent;
    opponent.Name(L"CPU");
    opponent.IsHuman(false);

    _game.Players().Append(human);
    _game.Players().Append(opponent);
    _game.Deal();
    BeginHumanTurn();
    Render();

    return;
}

void FrameworkView::AdvanceAITurns()
{
    while (_game && !_game.IsGameOver() && !IsHumanTurn())
    {
        Player const aiPlayer = _game.CurrentPlayer();
        auto const cards = aiPlayer.Hand().Cards();
        bool played = false;

        for (uint32_t i = 0; i < cards.Size(); i++)
        {
            Card const card = cards.GetAt(i);

            if (_game.CanPlayCard(card))
            {
                _game.PlayCard(card, ChooseWildColorForPlayer(aiPlayer));
                _statusMessage = aiPlayer.Name() + L" played " + CardLabel(card);
                played = true;
                break;
            }
        }

        if (!played)
        {
            Card const drawn = _game.DrawCard();

            if (_game.CanPlayCard(drawn))
            {
                _game.PlayCard(drawn, ChooseWildColorForPlayer(aiPlayer));
                _statusMessage = aiPlayer.Name() + L" drew and played " + CardLabel(drawn);
            }
            else
            {
                _game.EndTurn();
                _statusMessage = aiPlayer.Name() + L" drew and passed";
            }
        }
    }

    if (IsHumanTurn() && !_game.IsGameOver())
    {
        BeginHumanTurn();
    }

    Render();

    return;
}

void FrameworkView::Render()
{
    if (!_visuals)
    {
        return;
    }

    Rect const bounds = _window.Bounds();
    float const width = bounds.Width;
    float const height = bounds.Height;

    _visuals.RemoveAll();
    _hitTargets.clear();

    RenderBackground(width, height);

    if (_game)
    {
        RenderDrawPile(width, height);
        RenderDiscardPile(width, height);
        RenderOpponentHand(width, height);
        RenderHumanHand(width, height);
        RenderEndTurnButton(width, height);
        RenderWildPicker(width, height);
    }

    UpdateWindowTitle();

    return;
}

void FrameworkView::RenderBackground(float width, float height)
{
    SpriteVisual const background = CreateRectVisual(
        { width, height },
        { 0.0f, 0.0f, 0.0f },
        { 0xFF, 0x14, 0x4D, 0x2A },
        1.0f);

    _visuals.InsertAtTop(background);

    return;
}

void FrameworkView::RenderDrawPile(float width, float height)
{
    float const x = (width / 2.0f) - PileSpacing - (CardWidth / 2.0f);
    float const y = (height / 2.0f) - (CardHeight / 2.0f);

    SpriteVisual const backShadow = CreateRectVisual(
        { CardWidth, CardHeight },
        { x + 6.0f, y + 6.0f, 0.0f },
        { 0x90, 0x00, 0x00, 0x00 },
        1.0f);

    bool const drawEnabled = IsHumanTurn() && !_game.IsGameOver() && !_hasDrawnThisTurn && !_showWildPicker;

    SpriteVisual const pile = CreateRectVisual(
        { CardWidth, CardHeight },
        { x, y, 0.0f },
        { 0xFF, 0x1A, 0x1A, 0x1A },
        drawEnabled ? 1.0f : 0.45f);

    SpriteVisual const accent = CreateRectVisual(
        { CardWidth - 20.0f, CardHeight - 20.0f },
        { x + 10.0f, y + 10.0f, 0.0f },
        { 0xFF, 0xF5, 0xF5, 0xF5 },
        0.22f);

    _visuals.InsertAtTop(backShadow);
    _visuals.InsertAtTop(pile);
    _visuals.InsertAtTop(accent);

    if (drawEnabled)
    {
        _hitTargets.push_back(
        {
            HitTargetKind::DrawPile,
            { x, y, CardWidth, CardHeight },
            0,
            Mooville::QUno::Model::Color::Wild
        });
    }

    return;
}

void FrameworkView::RenderDiscardPile(float width, float height)
{
    float const x = (width / 2.0f) + PileSpacing - (CardWidth / 2.0f);
    float const y = (height / 2.0f) - (CardHeight / 2.0f);

    Card const topCard = _game.Deck().CurrentCard();
    Windows::UI::Color const faceColor = CardColor(topCard.Color());

    SpriteVisual const shadow = CreateRectVisual(
        { CardWidth, CardHeight },
        { x + 6.0f, y + 6.0f, 0.0f },
        { 0x90, 0x00, 0x00, 0x00 },
        1.0f);

    SpriteVisual const face = CreateRectVisual(
        { CardWidth, CardHeight },
        { x, y, 0.0f },
        faceColor,
        1.0f);

    SpriteVisual const inset = CreateRectVisual(
        { CardWidth - 18.0f, CardHeight - 18.0f },
        { x + 9.0f, y + 9.0f, 0.0f },
        Lightened(faceColor, 0x20),
        0.65f);

    _visuals.InsertAtTop(shadow);
    _visuals.InsertAtTop(face);
    _visuals.InsertAtTop(inset);
    RenderCardMarkers(x, y, CardWidth, CardHeight, topCard, 1.0f);

    return;
}

void FrameworkView::RenderHumanHand(float width, float height)
{
    Player const human = HumanPlayer();
    auto const cards = human.Hand().Cards();
    float const totalWidth = (cards.Size() * CardWidth) + ((cards.Size() > 0 ? cards.Size() - 1 : 0) * CardSpacing);
    float const startX = (width - totalWidth) / 2.0f;
    float const y = height - CardHeight - HandBottomMargin;

    for (uint32_t i = 0; i < cards.Size(); i++)
    {
        Card const card = cards.GetAt(i);
        bool const playable = IsHumanTurn() && !_game.IsGameOver() && !_showWildPicker && _game.CanPlayCard(card);
        bool const selectedWild = _showWildPicker && _pendingWildCard && (card.Color() == _pendingWildCard.Color()) && (card.Value() == _pendingWildCard.Value());
        float const x = startX + (i * (CardWidth + CardSpacing));
        Windows::UI::Color const faceColor = CardColor(card.Color());

        SpriteVisual const shadow = CreateRectVisual(
            { CardWidth, CardHeight },
            { x + 5.0f, y + 5.0f, 0.0f },
            { 0x90, 0x00, 0x00, 0x00 },
            1.0f);

        SpriteVisual const face = CreateRectVisual(
            { CardWidth, CardHeight },
            { x, y, 0.0f },
            faceColor,
            playable || selectedWild ? 1.0f : 0.45f);

        Windows::UI::Color const insetColor = selectedWild ?
            Windows::UI::Color { 0xFF, 0xFF, 0xFF, 0xFF } :
            (playable ? Lightened(faceColor, 0x24) : Windows::UI::Color { 0xFF, 0x80, 0x80, 0x80 });

        SpriteVisual const inset = CreateRectVisual(
            { CardWidth - 14.0f, CardHeight - 14.0f },
            { x + 7.0f, y + 7.0f, 0.0f },
            insetColor,
            selectedWild ? 0.85f : (playable ? 0.70f : 0.28f));

        _visuals.InsertAtTop(shadow);
        _visuals.InsertAtTop(face);
        _visuals.InsertAtTop(inset);
        RenderCardMarkers(x, y, CardWidth, CardHeight, card, playable || selectedWild ? 1.0f : 0.4f);

        _hitTargets.push_back(
        {
            HitTargetKind::HandCard,
            { x, y, CardWidth, CardHeight },
            i,
            Mooville::QUno::Model::Color::Wild
        });
    }

    return;
}

void FrameworkView::RenderOpponentHand(float width, float)
{
    Player const opponent = OpponentPlayer();
    auto const cards = opponent.Hand().Cards();
    float const totalWidth = (cards.Size() * (CardWidth * 0.6f)) + ((cards.Size() > 0 ? cards.Size() - 1 : 0) * 10.0f);
    float const startX = (width - totalWidth) / 2.0f;
    float const scaledWidth = CardWidth * 0.6f;
    float const scaledHeight = CardHeight * 0.6f;

    for (uint32_t i = 0; i < cards.Size(); i++)
    {
        float const x = startX + (i * (scaledWidth + 10.0f));
        float const y = OpponentTopMargin;

        SpriteVisual const face = CreateRectVisual(
            { scaledWidth, scaledHeight },
            { x, y, 0.0f },
            { 0xFF, 0x21, 0x21, 0x21 },
            0.95f);

        SpriteVisual const inset = CreateRectVisual(
            { scaledWidth - 12.0f, scaledHeight - 12.0f },
            { x + 6.0f, y + 6.0f, 0.0f },
            { 0xFF, 0xD1, 0xD1, 0xD1 },
            0.16f);

        _visuals.InsertAtTop(face);
        _visuals.InsertAtTop(inset);
    }

    return;
}

void FrameworkView::RenderEndTurnButton(float width, float height)
{
    bool const enabled = IsHumanTurn() && !_game.IsGameOver() && _hasDrawnThisTurn && !_showWildPicker;

    float const x = width - StatusButtonWidth - 18.0f;
    float const y = height - StatusButtonHeight - 18.0f;

    SpriteVisual const button = CreateRectVisual(
        { StatusButtonWidth, StatusButtonHeight },
        { x, y, 0.0f },
        enabled ? Windows::UI::Color { 0xFF, 0xF4, 0xF4, 0xF4 } : Windows::UI::Color { 0xFF, 0x88, 0x88, 0x88 },
        enabled ? 0.95f : 0.45f);

    SpriteVisual const inner = CreateRectVisual(
        { StatusButtonWidth - 8.0f, StatusButtonHeight - 8.0f },
        { x + 4.0f, y + 4.0f, 0.0f },
        enabled ? Windows::UI::Color { 0xFF, 0x16, 0x61, 0x31 } : Windows::UI::Color { 0xFF, 0x4C, 0x4C, 0x4C },
        0.9f);

    _visuals.InsertAtTop(button);
    _visuals.InsertAtTop(inner);

    if (enabled)
    {
        _hitTargets.push_back(
        {
            HitTargetKind::EndTurn,
            { x, y, StatusButtonWidth, StatusButtonHeight },
            0,
            Mooville::QUno::Model::Color::Wild
        });
    }

    return;
}

void FrameworkView::RenderWildPicker(float width, float height)
{
    if (!_showWildPicker || !_pendingWildCard)
    {
        return;
    }

    float const overlayWidth = (WildButtonSize * 4.0f) + (WildButtonGap * 3.0f) + 32.0f;
    float const overlayHeight = WildButtonSize + 32.0f;
    float const overlayX = (width - overlayWidth) / 2.0f;
    float const overlayY = (height - overlayHeight) / 2.0f + 110.0f;

    SpriteVisual const backdrop = CreateRectVisual(
        { overlayWidth, overlayHeight },
        { overlayX, overlayY, 0.0f },
        { 0xEE, 0x10, 0x10, 0x10 },
        1.0f);

    _visuals.InsertAtTop(backdrop);

    Mooville::QUno::Model::Color const colors[] =
    {
        Mooville::QUno::Model::Color::Red,
        Mooville::QUno::Model::Color::Blue,
        Mooville::QUno::Model::Color::Yellow,
        Mooville::QUno::Model::Color::Green,
    };

    for (uint32_t i = 0; i < _countof(colors); i++)
    {
        float const x = overlayX + 16.0f + (i * (WildButtonSize + WildButtonGap));
        float const y = overlayY + 16.0f;
        Windows::UI::Color const swatchColor = CardColor(colors[i]);

        SpriteVisual const swatch = CreateRectVisual(
            { WildButtonSize, WildButtonSize },
            { x, y, 0.0f },
            swatchColor,
            1.0f);

        SpriteVisual const inset = CreateRectVisual(
            { WildButtonSize - 10.0f, WildButtonSize - 10.0f },
            { x + 5.0f, y + 5.0f, 0.0f },
            Lightened(swatchColor, 0x20),
            0.6f);

        _visuals.InsertAtTop(swatch);
        _visuals.InsertAtTop(inset);

        _hitTargets.push_back(
        {
            HitTargetKind::WildChoice,
            { x, y, WildButtonSize, WildButtonSize },
            0,
            colors[i]
        });
    }

    return;
}

void FrameworkView::RenderCardMarkers(float x, float y, float width, float height, Mooville::QUno::Model::Card const& card, float opacity)
{
    float const insetX = x + 18.0f;
    float const insetY = y + 20.0f;
    float const insetWidth = width - 36.0f;
    float const insetHeight = height - 40.0f;
    Windows::UI::Color markerColor { 0xFF, 0xFA, 0xFA, 0xFA };
    Value const value = card.Value();

    if (value >= Value::Zero && value <= Value::Nine)
    {
        int dotCount = static_cast<int>(value) - static_cast<int>(Value::Zero);

        if (value == Value::Zero)
        {
            SpriteVisual const ring = CreateRectVisual(
                { insetWidth * 0.58f, insetHeight * 0.58f },
                { x + (width * 0.21f), y + (height * 0.21f), 0.0f },
                markerColor,
                opacity * 0.95f);

            SpriteVisual const cutout = CreateRectVisual(
                { insetWidth * 0.34f, insetHeight * 0.34f },
                { x + (width * 0.33f), y + (height * 0.33f), 0.0f },
                CardColor(card.Color()),
                opacity);

            _visuals.InsertAtTop(ring);
            _visuals.InsertAtTop(cutout);
            return;
        }

        constexpr int columns = 3;
        float const dotSize = 10.0f;
        float const gapX = (insetWidth - (columns * dotSize)) / 2.0f;
        float const stepX = dotSize + gapX;
        float const stepY = dotSize + 8.0f;

        for (int i = 0; i < dotCount; i++)
        {
            int const row = i / columns;
            int const column = i % columns;
            float const dotX = insetX + column * stepX;
            float const dotY = insetY + row * stepY;

            SpriteVisual const dot = CreateRectVisual(
                { dotSize, dotSize },
                { dotX, dotY, 0.0f },
                markerColor,
                opacity * 0.95f);

            _visuals.InsertAtTop(dot);
        }

        return;
    }

    if (value == Value::Skip)
    {
        SpriteVisual const bar = CreateRectVisual(
            { insetWidth * 0.72f, 14.0f },
            { x + (width * 0.14f), y + (height * 0.44f), 0.0f },
            markerColor,
            opacity * 0.95f);

        _visuals.InsertAtTop(bar);
        return;
    }

    if (value == Value::Reverse)
    {
        SpriteVisual const bar1 = CreateRectVisual(
            { insetWidth * 0.66f, 12.0f },
            { x + (width * 0.17f), y + (height * 0.35f), 0.0f },
            markerColor,
            opacity * 0.95f);

        SpriteVisual const bar2 = CreateRectVisual(
            { insetWidth * 0.66f, 12.0f },
            { x + (width * 0.17f), y + (height * 0.53f), 0.0f },
            markerColor,
            opacity * 0.95f);

        _visuals.InsertAtTop(bar1);
        _visuals.InsertAtTop(bar2);
        return;
    }

    if (value == Value::DrawTwo)
    {
        SpriteVisual const left = CreateRectVisual(
            { 22.0f, 34.0f },
            { x + (width * 0.24f), y + (height * 0.35f), 0.0f },
            markerColor,
            opacity * 0.95f);

        SpriteVisual const right = CreateRectVisual(
            { 22.0f, 34.0f },
            { x + (width * 0.50f), y + (height * 0.35f), 0.0f },
            markerColor,
            opacity * 0.95f);

        _visuals.InsertAtTop(left);
        _visuals.InsertAtTop(right);
        return;
    }

    if ((value == Value::Wild) || (value == Value::WildDrawFour))
    {
        float const quadWidth = insetWidth * 0.42f;
        float const quadHeight = insetHeight * 0.34f;
        float const left = x + (width * 0.14f);
        float const top = y + (height * 0.28f);

        SpriteVisual const red = CreateRectVisual({ quadWidth, quadHeight }, { left, top, 0.0f }, CardColor(Mooville::QUno::Model::Color::Red), opacity);
        SpriteVisual const blue = CreateRectVisual({ quadWidth, quadHeight }, { left + quadWidth + 4.0f, top, 0.0f }, CardColor(Mooville::QUno::Model::Color::Blue), opacity);
        SpriteVisual const yellow = CreateRectVisual({ quadWidth, quadHeight }, { left, top + quadHeight + 4.0f, 0.0f }, CardColor(Mooville::QUno::Model::Color::Yellow), opacity);
        SpriteVisual const green = CreateRectVisual({ quadWidth, quadHeight }, { left + quadWidth + 4.0f, top + quadHeight + 4.0f, 0.0f }, CardColor(Mooville::QUno::Model::Color::Green), opacity);

        _visuals.InsertAtTop(red);
        _visuals.InsertAtTop(blue);
        _visuals.InsertAtTop(yellow);
        _visuals.InsertAtTop(green);

        if (value == Value::WildDrawFour)
        {
            SpriteVisual const band = CreateRectVisual(
                { insetWidth * 0.74f, 10.0f },
                { x + (width * 0.13f), y + (height * 0.72f), 0.0f },
                markerColor,
                opacity * 0.95f);

            _visuals.InsertAtTop(band);
        }
    }
}

SpriteVisual FrameworkView::CreateRectVisual(float2 const& size, float3 const& offset, Windows::UI::Color const& color, float opacity)
{
    SpriteVisual const visual = _compositor.CreateSpriteVisual();
    visual.Size(size);
    visual.Offset(offset);
    visual.Brush(_compositor.CreateColorBrush(color));
    visual.Opacity(opacity);

    return visual;
}

bool FrameworkView::TryHandlePointer(float2 const& point)
{
    if (!_game || _game.IsGameOver() || !IsHumanTurn())
    {
        return false;
    }

    for (auto it = _hitTargets.rbegin(); it != _hitTargets.rend(); ++it)
    {
        if (!Contains(*it, point))
        {
            continue;
        }

        switch (it->Kind)
        {
        case HitTargetKind::DrawPile:
        {
            if (_hasDrawnThisTurn || _showWildPicker)
            {
                return true;
            }

            Card const drawn = _game.DrawCard();
            _hasDrawnThisTurn = true;
            _statusMessage = L"You drew " + CardLabel(drawn);
            Render();
            return true;
        }

        case HitTargetKind::EndTurn:
            if (_hasDrawnThisTurn && !_showWildPicker)
            {
                CompleteHumanTurn();
            }
            return true;

        case HitTargetKind::WildChoice:
            if (_showWildPicker && _pendingWildCard)
            {
                PlayHumanCard(_pendingWildCard, it->ColorChoice);
            }
            return true;

        case HitTargetKind::HandCard:
        {
            auto const cards = HumanPlayer().Hand().Cards();

            if (it->CardIndex >= cards.Size() || _showWildPicker)
            {
                return true;
            }

            Card const selectedCard = cards.GetAt(it->CardIndex);

            if (!_game.CanPlayCard(selectedCard))
            {
                _statusMessage = L"That card does not match the discard pile.";
                Render();
                return true;
            }

            if (IsWildCard(selectedCard))
            {
                _pendingWildCard = selectedCard;
                _showWildPicker = true;
                _statusMessage = L"Choose a color for " + CardLabel(selectedCard);
                Render();
                return true;
            }

            PlayHumanCard(selectedCard, Mooville::QUno::Model::Color::Wild);
            return true;
        }

        default:
            return false;
        }
    }

    return false;
}

void FrameworkView::PlayHumanCard(Mooville::QUno::Model::Card const& card, Mooville::QUno::Model::Color wildColor)
{
    _game.PlayCard(card, wildColor);
    _statusMessage = L"You played " + CardLabel(card);
    _showWildPicker = false;
    _pendingWildCard = nullptr;
    _hasDrawnThisTurn = false;
    Render();
    AdvanceAITurns();

    return;
}

void FrameworkView::BeginHumanTurn()
{
    _hasDrawnThisTurn = false;
    _showWildPicker = false;
    _pendingWildCard = nullptr;

    return;
}

void FrameworkView::CompleteHumanTurn()
{
    _game.EndTurn();
    _statusMessage = L"You ended your turn.";
    BeginHumanTurn();
    Render();
    AdvanceAITurns();

    return;
}

void FrameworkView::UpdateWindowTitle()
{
    if (!_game)
    {
        ApplicationView::GetForCurrentView().Title(L"QUno");
        return;
    }

    std::wstring title = L"QUno";

    if (_game.IsGameOver())
    {
        title += L" | Winner: ";
        title += WinnerLabel().c_str();
    }
    else
    {
        title += L" | Turn: ";
        title += _game.CurrentPlayer().Name().c_str();
    }

    title += L" | Top: ";
    title += CardLabel(_game.Deck().CurrentCard()).c_str();

    if (_game.Deck().CurrentCard().Color() == Mooville::QUno::Model::Color::Wild)
    {
        title += L" (";
        title += ColorLabel(_game.Deck().CurrentWildColor()).c_str();
        title += L")";
    }

    Player const human = HumanPlayer();
    title += L" | Your hand:";

    auto const cards = human.Hand().Cards();

    for (uint32_t i = 0; i < cards.Size(); i++)
    {
        title += L" [";
        title += std::to_wstring(i + 1);
        title += L"] ";
        title += CardLabel(cards.GetAt(i)).c_str();
    }

    if (_showWildPicker)
    {
        title += L" | Pick a color from the four squares above your hand.";
    }
    else if (_hasDrawnThisTurn)
    {
        title += L" | You may play a card or click End Turn.";
    }

    if (!_statusMessage.empty())
    {
        title += L" | ";
        title += _statusMessage.c_str();
    }

    ApplicationView::GetForCurrentView().Title(hstring { title });

    return;
}

Player FrameworkView::HumanPlayer()
{
    for (Player const& player : _game.Players())
    {
        if (player.IsHuman())
        {
            return player;
        }
    }

    throw hresult_error(E_FAIL);
}

Player FrameworkView::OpponentPlayer()
{
    for (Player const& player : _game.Players())
    {
        if (!player.IsHuman())
        {
            return player;
        }
    }

    throw hresult_error(E_FAIL);
}

bool FrameworkView::IsHumanTurn() const
{
    return _game && !_game.IsGameOver() && _game.CurrentPlayer().IsHuman();
}

Mooville::QUno::Model::Color FrameworkView::ChooseWildColorForPlayer(Mooville::QUno::Model::Player const& player)
{
    if (player.Hand().Cards().Size() == 0)
    {
        return Mooville::QUno::Model::Color::Red;
    }

    return player.ChooseWildColor();
}

hstring FrameworkView::CardLabel(Mooville::QUno::Model::Card const& card)
{
    return ColorLabel(card.Color()) + L" " + ValueLabel(card.Value());
}

hstring FrameworkView::ColorLabel(Mooville::QUno::Model::Color color)
{
    switch (color)
    {
    case Mooville::QUno::Model::Color::Red:
        return L"Red";
    case Mooville::QUno::Model::Color::Blue:
        return L"Blue";
    case Mooville::QUno::Model::Color::Yellow:
        return L"Yellow";
    case Mooville::QUno::Model::Color::Green:
        return L"Green";
    default:
        return L"Wild";
    }
}

hstring FrameworkView::ValueLabel(Mooville::QUno::Model::Value value)
{
    switch (value)
    {
    case Mooville::QUno::Model::Value::Wild:
        return L"Wild";
    case Mooville::QUno::Model::Value::WildDrawFour:
        return L"Wild Draw Four";
    case Mooville::QUno::Model::Value::DrawTwo:
        return L"Draw Two";
    case Mooville::QUno::Model::Value::Reverse:
        return L"Reverse";
    case Mooville::QUno::Model::Value::Skip:
        return L"Skip";
    case Mooville::QUno::Model::Value::Zero:
        return L"Zero";
    case Mooville::QUno::Model::Value::One:
        return L"One";
    case Mooville::QUno::Model::Value::Two:
        return L"Two";
    case Mooville::QUno::Model::Value::Three:
        return L"Three";
    case Mooville::QUno::Model::Value::Four:
        return L"Four";
    case Mooville::QUno::Model::Value::Five:
        return L"Five";
    case Mooville::QUno::Model::Value::Six:
        return L"Six";
    case Mooville::QUno::Model::Value::Seven:
        return L"Seven";
    case Mooville::QUno::Model::Value::Eight:
        return L"Eight";
    default:
        return L"Nine";
    }
}

hstring FrameworkView::WinnerLabel()
{
    for (Player const& player : _game.Players())
    {
        if (player.Hand().Cards().Size() == 0)
        {
            return player.Name();
        }
    }

    return L"No winner";
}

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    auto frameworkViewSource = make<FrameworkViewSource>();
    CoreApplication::Run(frameworkViewSource);

    return 0;
}
