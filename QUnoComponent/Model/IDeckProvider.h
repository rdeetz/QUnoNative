// QUnoComponent - IDeckProvider.h
// 2021 Roger Deetz

#pragma once

#include "Model\Card.h"

namespace winrt::Mooville::QUno::Model::implementation
{
	__interface IDeckProvider
	{
		Windows::Foundation::Collections::IIterable<Mooville::QUno::Model::Card> ProvideCards();
	};
}
