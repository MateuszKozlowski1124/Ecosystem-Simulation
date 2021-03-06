#include "MovementComponent.h"

// TODO: unhardcode that
MovementComponent::MovementComponent()
	: m_brain(std::make_unique<Blueberry::Brain>(7U, 3U))
	, m_prevVelocity(0, 0)
	, m_velocity(0, 0)
	, m_acceleration(0.0f, 0.0f)
{
	for (int i = 0; i < 3; i++)
	{
		m_brain->mutateRandomNeuronBias();
	}
}

MovementComponent::MovementComponent(
	const sf::Vector2i& defaultVelocity,
	const char* brainFilePath)
	: m_brain(std::make_unique<Blueberry::Brain>(0U, 0U))
	, m_prevVelocity(0, 0)
	, m_velocity(defaultVelocity)
	, m_acceleration(0.0f, 0.0f)
{
	sf::Vector2i velocity = defaultVelocity;

	velocityGuard();

	loadBrainFromFile(brainFilePath);
}

MovementComponent::MovementComponent(const MovementComponent& rhs)
	: m_brain(std::make_unique<Blueberry::Brain>(0U, 0U))
	, m_prevVelocity(0, 0)
	, m_velocity(rhs.m_velocity)
	, m_acceleration(rhs.m_acceleration)
{
	sf::Vector2i velocity = rhs.m_velocity;

	velocityGuard();

	*m_brain = *rhs.m_brain;
}

MovementComponent& MovementComponent::operator=(const MovementComponent& rhs)
{
	sf::Vector2i velocity = rhs.m_velocity;

	velocityGuard();

	if (this != &rhs)
	{
		*m_brain = *rhs.m_brain;
		m_prevVelocity = rhs.m_prevVelocity;
		m_velocity = rhs.m_velocity;
		m_acceleration = rhs.m_acceleration;
	}

	return *this;
}

void MovementComponent::saveBrainToFile(const char* filePath) const
{
	m_brain->saveToFile(filePath);
}

void MovementComponent::loadBrainFromFile(const char* filePath)
{
	m_brain->loadFromFile(filePath);
}

void MovementComponent::update(
	float dt,
	unsigned availableEnergy,
	float speedFactor,
	const std::vector<Blueberry::Scalar>& brainInputs,
	bool allowUserInput,
	std::ofstream& debugFile,
	const std::unordered_map<std::string, int>& keybinds)
{
	// TODO: implement slowing down using speed_factor:
	
	updateAcceleration(brainInputs, allowUserInput, debugFile, keybinds);

	// updating previous velocity has to take place 
	// before returning* the function if acceleration is impossible;
	// this caused a bug
	m_prevVelocity = m_velocity;

	if (accelerationIsImpossible(dt, availableEnergy))
	{
		std::clog << "acceleration impossible; fun fact: a=" << m_acceleration.x << ' ' << m_acceleration.y << '\n';
		return; // *
	}

	m_velocity.x += static_cast<int>(m_acceleration.x);
	m_velocity.y += static_cast<int>(m_acceleration.y);

	velocityGuard();
}

// accessors:

const Blueberry::Brain& MovementComponent::getBrain() const
{
	return *m_brain;
}

