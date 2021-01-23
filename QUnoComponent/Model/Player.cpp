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
}
