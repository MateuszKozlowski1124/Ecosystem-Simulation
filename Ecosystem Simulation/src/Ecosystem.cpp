#include "stdafx.h"
#include "Ecosystem.h"

// static variables:
std::string Ecosystem::templateConfigFilePath = "resources/template files/ecosystem initializator.ini";
std::string Ecosystem::configFileName = "ecosystem initializator.ini";
std::string Ecosystem::fruitsFileName = "fruits.ini";

// public static methods:
void Ecosystem::createConfigFile(const std::string& folder_path)
{
	std::filesystem::create_directories(folder_path);

	std::ifstream src(Ecosystem::templateConfigFilePath, std::ios::binary);
	std::ofstream dst(folder_path + "/" + Ecosystem::configFileName, std::ios::binary);

	if (!src.is_open()) throw("ERROR::CANNOT OPEN A FILE: " + Ecosystem::templateConfigFilePath);
	if (!dst.is_open()) throw("ERROR::CANNOT OPEN A FILE: " + folder_path + '/' + Ecosystem::configFileName);

	dst << src.rdbuf(); // copy from template file

	dst.close();
}

void Ecosystem::setUpEcosystemFolder(const std::string& folder_path)
{
	// read data from a config file:
	std::ifstream file(folder_path + '/' + Ecosystem::configFileName);

	if (!file.is_open()) throw("ERROR::ECOSYSTEM::CANNOT OPEN A FILE: " + folder_path + '/' + Ecosystem::configFileName);

	std::string temp;
	sf::Vector2f worldSize;
	float borderThickness;
	unsigned animalsCount = 0U, fruitsCount = 0U;

	file >> temp >> worldSize.x;
	file >> temp >> worldSize.y;
	file >> temp >> borderThickness;
	file >> temp >> animalsCount;
	file >> temp >> fruitsCount;

	file.close();

	// create folders for animals:
	for (int i = 0; i < animalsCount; i++)
		Animal::setUpAnimalFolder(folder_path + '/' + "animal" + std::to_string(i));

	// fruits file:
	std::ofstream fruitsFile(folder_path + '/' + Ecosystem::fruitsFileName);

	if (!fruitsFile.is_open()) throw("ERROR::ECOSYSTEM::CANNOT OPEN A FILE: " + folder_path + '/' + Ecosystem::fruitsFileName);

	std::stringstream ss;

	for (float i = 0.f; i < fruitsCount; i++) 
	{
		Fruit f(10e2);
		f.setRandomPos(worldSize, borderThickness);
		ss << f.getPosition().x << ' ' << f.getPosition().y << '\n';
	}

	fruitsFile << ss.str();

	fruitsFile.close();
}

// constructor/destructor:
Ecosystem::Ecosystem()
	: borderThickness(0U), 
	  animalsCount(0U), fruitsCount(0U),
	  trackedAnimal(nullptr),
	  totalTimeElapsed(0.f), dtSinceLastWorldUpdate(0.f),
	  initialized(false)
{
	
}

Ecosystem::~Ecosystem()
{
	for (auto& animal : this->animals) delete animal;
	
	for (auto& Fruit : this->fruits) delete Fruit;
}

// initialization:
void Ecosystem::loadFromFolder(const std::string& folder_path)
{
	std::cout << "LOADING AN ECOSYSTEM FROM FOLDER LOGS:\n";

	this->directoryPath = folder_path;
	
	this->readDataFromConfigFile(folder_path);
	this->initBorders();
	this->initBackground();
	this->initAnimals(folder_path);
	this->initFruits(folder_path);
	
	this->initialized = true;

	// TODO: rmv later!:
	std::cout << "AN ECOSYSTEM HAS BEEN LOADED CORRECTLY\n";
}

// accessors:
const sf::Vector2f& Ecosystem::getWorldSize() const
{
	return this->worldSize;
}

const std::string& Ecosystem::getDirectoryPath() const
{
	return this->directoryPath;
}