unsigned MovementComponent::getEnergyToExpel() const
{
	if (getKineticEnergyDelta() >= 0) return 0;

	//std::cout << "THERE IS SOME ENERGY TO EXPEL!\n";

	if (getKineticEnergyDelta() > 4'000'000'000)
	{
		std::cout << "UNSIGNED BUG FROM getKineticEnergyDelta() const!\n";
	}

	// hp change = kin E change and both of them are now smaller so the energy has to be expeled
	// for example as a new food that the can be eaten by an animal to which the MovementComponent belongs
	return -2 * getKineticEnergyDelta();
}

int MovementComponent::getKineticEnergyDelta() const
{
	return static_cast<int>(getKineticEnergy()) - static_cast<int>(getPreviousKineticEnergy());
}

unsigned MovementComponent::getPreviousKineticEnergy() const
{
	return getPreviousVelocityVectorSquaredValue();
}

unsigned MovementComponent::getKineticEnergy() const
{
	return getVelocityVectorSquaredValue();
}

unsigned MovementComponent::getPreviousVelocityVectorSquaredValue() const
{
	return getVectorSquaredValue(m_prevVelocity, false, "prev vel", m_acceleration);
}

unsigned MovementComponent::getVelocityVectorSquaredValue() const
{
	return getVectorSquaredValue(m_velocity, false, "vel", m_acceleration);
}

float MovementComponent::getAccelerationVectorValue() const
{
	return getVectorValue(m_acceleration);
}

const sf::Vector2i& MovementComponent::getPreviousVelocityVector() const
{
	return m_prevVelocity;
}

const sf::Vector2i& MovementComponent::getVelocityVector() const
{
	return m_velocity;
}

const sf::Vector2f& MovementComponent::getAccelerationVector() const
{
	return m_acceleration;
}

// mutators:

void MovementComponent::mutateBrain(unsigned brainMutationsCount)
{
	if (m_brain->getEnabledSynapsesCount() < 8 || m_brain->getEnabledNeuronsCount() < 1U)
	{
		anabolicBrainMutation(brainMutationsCount);
	}
	else
	{
		m_brain->mutate(brainMutationsCount);
	}
}

/*
void MovementComponent::setVelocity(const sf::Vector2i& velocity)
{
	if (velocity.x > 2'000'000'000 || velocity.y > 2'000'000'000)
	{
		std::cerr << "WEIRD VEL1: (" << velocity.x << ", " << velocity.y << ")\n";
		exit(-13);
	}

	setVelocity(velocity.x, velocity.y);
}

void MovementComponent::setVelocity(int x, int y)
{
	if (x > 2'000'000'000 || y > 2'000'000'000)
	{
		std::cerr << "WEIRD VEL2: (" << x << ", " << y << ")\n";
		exit(-13);
	}

	//m_prevVelocity = m_velocity;
	m_velocity.x = x;
	m_velocity.y = y;
}

void MovementComponent::setVelocityX(int vx)
{
	if (vx > 2'000'000'000)
	{
		std::cerr << "WEIRD VELOCITY X!\n";
		exit(-13);
	}

	//m_prevVelocity = m_velocity;
	m_velocity.x = vx;
}

void MovementComponent::setVelocityY(int vy)
{
	if (vy > 2'000'000'000)
	{
		std::cerr << "WEIRD VELOCITY Y!\n";
		exit(-13);
	}

	//m_prevVelocity = m_velocity;
	m_velocity.y = vy;
}
*/

void MovementComponent::elasticReboundInAxisX()
{
	m_velocity.x *= -1;
}

void MovementComponent::elasticReboundInAxisY()
{
	m_velocity.y *= -1;
}

void MovementComponent::setVelocitiesLoadedFromFile(
	const sf::Vector2i& velocity, 
	const sf::Vector2i& prevVel)
{
	m_velocity = velocity;
	m_prevVelocity = prevVel;
}

// private methods:

void MovementComponent::resetVelocity()
{
	m_velocity = { 0, 0 };
}

void MovementComponent::updateAcceleration(
	const std::vector<Blueberry::Scalar>& brainInputs,
	bool allowUserInput,
	std::ofstream& debugFile,
	const std::unordered_map<std::string, int>& keybinds
)
{
	m_brain->propagateForward(brainInputs);

	const std::vector<Blueberry::Scalar>& brainOutput = m_brain->getOutput();

	m_acceleration.x = 10.0 * brainOutput[0];
	m_acceleration.y = 10.0 * brainOutput[1];

	if (m_acceleration.x > 500'000 || m_acceleration.y > 500'000)
	{
		std::clog << "a: " << m_acceleration.x << ' ' << m_acceleration.y << '\n';
		std::clog << "inputs:\n";
		for (int i = 0; i < m_brain->getInputSize(); i++)
		{
			std::clog << m_brain->getNeurons()[i].getVal() << ' ' << m_brain->getNeurons()[i].getActVal() << '\n';
		}
	}

	if (allowUserInput)
	{
		handleUserInput(keybinds);
	}
}

void MovementComponent::handleUserInput(const std::unordered_map<std::string, int>& keybinds)
{
	bool left  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key(keybinds.at("LEFT")));
	bool right = sf::Keyboard::isKeyPressed(sf::Keyboard::Key(keybinds.at("RIGHT")));
	bool up    = sf::Keyboard::isKeyPressed(sf::Keyboard::Key(keybinds.at("UP")));
	bool down  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key(keybinds.at("DOWN")));

	if (left || right || down || up)
	{
		m_acceleration = sf::Vector2f(0.0f, 0.0f);

		// TODO: unhardcode:
		float userInputForce = 16.0f;

		if (left)
		{
			m_acceleration.x -= userInputForce;
		}
		if (right)
		{
			m_acceleration.x += userInputForce;
		}
		if (up)
		{
			m_acceleration.y -= userInputForce;
		}
		if (down)
		{
			m_acceleration.y += userInputForce;
		}
	}
}

