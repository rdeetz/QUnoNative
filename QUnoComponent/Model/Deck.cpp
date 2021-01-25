// QUnoComponent - Deck.cpp
// 2021 Roger Deetz

#include "pch.h"
#include "Model\Deck.h"
#include "Model.Deck.g.cpp"

namespace winrt::Mooville::QUno::Model::implementation
{
    Deck::Deck()
    {
        _currentWildColor = Mooville::QUno::Model::Color::Wild;
        _drawPile = winrt::single_threaded_vector<Mooville::QUno::Model::Card>();
        _discardPile = winrt::single_threaded_vector<Mooville::QUno::Model::Card>();
    }

    Mooville::QUno::Model::Card Deck::CurrentCard()
    {
        throw hresult_not_implemented();
    }

    void Deck::CurrentCard(Mooville::QUno::Model::Card card)
    {
        throw hresult_not_implemented();
    }

    Mooville::QUno::Model::Color Deck::CurrentWildColor()
    {
        return _currentWildColor;
    }

    void Deck::CurrentWildColor(Mooville::QUno::Model::Color wildColor)
    {
        _currentWildColor = wildColor;
        return;
    }

    Windows::Foundation::Collections::IVector<Mooville::QUno::Model::Card> Deck::DrawPile()
    {
        return _drawPile;
    }

    Windows::Foundation::Collections::IVector<Mooville::QUno::Model::Card> Deck::DiscardPile()
    {
        return _discardPile;
    }

    void Deck::Shuffle()
    {
        throw hresult_not_implemented();
    }

    Mooville::QUno::Model::Card Deck::Draw()
    {
        throw hresult_not_implemented();
    }

    void Deck::Play(Mooville::QUno::Model::Card card)
    {
        throw hresult_not_implemented();
    }
}