unsigned Ecosystem::getBorderThickness() const
{
	return this->borderThickness;
}

void Ecosystem::printAnimalsPositions() const
{
	std::cout << "Animal positions:\n";
	for (const auto& animal : this->animals)
		std::cout << animal->getPosition().x << ' ' << animal->getPosition().y << '\n';
}

const sf::Vector2f* Ecosystem::getTrackedAnimalPosition() const
{
	if (this->trackedAnimal) return &this->trackedAnimal->getPosition();
	
	return nullptr;
}

float Ecosystem::getTotalTimeElapsed() const
{
	return this->totalTimeElapsed;
}

bool Ecosystem::isInitialized() const
{
	return this->initialized;
}

// other public methods:
void Ecosystem::update(
	float dt,
	float speed_factor,
	const std::vector<sf::Event>& events,
	const sf::Vector2f& mouse_pos_view, 
	bool paused,
	const std::string& god_tool)
{
	this->totalTimeElapsed += dt;
	
	this->useGodTool(events, mouse_pos_view, god_tool);

	if (!paused) this->updateWorld(dt, speed_factor);
}

void Ecosystem::render(sf::RenderTarget& target)
{
	target.draw(this->border);
	target.draw(this->background);

	for (const auto& animal : this->animals) animal->renderBody(target);

	for (const auto& animal : this->animals)
		if (animal->isHpBarRendered())
			animal->renderHpBar(target);

	for (const auto& Fruit : this->fruits) Fruit->render(target);

	for (const auto& animal : this->animals)
		if (animal->isBrainRendered())
			animal->renderBrainPreview(target);
}

// private initialization:
void Ecosystem::readDataFromConfigFile(const std::string& folder_path)
{
	std::ifstream file(folder_path + '/' + Ecosystem::configFileName);

	if (!file.is_open())
	{
		std::cerr << "ERROR::ECOSYSTEM::CANNOT OPEN FILE: " + folder_path + '/' + Ecosystem::configFileName;
		exit(-1);
	}

	std::string temp;
	unsigned animalsCount = 0U, fruitsCount = 0U;

	file >> temp >> this->worldSize.x;
	file >> temp >> this->worldSize.y;
	file >> temp >> this->borderThickness;
	file >> temp >> this->animalsCount;
	file >> temp >> this->fruitsCount;

	file.close();

	// TODO: rmv later!:
	std::cout << "READING FROM A FILE IS DONE\n";
}

void Ecosystem::initBorders()
{
	this->border.setFillColor(sf::Color(48, 48, 48));
	this->border.setSize(this->worldSize);
}

void Ecosystem::initBackground()
{
	this->background.setFillColor(sf::Color(32, 32, 32));
	this->background.setSize(
		sf::Vector2f(
			this->worldSize.x - 2.f * this->borderThickness,
			this->worldSize.y - 2.f * this->borderThickness
		)
	);
	this->background.setPosition(this->borderThickness, this->borderThickness);

	// TODO: rmv later!:
	std::cout << "BACKGROUND IS DONE!\n";
}

void Ecosystem::initAnimals(const std::string& folder_path)
{
	for (int i = 0; i < this->animalsCount; i++)
	{
		this->animals.push_back(new Animal(10e6, 10e6, false, false));
		this->animals[i]->loadFromFolder(folder_path + '/' + "animal" + std::to_string(i));
	}

	// TODO: rmv later!:
	std::cout << "ANIMALS'RE DONE\n";
}

void Ecosystem::initFruits(const std::string& folder_path)
{
	// TODO: add static Fruit name file
	std::ifstream file1(folder_path + '/' + "fruits.ini");

	if (!file1.is_open())
	{
		std::cerr << "ERROR::ECOSYSTEM::CANNOT OPEN FILE: " + folder_path + '/' + Ecosystem::fruitsFileName;
		exit(-1);
	}

	this->fruits.reserve(this->fruitsCount);

	std::cout << "FRUITS RESERVATION IS DONE!\n";

	for (int i = 0; i < this->fruitsCount; i++)
	{
		float x, y;

		file1 >> x;
		file1 >> y;
		
		this->fruits.push_back(new Fruit(10e6)); // TODO: rmv that hardcoded variable!
		this->fruits.back()->setPosition(x, y);
	}

	std::cout << "FRUITS'RE DONE!\n";
}

