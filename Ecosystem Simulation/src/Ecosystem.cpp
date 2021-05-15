#include "stdafx.h"
#include "Ecosystem.h"

// static variables:
std::string Ecosystem::templateConfigFilePath = "resources/template files/ecosystem initializator.ini";
std::string Ecosystem::configFileName = "ecosystem initializator.ini";
std::string Ecosystem::foodFileName = "food.ini";

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
	unsigned animalsCount = 0U, foodCount = 0U;

	file >> temp >> worldSize.x;
	file >> temp >> worldSize.y;
	file >> temp >> borderThickness;
	file >> temp >> animalsCount;
	file >> temp >> foodCount;

	file.close();

	// create folders for animals:
	for (int i = 0; i < animalsCount; i++)
		Animal::setUpAnimalFolder(folder_path + '/' + "animal" + std::to_string(i));

	// food file:
	std::ofstream foodFile(folder_path + '/' + Ecosystem::foodFileName);

	if (!foodFile.is_open()) throw("ERROR::ECOSYSTEM::CANNOT OPEN A FILE: " + folder_path + '/' + Ecosystem::foodFileName);

	std::stringstream ss;
	
	CrappyNeuralNets::RandomNumbersGenerator generator;

	for (float i = 0.f; i < foodCount; i++) 
	{
		Food f;
		f.setRandomPos(worldSize, borderThickness, generator);
		ss << f.getPosition().x << ' ' << f.getPosition().y << '\n';
	}

	foodFile << ss.str();

	foodFile.close();
}

// constructor/destructor:
Ecosystem::Ecosystem()
	: directoryPath("ECOSYSTEM IS NOT INITIALIZED"), borderThickness(0U)
{
	
}

Ecosystem::~Ecosystem()
{
	for (auto& animal : this->animals) delete animal;
	
	for (auto& food : this->food) delete food;
}

