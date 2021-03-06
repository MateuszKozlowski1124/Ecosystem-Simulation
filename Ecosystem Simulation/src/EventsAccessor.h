#pragma once

#include "Libraries.h"

static class EventsAccessor
{
public:
	static bool hasEventOccured(
		const sf::Event::EventType& eventType,
		const std::vector<sf::Event>& events
	);
};