// private methods:
// private utilities:
std::vector<CrappyNeuralNets::Scalar> Ecosystem::getInputsForBrain(const Animal& animal) const
{
	std::vector<CrappyNeuralNets::Scalar> inputsForBrain;
	
	inputsForBrain.reserve(5);

	inputsForBrain.push_back(animal.getVelocity().x);
	inputsForBrain.push_back(animal.getVelocity().y);
	
	inputsForBrain.push_back(log2(animal.getHp()));

	Fruit* theNearestFruit = findTheNearestFruit(animal);
	
	inputsForBrain.push_back(theNearestFruit->getPosition().x - animal.getPosition().x);
	inputsForBrain.push_back(theNearestFruit->getPosition().y - animal.getPosition().y);

	return inputsForBrain;
}

Fruit* Ecosystem::findTheNearestFruit(const Animal& animal) const
{
	Fruit* theNearestFruit = this->fruits[0];
	float theSmallestDistance = INFINITY;

	for (const auto& it : this->fruits)
	{
		float acceleration = animal.getPosition().x - it->getPosition().x;
		float b = animal.getPosition().y - it->getPosition().y;

		float distance = sqrt(pow(acceleration, 2) + pow(b, 2));

		if (distance < theSmallestDistance)
		{
			theSmallestDistance = distance;
			theNearestFruit = it;
		}
	}
	
	return theNearestFruit;
}

void Ecosystem::useGodTool(
	const std::vector<sf::Event>& events,
	const sf::Vector2f& mouse_pos_view, 
	const std::string& god_tool)
{
	// TODO: add info that if there is no God tool then god_tool string should be equal to ""

	if (!EventsAccessor::hasEventOccured(static_cast<sf::Event::EventType>(sf::Event::MouseButtonPressed), events))
		return;

	// now use God tool:
	if (god_tool == "TRACK") this->trackingTool(mouse_pos_view);

	else if (god_tool == "KILL") this->killingTool(mouse_pos_view);

	else if (god_tool == "REPLACE") this->replacingTool(mouse_pos_view);

	else if (god_tool == "BRAIN") this->brainTool(mouse_pos_view);

	else if (god_tool == "STOP") this->stoppingTool(mouse_pos_view);

	else if (god_tool == "INFO") this->infoTool(mouse_pos_view);

	else if (god_tool == "") return;

	else
	{
		std::cerr << "INCORRECT GOD TOOL: " << god_tool << '\n';
		exit(-1);
	}
}

void Ecosystem::updateWorld(float dt, float speed_factor)
{
	for (const auto& animal : this->animals)
		animal->update(dt, speed_factor, getInputsForBrain(*animal));

	this->removeDeadAnimals();

	this->avoidGoingBeyondTheWorld();

	this->feedAnimals();

	this->removeEatenFruit();
}

void Ecosystem::removeDeadAnimals()
{
	for (int i = 0; i < this->animals.size();)
	{
		if (!this->animals[i]->isAlive())
		{
			this->convertKineticEnergyToFruit(this->animals[i], false);
			this->removeAnimal(this->animals[i]);
		}

		else i++;
	}
}

void Ecosystem::convertKineticEnergyToFruit(Animal* animal, bool random_pos)
{
	this->fruits.push_back(new Fruit(animal->getKineticEnergy()));
	
	if (random_pos)
		this->fruits.back()->setRandomPos(this->worldSize, this->borderThickness);
	else
		this->fruits.back()->setPosition(animal->getPosition());

	animal->setVelocity(sf::Vector2f(0.0f, 0.0f));
}

