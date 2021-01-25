// QUnoComponent - Deck.h
// 2021 Roger Deetz

#pragma once

#include "Model\Color.h"
#include "Model\Card.h"
#include "Model.Deck.g.h"

namespace winrt::Mooville::QUno::Model::implementation
{
    struct Deck : DeckT<Deck>
    {
        Deck();

        Mooville::QUno::Model::Card CurrentCard();
        void CurrentCard(Mooville::QUno::Model::Card card);
        Mooville::QUno::Model::Color CurrentWildColor();
        void CurrentWildColor(Mooville::QUno::Model::Color wildColor);
        Windows::Foundation::Collections::IVector<Mooville::QUno::Model::Card> DrawPile();
        Windows::Foundation::Collections::IVector<Mooville::QUno::Model::Card> DiscardPile();
        void Shuffle();
        Mooville::QUno::Model::Card Draw();
        void Play(Mooville::QUno::Model::Card card);

    private:
        Mooville::QUno::Model::Color _currentWildColor;
        Windows::Foundation::Collections::IVector<Mooville::QUno::Model::Card> _drawPile;
        Windows::Foundation::Collections::IVector<Mooville::QUno::Model::Card> _discardPile;
    };
}

namespace winrt::Mooville::QUno::Model::factory_implementation
{
    struct Deck : DeckT<Deck, implementation::Deck>
    {
    };
}
