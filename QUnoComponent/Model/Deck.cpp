// QUnoComponent - Deck.cpp
// 2021 Roger Deetz

#include "pch.h"
#include "Model\Deck.h"
#include "Model.Deck.g.cpp"
#include <random>

namespace winrt::Mooville::QUno::Model::implementation
{
    Deck::Deck()
    {
        _currentWildColor = Mooville::QUno::Model::Color::Wild; // TODO Can this be nullable, or should it be?
        _drawPile = winrt::single_threaded_vector<Mooville::QUno::Model::Card>();
        _discardPile = winrt::single_threaded_vector<Mooville::QUno::Model::Card>();

        ProvideCards();
    }

    Mooville::QUno::Model::Card Deck::CurrentCard()
    {
        if (_drawPile.Size() > 0)
        {
            auto card = _drawPile.GetAt(0);
            return card;
        }
        else
        {
            // TODO Provide an error message.
            throw hresult_out_of_bounds();
        }
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
        if (_drawPile.Size() > 0)
        {
            auto card = _drawPile.GetAt(0);
            _drawPile.RemoveAt(0);
            return card;
        }
        else
        {
            if (_discardPile.Size() > 0)
            {
                auto card = _discardPile.GetAt(0);
                _discardPile.RemoveAt(0);

                for (uint32_t i = 0; i < _discardPile.Size(); i++)
                {
                    _drawPile.Append(_discardPile.GetAt(i));
                }

                _discardPile.Clear();
                _discardPile.InsertAt(0, card);

                for (uint32_t i = _drawPile.Size() - 1; i > 0; i--)
                {
                    uint32_t randomIndex = GetRandomIndex(_drawPile.Size() - 1);
                    auto temp = _drawPile.GetAt(i);
                    _drawPile.SetAt(i, _drawPile.GetAt(randomIndex));
                    _drawPile.SetAt(randomIndex, temp);
                }

                // TODO Fire the "reshuffled" event?
            }

            if (_drawPile.Size() > 0)
            {
                auto card = _drawPile.GetAt(0);
                _drawPile.RemoveAt(0);
                return card;
            }
            else
            {
                // TODO Provide a better error message here.
                throw hresult_error();
            }
        }
    }

    void Deck::Play(Mooville::QUno::Model::Card card)
    {
        _discardPile.InsertAt(0, card);
        return;
    }

    void Deck::ProvideCards()
    {
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Wild, Mooville::QUno::Model::Value::WildDrawFour));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Wild, Mooville::QUno::Model::Value::WildDrawFour));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Wild, Mooville::QUno::Model::Value::Wild));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Wild, Mooville::QUno::Model::Value::Wild));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Red, Mooville::QUno::Model::Value::DrawTwo));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Red, Mooville::QUno::Model::Value::Reverse));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Red, Mooville::QUno::Model::Value::Skip));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Red, Mooville::QUno::Model::Value::Zero));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Red, Mooville::QUno::Model::Value::One));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Red, Mooville::QUno::Model::Value::Two));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Red, Mooville::QUno::Model::Value::Three));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Red, Mooville::QUno::Model::Value::Four));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Red, Mooville::QUno::Model::Value::Five));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Red, Mooville::QUno::Model::Value::Six));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Red, Mooville::QUno::Model::Value::Seven));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Red, Mooville::QUno::Model::Value::Eight));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Red, Mooville::QUno::Model::Value::Nine));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Blue, Mooville::QUno::Model::Value::DrawTwo));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Blue, Mooville::QUno::Model::Value::Reverse));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Blue, Mooville::QUno::Model::Value::Skip));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Blue, Mooville::QUno::Model::Value::Zero));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Blue, Mooville::QUno::Model::Value::One));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Blue, Mooville::QUno::Model::Value::Two));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Blue, Mooville::QUno::Model::Value::Three));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Blue, Mooville::QUno::Model::Value::Four));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Blue, Mooville::QUno::Model::Value::Five));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Blue, Mooville::QUno::Model::Value::Six));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Blue, Mooville::QUno::Model::Value::Seven));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Blue, Mooville::QUno::Model::Value::Eight));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Blue, Mooville::QUno::Model::Value::Nine));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Yellow, Mooville::QUno::Model::Value::DrawTwo));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Yellow, Mooville::QUno::Model::Value::Reverse));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Yellow, Mooville::QUno::Model::Value::Skip));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Yellow, Mooville::QUno::Model::Value::Zero));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Yellow, Mooville::QUno::Model::Value::One));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Yellow, Mooville::QUno::Model::Value::Two));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Yellow, Mooville::QUno::Model::Value::Three));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Yellow, Mooville::QUno::Model::Value::Four));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Yellow, Mooville::QUno::Model::Value::Five));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Yellow, Mooville::QUno::Model::Value::Six));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Yellow, Mooville::QUno::Model::Value::Seven));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Yellow, Mooville::QUno::Model::Value::Eight));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Yellow, Mooville::QUno::Model::Value::Nine));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Green, Mooville::QUno::Model::Value::DrawTwo));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Green, Mooville::QUno::Model::Value::Reverse));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Green, Mooville::QUno::Model::Value::Skip));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Green, Mooville::QUno::Model::Value::Zero));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Green, Mooville::QUno::Model::Value::One));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Green, Mooville::QUno::Model::Value::Two));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Green, Mooville::QUno::Model::Value::Three));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Green, Mooville::QUno::Model::Value::Four));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Green, Mooville::QUno::Model::Value::Five));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Green, Mooville::QUno::Model::Value::Six));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Green, Mooville::QUno::Model::Value::Seven));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Green, Mooville::QUno::Model::Value::Eight));
        _drawPile.Append(make<Mooville::QUno::Model::implementation::Card>(Mooville::QUno::Model::Color::Green, Mooville::QUno::Model::Value::Nine));

        return;
    }

    uint32_t Deck::GetRandomIndex(uint32_t max)
    {
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<> distribution(0, max);

        return distribution(generator);
    }
}