void Ecosystem::removeAnimal(Animal*& animal)
{
	if (animal == this->trackedAnimal) this->trackedAnimal = nullptr;

	delete animal;
	std::swap(animal, this->animals.back());
	this->animals.pop_back();
}

void Ecosystem::avoidGoingBeyondTheWorld()
{
	// avoid going beyond the world:
	for (auto& animal : this->animals)
	{
		// left border:
		if (animal->getPosition().x - animal->getRadius() < this->borderThickness)
		{
			animal->setVelocity({ -animal->getVelocity().x, animal->getVelocity().y });
			animal->setPosition(sf::Vector2f(this->borderThickness + animal->getRadius(), animal->getPosition().y));
		}

		// right border:
		else if (animal->getPosition().x + animal->getRadius() > this->worldSize.x - this->borderThickness)
		{
			animal->setVelocity({ -animal->getVelocity().x, animal->getVelocity().y });
			animal->setPosition(sf::Vector2f(this->worldSize.x - this->borderThickness - animal->getRadius(), animal->getPosition().y));
		}

		// it is possible that an animal crossed 2 perpendicular borders in the same frame, so we don't use else if here:
		
		// top border:
		if (animal->getPosition().y - animal->getRadius() < this->borderThickness)
		{
			animal->setVelocity({ animal->getVelocity().x, -animal->getVelocity().y });
			animal->setPosition(sf::Vector2f(animal->getPosition().x, this->borderThickness + animal->getRadius()));
		}

		// bottom border:
		else if (animal->getPosition().y + animal->getRadius() > this->worldSize.y - this->borderThickness)
		{
			animal->setVelocity({ animal->getVelocity().x, -animal->getVelocity().y });
			animal->setPosition(sf::Vector2f(animal->getPosition().x, this->worldSize.y - this->borderThickness - animal->getRadius()));
		}
	}
}

void Ecosystem::feedAnimals()
{
	std::sort(this->animals.begin(), this->animals.end(), compareAnimalsYPositions);

	for (int f = 0; f < this->fruits.size(); f++)
	{
		unsigned left = 0, right = this->animals.size() - 1U;

		while (left < right)
		{
			unsigned center = (left + right) / 2;

			if (this->animalIsToHigh(*this->animals[center], *this->fruits[f]))
			{
				left = center + 1;
			}
			else
			{
				right = center;
			}
		}

		this->tryToFindConsumer(*this->fruits[f], left);
	}
}

bool Ecosystem::compareAnimalsYPositions(const Animal* a1, const Animal* a2)
{
	return a1->getPosition().y < a2->getPosition().y;
}

bool Ecosystem::animalIsToHigh(const Animal& animal, const Fruit& Fruit)
{
	return (animal.getPosition().y - Fruit.getPosition().y) < -(animal.getRadius() + Fruit.getRadius());
}

bool Ecosystem::animalReachesFruitInY(const Animal& animal, const Fruit& Fruit)
{
	if (abs(animal.getPosition().y - Fruit.getPosition().y) <= animal.getRadius() + Fruit.getRadius()) return true;
	
	return false;
}

bool Ecosystem::animalReachesFruit(const Animal& animal, const Fruit& Fruit)
{
	float acceleration = animal.getPosition().x - Fruit.getPosition().x;
	float b = animal.getPosition().y - Fruit.getPosition().y;

	float distance = sqrt(pow(acceleration, 2) + pow(b, 2));

	if (distance <= animal.getRadius() + Fruit.getRadius()) return true;

	return false;
}

int Ecosystem::tryToFindConsumer(Fruit& fruit, unsigned start_animal_index)
{
	unsigned animal_index = start_animal_index;

	while (animal_index < this->animals.size() && this->animalReachesFruitInY(*this->animals[animal_index], fruit))
	{
		if (this->animalReachesFruit(*this->animals[animal_index], fruit))
		{
			this->eat(*this->animals[animal_index], fruit);
			return 1;
		}
		animal_index++;
	}

	return 0;
}

