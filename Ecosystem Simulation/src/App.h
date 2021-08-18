#pragma once

#include "MainMenuState.h"
#include "FPS.h"

class App
{
public:
	App();
	~App();
	
	void run();

private:
	void initVariables();
	void initGraphicsSettings();
	void initWindow();
	void initKeys();
	void initEcosystem();
	void initStateData();
	void initStates();
	void initFont();
	void initFPSpreview();

	void updateDt();
	void update();
	
	void updateFPSpreview();
	void updateEvents();
	void updateStates();

	void render();

private:
	GraphicsSettings m_gfxSettings;
	sf::RenderWindow* m_window;
	std::unordered_map<std::string, int> m_supportedKeys;
	std::stack<State*> m_states;
	Ecosystem* m_ecosystem;
	std::vector<sf::Event> m_events;

	StateData m_stateData;

	sf::Clock m_clock;
	float m_dt;
	
	sf::Font m_font;
};
