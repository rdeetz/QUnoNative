// QUnoComponent - Game.cpp
// 2021 Roger Deetz

#include "pch.h"
#include "Model\Game.h"
#include "Model.Game.g.cpp"

namespace winrt::Mooville::QUno::Model::implementation
{
    Game::Game()
    {
    }

    Windows::Foundation::Collections::IObservableVector<Mooville::QUno::Model::Player> Game::Players()
    {
        throw hresult_not_implemented();
    }

    Mooville::QUno::Model::Deck Game::Deck()
    {
        throw hresult_not_implemented();
    }

    Mooville::QUno::Model::Direction Game::CurrentDirection()
    {
        throw hresult_not_implemented();
    }

    void Game::CurrentDirection(Mooville::QUno::Model::Direction currentDirection)
    {
        throw hresult_not_implemented();
    }

    Mooville::QUno::Model::Player Game::CurrentPlayer()
    {
        throw hresult_not_implemented();
    }

    int Game::CurrentPlayerIndex()
    {
        throw hresult_not_implemented();
    }

    void Game::CurrentPlayerIndex(int currentPlayerIndex)
    {
        throw hresult_not_implemented();
    }

    bool Game::IsGameOver()
    {
        throw hresult_not_implemented();
    }

    void Game::Deal()
    {
        throw hresult_not_implemented();
    }

    bool Game::CanPlay(Mooville::QUno::Model::Card card)
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
