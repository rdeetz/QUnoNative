// QUnoComponent - Game.cpp
// 2021 Roger Deetz

#include "pch.h"
#include "Model\Game.h"
#include "Model.Game.g.cpp"

namespace winrt::Mooville::QUno::Model::implementation
{
    Game::Game()
    {
        _players = winrt::single_threaded_observable_vector<Mooville::QUno::Model::Player>();
        _deck = make<Mooville::QUno::Model::implementation::Deck>();
    }

    Windows::Foundation::Collections::IObservableVector<Mooville::QUno::Model::Player> Game::Players()
    {
        return _players;
    }

    Mooville::QUno::Model::Deck Game::Deck()
    {
        return _deck;
    }

    Mooville::QUno::Model::Direction Game::CurrentDirection()
    {
        return _currentDirection;
    }

    void Game::CurrentDirection(Mooville::QUno::Model::Direction currentDirection)
    {
        _currentDirection = currentDirection;
        return;
    }

    Mooville::QUno::Model::Player Game::CurrentPlayer()
    {
        // TODO Find the player in the players collection at the current index.
        throw hresult_not_implemented();
    }

    int Game::CurrentPlayerIndex()
    {
        return _currentPlayerIndex;
    }

    void Game::CurrentPlayerIndex(int currentPlayerIndex)
    {
        _currentPlayerIndex = currentPlayerIndex;
        return;
    }

    bool Game::IsGameOver()
    {
        throw hresult_not_implemented();
    }

    void Game::Deal()
    {
        throw hresult_not_implemented();
    }

    bool Game::CanPlayCard(Mooville::QUno::Model::Card card)
    {
        throw hresult_not_implemented();
    }

    void Game::PlayCard(Mooville::QUno::Model::Card card, Mooville::QUno::Model::Color wildColor)
    {
        throw hresult_not_implemented();
    }

    Mooville::QUno::Model::Card Game::DrawCard()
    {
        throw hresult_not_implemented();
    }
}