bool MovementComponent::accelerationIsImpossible(
	float dt, 
	unsigned availableEnergy)
{
	// previous and wrong approach to solve the problem:
	// check if hp will be greater than 0:
	//const sf::Vector2f velVectDelta{
	//	m_acceleration.x * dt,
	//	m_acceleration.y * dt
	//};
	//const sf::Vector2f newVelVect = m_velocity + velVectDelta;
	//
	//const float newVelVectVal = getVectorValue(newVelVect);
	//const double newKinEnergy = 0.5 * std::pow(newVelVectVal, 2.0);
	//
	//std::cout << availableEnergy << " vs " << abs(newKinEnergy - getKineticEnergy()) << '\n';
	//
	//return availableEnergy >= abs(newKinEnergy - getKineticEnergy());

	// check if total energy will be greater than 0:

	// it used to be like this but I want it to be independable from FPS count:
	/*
	const sf::Vector2f velVectDelta{
		m_acceleration.x * dt,
		m_acceleration.y * dt
	};
	*/
	const sf::Vector2i velVectDelta{
		static_cast<int>(m_acceleration.x),
		static_cast<int>(m_acceleration.y)
	};
	
	const sf::Vector2i newVelVect = m_velocity + velVectDelta;
	
	int newKinEnergy = getVectorSquaredValue(newVelVect, true, "impossible a", m_acceleration);

	int newHp = static_cast<int>(availableEnergy) - abs(
		newKinEnergy - static_cast<int>(getKineticEnergy())
	);

	int newTotalEnergy = newKinEnergy + newHp;

	return newTotalEnergy < 0;
}

float MovementComponent::getVectorValue(const sf::Vector2f& vector)
{
	return sqrt(std::pow(vector.x, 2.0f) + std::pow(vector.y, 2.0f));
}

float MovementComponent::getVectorSquaredValue(
	const sf::Vector2i& vector,
	bool simulation,
	std::string origin,
	const sf::Vector2f& acceleration)
{
	if (simulation == false && (vector.x > 50'000 || vector.y > 50'000))
	{
		std::cerr 
			<< "origin: " << origin << '\n'
			<< "a: " << acceleration.x << ' ' << acceleration.y << '\n'
			<< "ERROR: MovementComponent::getVectorSquaredValue(...): "
			<< "very big velocity!: " << vector.x << ' ' << vector.y << '\n';
		exit(-13);
	}

	return vector.x * vector.x + vector.y * vector.y;
}

void MovementComponent::velocityGuard() const
{
	if (m_velocity.x > 2'000'000'000 || m_velocity.y > 2'000'000'000)
	{
		std::cerr << "WEIRD VELOCITY: (" << m_velocity.x << ", " << m_velocity.y << ")\n";
		std::cerr << "PREVIOUS VELOCITY: (" << m_prevVelocity.x << ' ' << m_prevVelocity.y << ")\n";
		std::cerr << "ACCELERATION: (" << m_acceleration.x << ' ' << m_acceleration.y << ")\n";
		exit(-13);
	}
}

void MovementComponent::anabolicBrainMutation(unsigned brainMutationsCount)
{
	for (int i = 0; i < brainMutationsCount; i++)
	{
		// 33% probability for a new neuron and 67% probability for a new synapse:
		if (Blueberry::RandomEngine::getIntInRange(0, 2) == 0)
		{
			m_brain->addRandomNeuron();
		}
		else
		{
			m_brain->addRandomSynapse();
		}
	}
}
