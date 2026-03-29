// QUnoComponent - Player.cpp
// 2021 Roger Deetz

#include "pch.h"
#include "Model\Player.h"
#include "Model.Player.g.cpp"

namespace winrt::Mooville::QUno::Model::implementation
{
    Player::Player()
    {
        _name = L"Player 1";
        _isHuman = false;
        _hand = make<Mooville::QUno::Model::implementation::Hand>();
    }

    hstring Player::Name()
    {
        return _name;
    }

    void Player::Name(hstring name)
    {
        _name = name;
        return;
    }

    boolean Player::IsHuman()
    {
        return _isHuman;
    }

    void Player::IsHuman(boolean isHuman)
    {
        _isHuman = isHuman;
        return;
    }

    Mooville::QUno::Model::Hand Player::Hand()
    {
        return _hand;
    }

    Mooville::QUno::Model::Card Player::ChooseCardToPlay()
    {
        if (_hand.Cards().Size() == 0)
        {
            throw hresult_out_of_bounds();
        }

        return _hand.Cards().GetAt(0);
    }

    Mooville::QUno::Model::Color Player::ChooseWildColor()
    {
        uint32_t redCount = 0;
        uint32_t blueCount = 0;
        uint32_t yellowCount = 0;
        uint32_t greenCount = 0;

        for (Mooville::QUno::Model::Card const& card : _hand.Cards())
        {
            switch (card.Color())
            {
            case Mooville::QUno::Model::Color::Red:
                redCount++;
                break;
            case Mooville::QUno::Model::Color::Blue:
                blueCount++;
                break;
            case Mooville::QUno::Model::Color::Yellow:
                yellowCount++;
                break;
            case Mooville::QUno::Model::Color::Green:
                greenCount++;
                break;
            default:
                break;
            }
        }

        Mooville::QUno::Model::Color chosenColor = Mooville::QUno::Model::Color::Red;
        uint32_t bestCount = redCount;

        if (blueCount > bestCount)
        {
            chosenColor = Mooville::QUno::Model::Color::Blue;
            bestCount = blueCount;
        }

        if (yellowCount > bestCount)
        {
            chosenColor = Mooville::QUno::Model::Color::Yellow;
            bestCount = yellowCount;
        }

        if (greenCount > bestCount)
        {
            chosenColor = Mooville::QUno::Model::Color::Green;
        }

        return chosenColor;
    }
}
