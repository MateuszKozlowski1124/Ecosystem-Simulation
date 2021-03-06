#pragma once

#include "BrainPreview.h"

namespace gui
{
	class BrainPreviewModifier
	{
	public:
		BrainPreviewModifier(
			const sf::Vector2f& pos,
			const sf::Vector2f& size
		);

		void update(
			gui::BrainPreview& brainPreview,
			const sf::Vector2f& mousePos,
			const std::vector<sf::Event>& events
		);
		void render(
			gui::BrainPreview& brainPreview,
			sf::RenderTarget& target
		);

		// accessors:

		sf::FloatRect getBgBounds() const;

		const std::unique_ptr<gui::ImageButton>& getCloseBtn() const;

	private:
		// initialization:

		void initCloseBtn();

		// utils:

		void updateNeuronsPos(
			gui::BrainPreview& brainPreview,
			const sf::Vector2f& mousePos,
			const std::vector<sf::Event>& events
		);
		void handleEvents(
			gui::BrainPreview& brainPreview,
			const sf::Vector2f& mousePos,
			const std::vector<sf::Event>& events
		);
		bool aNeuronIsBeingModified() const;

		bool neuronIsHovered(
			const sf::CircleShape& neuron,
			const sf::Vector2f& mousePos
		) const;

		void updateCloseBtn(
			const sf::Vector2f& mousePos,
			const std::vector<sf::Event>& events
		);
		void getUpdatesFromImgBtn();

	private:
		gui::BrainPreview* m_previouslyModifiedBrainPreview;

		int m_modifiedNeuronIdx;

		const sf::Vector2f m_pos;
		const sf::Vector2f m_size;

		std::unique_ptr<gui::ImageButton> m_closeBtn;
	};
}
