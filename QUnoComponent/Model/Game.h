// QUnoComponent - Game.h
// 2021 Roger Deetz

#pragma once

#include "Model\Color.h"
#include "Model\Card.h"
#include "Model\Direction.h"
#include "Model\Player.h"
#include "Model\Deck.h"
#include "Model.Game.g.h"

namespace winrt::Mooville::QUno::Model::implementation
{
    struct Game : GameT<Game>
    {
        Game();

        Windows::Foundation::Collections::IObservableVector<Mooville::QUno::Model::Player> Players();
        Mooville::QUno::Model::Deck Deck();
        Mooville::QUno::Model::Direction CurrentDirection();
        void CurrentDirection(Mooville::QUno::Model::Direction currentDirection);
        Mooville::QUno::Model::Player CurrentPlayer();
        int CurrentPlayerIndex();
        void CurrentPlayerIndex(int currentPlayerIndex);
        bool IsGameOver();
        void Deal();
        bool CanPlayCard(Mooville::QUno::Model::Card card);
        void PlayCard(Mooville::QUno::Model::Card card, Mooville::QUno::Model::Color wildColor);
        Mooville::QUno::Model::Card DrawCard();

    private:
        Windows::Foundation::Collections::IObservableVector<Mooville::QUno::Model::Player> _players;
        Mooville::QUno::Model::Deck _deck;
        Mooville::QUno::Model::Direction _currentDirection;
        int _currentPlayerIndex;
    };
}

namespace winrt::Mooville::QUno::Model::factory_implementation
{
    struct Game : GameT<Game, implementation::Game>
    {
    };
}
