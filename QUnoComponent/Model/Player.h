// QUnoComponent - Player.h
// 2021 Roger Deetz

#pragma once

#include "Model\Hand.h"
#include "Model.Player.g.h"

namespace winrt::Mooville::QUno::Model::implementation
{
    struct Player : PlayerT<Player>
    {
        Player();

        hstring Name();
        void Name(hstring name);
        boolean IsHuman();
        void IsHuman(boolean isHuman);
        Mooville::QUno::Model::Hand Hand();
        Mooville::QUno::Model::Card ChooseCardToPlay(); // Need at least the current card, maybe other game data?
        Mooville::QUno::Model::Color ChooseWildColor();

    private:
        hstring _name;
        boolean _isHuman;
        Mooville::QUno::Model::Hand _hand;
    };
}

namespace winrt::Mooville::QUno::Model::factory_implementation
{
    struct Player : PlayerT<Player, implementation::Player>
    {
    };
}
