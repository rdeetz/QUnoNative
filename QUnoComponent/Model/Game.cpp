// QUnoComponent - Game.cpp
// 2021 Roger Deetz

#include "pch.h"
#include "Model\Game.h"
#include "Model.Game.g.cpp"

namespace winrt::Mooville::QUno::Model::implementation
{
    namespace
    {
        constexpr int StartingHandSize = 7;

        bool IsWild(Mooville::QUno::Model::Card const& card)
        {
            return card.Color() == Mooville::QUno::Model::Color::Wild;
        }

        bool IsSkip(Mooville::QUno::Model::Card const& card)
        {
            return card.Value() == Mooville::QUno::Model::Value::Skip;
        }

        bool IsReverse(Mooville::QUno::Model::Card const& card)
        {
            return card.Value() == Mooville::QUno::Model::Value::Reverse;
        }

        bool IsDrawPenalty(Mooville::QUno::Model::Card const& card)
        {
            return (card.Value() == Mooville::QUno::Model::Value::DrawTwo) ||
                (card.Value() == Mooville::QUno::Model::Value::WildDrawFour);
        }

        int GetDrawPenaltyCount(Mooville::QUno::Model::Card const& card)
        {
            switch (card.Value())
            {
            case Mooville::QUno::Model::Value::DrawTwo:
                return 2;
            case Mooville::QUno::Model::Value::WildDrawFour:
                return 4;
            default:
                return 0;
            }
        }

        bool CardsMatch(Mooville::QUno::Model::Card const& left, Mooville::QUno::Model::Card const& right)
        {
            return (left.Color() == right.Color()) && (left.Value() == right.Value());
        }

        int NormalizeIndex(int index, uint32_t count)
        {
            int const signedCount = static_cast<int>(count);
            return (index % signedCount + signedCount) % signedCount;
        }

        int DirectionOffset(Mooville::QUno::Model::Direction direction)
        {
            return (direction == Mooville::QUno::Model::Direction::Clockwise) ? 1 : -1;
        }
    }

    Game::Game()
    {
        _players = winrt::single_threaded_observable_vector<Mooville::QUno::Model::Player>();
        _deck = make<Mooville::QUno::Model::implementation::Deck>();
        _currentDirection = Mooville::QUno::Model::Direction::Clockwise;
        _currentPlayerIndex = 0;
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
        if (_players.Size() == 0)
        {
            throw hresult_out_of_bounds();
        }

        return _players.GetAt(static_cast<uint32_t>(NormalizeIndex(_currentPlayerIndex, _players.Size())));
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
        for (Mooville::QUno::Model::Player const& player : _players)
        {
            if (player.Hand().Cards().Size() == 0)
            {
                return true;
            }
        }

        return false;
    }

    void Game::Deal()
    {
        if (_players.Size() == 0)
        {
            throw hresult_out_of_bounds();
        }

        _deck = make<Mooville::QUno::Model::implementation::Deck>();
        _deck.Shuffle();

        for (Mooville::QUno::Model::Player const& player : _players)
        {
            player.Hand().Cards().Clear();
        }

        for (int cardIndex = 0; cardIndex < StartingHandSize; cardIndex++)
        {
            for (Mooville::QUno::Model::Player const& player : _players)
            {
                player.Hand().Cards().Append(_deck.Draw());
            }
        }

        while (true)
        {
            Mooville::QUno::Model::Card const openingCard = _deck.Draw();

            if (IsWild(openingCard))
            {
                _deck.DrawPile().Append(openingCard);
                continue;
            }

            _deck.Play(openingCard);
            _deck.CurrentWildColor(Mooville::QUno::Model::Color::Wild);
            break;
        }

        _currentDirection = Mooville::QUno::Model::Direction::Clockwise;
        _currentPlayerIndex = 0;

        return;
    }

    bool Game::CanPlayCard(Mooville::QUno::Model::Card card)
    {
        Mooville::QUno::Model::Card const currentCard = _deck.CurrentCard();

        if (IsWild(card))
        {
            return true;
        }

        if (currentCard.Color() == Mooville::QUno::Model::Color::Wild)
        {
            return (card.Color() == _deck.CurrentWildColor()) || (card.Value() == currentCard.Value());
        }

        return (card.Color() == currentCard.Color()) || (card.Value() == currentCard.Value());
    }

    void Game::PlayCard(Mooville::QUno::Model::Card card, Mooville::QUno::Model::Color wildColor)
    {
        if (!CanPlayCard(card))
        {
            throw hresult_invalid_argument();
        }

        Mooville::QUno::Model::Player const currentPlayer = CurrentPlayer();
        auto const handCards = currentPlayer.Hand().Cards();
        bool foundCard = false;

        for (uint32_t i = 0; i < handCards.Size(); i++)
        {
            if (CardsMatch(handCards.GetAt(i), card))
            {
                handCards.RemoveAt(i);
                foundCard = true;
                break;
            }
        }

        if (!foundCard)
        {
            throw hresult_invalid_argument();
        }

        if (IsWild(card))
        {
            if (wildColor == Mooville::QUno::Model::Color::Wild)
            {
                handCards.Append(card);
                throw hresult_invalid_argument();
            }

            _deck.CurrentWildColor(wildColor);
        }
        else
        {
            _deck.CurrentWildColor(Mooville::QUno::Model::Color::Wild);
        }

        _deck.Play(card);

        if (IsGameOver())
        {
            return;
        }

        int stepCount = 1;

        if (IsReverse(card))
        {
            _currentDirection =
                (_currentDirection == Mooville::QUno::Model::Direction::Clockwise) ?
                Mooville::QUno::Model::Direction::Counterclockwise :
                Mooville::QUno::Model::Direction::Clockwise;

            if (_players.Size() == 2)
            {
                stepCount = 2;
            }
        }

        if (IsSkip(card))
        {
            stepCount = 2;
        }

        if (IsDrawPenalty(card))
        {
            int const penaltyOffset = (_currentDirection == Mooville::QUno::Model::Direction::Clockwise) ? 1 : -1;
            int const penalizedIndex = NormalizeIndex(_currentPlayerIndex + penaltyOffset, _players.Size());
            auto const penalizedPlayer = _players.GetAt(static_cast<uint32_t>(penalizedIndex));

            for (int i = 0; i < GetDrawPenaltyCount(card); i++)
            {
                penalizedPlayer.Hand().Cards().Append(_deck.Draw());
            }

            stepCount = 2;
        }

        int const directionOffset = (_currentDirection == Mooville::QUno::Model::Direction::Clockwise) ? 1 : -1;
        _currentPlayerIndex = NormalizeIndex(_currentPlayerIndex + (directionOffset * stepCount), _players.Size());

        return;
    }

    Mooville::QUno::Model::Card Game::DrawCard()
    {
        Mooville::QUno::Model::Card const card = _deck.Draw();
        CurrentPlayer().Hand().Cards().Append(card);

        return card;
    }

    void Game::EndTurn()
    {
        _currentPlayerIndex = NormalizeIndex(_currentPlayerIndex + DirectionOffset(_currentDirection), _players.Size());

        return;
    }
}
