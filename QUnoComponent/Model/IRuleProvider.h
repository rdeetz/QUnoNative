// QUnoComponent - IRuleProvider.h
// 2021 Roger Deetz

#pragma once

#include "Model\Color.h"
#include "Model\Card.h"

namespace winrt::Mooville::QUno::Model::implementation
{
	__interface IRuleProvider
	{
		int GetHandSize();
		bool CanPlay(Mooville::QUno::Model::Card card, Mooville::QUno::Model::Card currentCard, Mooville::QUno::Model::Color currentWildColor);
		bool ChangedDirection(Mooville::QUno::Model::Card card);
		int	GetPlayerIncrement(Mooville::QUno::Model::Card card);
		int GetNumberOfCardsToAdd(Mooville::QUno::Model::Card card);
		bool ValidateWildColor(Mooville::QUno::Model::Card card, Mooville::QUno::Model::Color wildColor);
	};
}