// initialization:
void Ecosystem::loadFromFolder(const std::string& folder_path)
{
	this->directoryPath = folder_path;
	
	// read data from config file:
	std::ifstream file(folder_path + '/' + Ecosystem::configFileName);

	if (!file.is_open()) std::cerr << "ERROR::ECOSYSTEM::CANNOT OPEN FILE: " + folder_path + '/' + Ecosystem::configFileName;

	std::string temp;
	unsigned animalsCount = 0U, foodCount = 0U;

	file >> temp >> this->worldSize.x;
	file >> temp >> this->worldSize.y;
	file >> temp >> this->borderThickness;
	file >> temp >> animalsCount;
	file >> temp >> foodCount;

	file.close();

	// borders and backgroundRect:
	this->border.setFillColor(sf::Color(32, 32, 32));
	this->border.setSize(this->worldSize);

	this->background.setFillColor(sf::Color::Black);
	this->background.setSize(
		sf::Vector2f(
			this->worldSize.x - 2.f * this->borderThickness,
			this->worldSize.y - 2.f * this->borderThickness
		)
	);
	this->background.setPosition(this->borderThickness, this->borderThickness);

	// animals:
	for (int i = 0; i < animalsCount; i++)
	{	
		this->animals.push_back(new Animal());
		this->animals[i]->loadFromFolder(folder_path + '/' + "animal" + std::to_string(i));
	}

	// food:
	// TODO: add static food name file
	std::ifstream file1(folder_path + '/' + "food.ini");

	if (!file1.is_open()) std::cerr << "ERROR::ECOSYSTEM::CANNOT OPEN FILE: " + folder_path + '/' + Ecosystem::configFileName;

	this->food.reserve(foodCount);

	float x, y;

	for (int i = 0; i < foodCount; i++)
	{
		file1 >> x;
		file1 >> y;
		this->food.push_back(new Food());
		this->food.back()->setPos(x, y);
	}
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

// other public methods:
void Ecosystem::update(float dt, const std::vector<sf::Event>& events, const sf::Vector2f& mousePosView)
{
	for (const auto& animal : this->animals) animal->updateBodyAndHp(dt, getInputsForBrain(*animal));

	// avoid going beyond the world:
	for (auto& animal : this->animals)
	{
		bool crossedLeftBorder = animal->getPos().x < this->borderThickness + animal->getRadius();
		bool crossedRightBorder = animal->getPos().x > this->worldSize.x - this->borderThickness - animal->getRadius();

		if (crossedLeftBorder || crossedRightBorder)
			animal->setVelocity({ -animal->getVelocity().x, animal->getVelocity().y });
		
		bool crossedTopBorder = animal->getPos().y < this->borderThickness + animal->getRadius();
		bool crossedBottomBorder = animal->getPos().y > this->worldSize.y - this->borderThickness - animal->getRadius();

		if (crossedTopBorder || crossedBottomBorder)
			animal->setVelocity({ animal->getVelocity().x, -animal->getVelocity().y });
	}

	// eat food!:
	// TODO: come up with a new way of generating random numbers:
	CrappyNeuralNets::RandomNumbersGenerator generator;

	for (int i=0; i<this->animals.size(); i++)
	{
		for (int j=0; j<this->food.size(); j++)
		{
			float a = this->animals[i]->getPos().x - this->food[j]->getPosition().x;
			float b = this->animals[i]->getPos().y - this->food[j]->getPosition().y;

			float distance = sqrt(pow(a, 2) + pow(b, 2));

			if (distance <= this->animals[i]->getRadius() + this->food[j]->getRadius())
			{
				this->animals[i]->setHp(1000.f);
				this->food[j]->setRandomPos(this->worldSize, this->borderThickness, generator);
			}
		}
	}

	// kill animals that aren't alive (even if this sentense doesn't make sense ;)) 
	for (int i = 0; i < this->animals.size(); i++)
		if (!this->animals[i]->isAlive())
		{
			delete this->animals[i];
			std::swap(this->animals[i], this->animals.back());
			this->animals.pop_back();
		}

	// showing brain:
	bool temp = false;

	for (const auto& event : events)
		if (event.type == sf::Event::MouseButtonPressed)
		{
			temp = true;
			break;
		}

	if (temp)
		for (auto& animal : this->animals)
		{
			float a = animal->getPos().x - mousePosView.x;
			float b = animal->getPos().y - mousePosView.y;

			float distance = sqrt(pow(a, 2) + pow(b, 2));

			if (animal->getRadius() >= distance) animal->setBrainIsRendered(!animal->isBrainRendered());
		}

	for (auto& animal : this->animals)
		if (animal->isBrainRendered())
			animal->updateBrainPreview();
}

void Ecosystem::render(sf::RenderTarget& target)
{
	target.draw(this->border);
	target.draw(this->background);

	for (const auto& food : this->food) food->render(target);

	for (const auto& animal : this->animals) animal->renderBody(target);
	
	for (const auto& animal : this->animals) animal->renderHpBar(target);
	
	for (const auto& animal : this->animals)
		if (animal->isBrainRendered())
			animal->renderBrain(target);
}

// private methods:
// private utilities:
std::vector<CrappyNeuralNets::Scalar> Ecosystem::getInputsForBrain(const Animal& animal)
{
	std::vector<CrappyNeuralNets::Scalar> inputsForBrain;
	
	inputsForBrain.reserve(5);

	inputsForBrain.push_back(animal.getVelocity().x);
	inputsForBrain.push_back(animal.getVelocity().y);
	
	inputsForBrain.push_back(animal.getHp() / 1000.f);

	Food* theNearestFood = findTheNearestFood(animal);
	
	inputsForBrain.push_back(theNearestFood->getPosition().x - animal.getPos().x);
	inputsForBrain.push_back(theNearestFood->getPosition().y - animal.getPos().y);

	return inputsForBrain;
}

Food* Ecosystem::findTheNearestFood(const Animal& animal)
{
	Food* theNearestFood = this->food[0];
	float theSmallestDistance = INFINITY;

	for (const auto& it : this->food)
	{
		float a = animal.getPos().x - it->getPosition().x;
		float b = animal.getPos().y - it->getPosition().y;

		float distance = sqrt(pow(a, 2) + pow(b, 2));

		if (distance < theSmallestDistance)
		{
			theSmallestDistance = distance;
			theNearestFood = it;
		}
	}
	
	return theNearestFood;
}