void Ecosystem::eat(Animal& animal, Fruit& fruit)
{	
	if (fruit.getEnergy() + animal.getHp() > animal.getMaxHp())
	{
		fruit.setEnergy(fruit.getEnergy() + animal.getHp() - animal.getMaxHp());
		animal.setHp(animal.getMaxHp());

		// clone using energy surplus!:
		while (fruit.getEnergy() > animal.getMaxHp())
		{
			fruit.setEnergy(fruit.getEnergy() - animal.getMaxHp());

			this->animals.push_back(new Animal(0.f, 0.f, true, true));
			this->animals.back()->copyConstructor(animal);
			this->animals.back()->setHp(animal.getMaxHp());
			this->animals.back()->setVelocity(sf::Vector2f(0.f, 0.f));
			this->animals.back()->randomMutate(50.0);
		}

		this->animals.push_back(new Animal(0.f, 0.f, true, true));
		this->animals.back()->copyConstructor(animal);
		this->animals.back()->setHp(fruit.getEnergy());
		this->animals.back()->setVelocity(sf::Vector2f(0.f, 0.f));
		this->animals.back()->randomMutate(50.0);
	}
	else
	{
		animal.increaseHp(fruit.getEnergy());
	}
	
	fruit.setEnergy(0.f);
}

void Ecosystem::removeEatenFruit()
{
	for (int i = 0; i < this->fruits.size(); i++)
		if (this->fruits[i]->getEnergy() == 0.f)
		{
			delete this->fruits[i];
			std::swap(this->fruits[i], this->fruits.back());
			this->fruits.pop_back();
		}
}

void Ecosystem::convertAnimalToFruit(Animal*& animal)
{
	this->fruits.push_back(new Fruit(animal->getHp() + animal->getKineticEnergy()));
	this->fruits.back()->setPosition(animal->getPosition());

	if (animal == this->trackedAnimal) this->trackedAnimal = nullptr;

	this->removeAnimal(animal);
}

// God tools:
void Ecosystem::trackingTool(const sf::Vector2f& mouse_pos_view)
{
	for (auto& animal : this->animals)
	{
		if (animal->isCovered(mouse_pos_view))
		{
			// the tracked animal is no longer tracked:
			if (animal == this->trackedAnimal)
			{
				animal->setColor(sf::Color::Red); // reset its color
				this->trackedAnimal = nullptr;
				return;
			}
			
			// a new animal is tracked:
			// reset body color of the previous tracked animal:
			if (this->trackedAnimal) this->trackedAnimal->setColor(sf::Color::Red);
			
			// set new tracked animal:
			this->trackedAnimal = animal;
			
			// change color:
			animal->setColor(sf::Color(150, 0, 200));

			return;
		}
	}
}

void Ecosystem::killingTool(const sf::Vector2f& mouse_pos_view)
{
	// animals:
	for (auto& animal : this->animals)
	{
		if (animal->isCovered(mouse_pos_view))
		{
			this->convertAnimalToFruit(animal);
			break;
		}
	}
}

void Ecosystem::replacingTool(const sf::Vector2f& mouse_pos_view)
{
	std::cout << "REPLACING TOOL IS NOT DEFINED YET!\n";
}

void Ecosystem::brainTool(const sf::Vector2f& mouse_pos_view)
{
	for (auto& animal : this->animals)
	{
		if (animal->isCovered(mouse_pos_view))
		{
			animal->setBrainIsRendered(!animal->isBrainRendered());
			return;
		}
	}
}

void Ecosystem::stoppingTool(const sf::Vector2f& mouse_pos_view)
{
	for (auto& animal : this->animals)
	{
		if (animal->isCovered(mouse_pos_view))
		{
			this->convertKineticEnergyToFruit(animal, true);
			return;
		}
	}
}

void Ecosystem::infoTool(const sf::Vector2f& mouse_pos_view)
{
	std::cout << "INFO TOOL ISN'T DEFINED YET!\n";
}
