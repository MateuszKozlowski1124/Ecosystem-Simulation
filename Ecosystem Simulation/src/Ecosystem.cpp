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
	unsigned individualsCount = 0U, foodCount = 0U;

	for (int i = 0; i < 4; i++) file >> temp >> individualsCount;

	file >> temp >> foodCount;

	file.close();

	// create folders for individuals:
	for (int i = 0; i < individualsCount; i++)
		Individual::setUpIndividualFolder(folder_path + '/' + std::to_string(i) + "individual");

	// food file:
	std::ofstream foodFile(folder_path + '/' + Ecosystem::foodFileName);

	if (!foodFile.is_open()) throw("ERROR::ECOSYSTEM::CANNOT OPEN A FILE: " + folder_path + '/' + Ecosystem::foodFileName);

	std::stringstream ss;
	
	for (float i = 0.f; i < foodCount; i++) ss << 192.f * i << ' ' << 108.f * i << '\n';

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
	for (auto& individual : this->individuals) delete individual;
	
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
	unsigned individualsCount = 0U, foodCount = 0U;

	file >> temp >> this->worldSize.x;
	file >> temp >> this->worldSize.y;
	file >> temp >> this->borderThickness;
	file >> temp >> individualsCount;
	file >> temp >> foodCount;

	file.close();

	// borders and background:
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

	// individuals:
	for (int i = 0; i < individualsCount; i++)
	{	
		this->individuals.push_back(new Individual());
		this->individuals[i]->loadFromFolder(folder_path + '/' + std::to_string(i) + "individual");

		// TODO: rmv later!:
		this->individuals[i]->getMovementComponent().set_x(i * 192.f);
		this->individuals[i]->getMovementComponent().set_y(i * 108.f);
	}

	// food:
	// TODO: add static food name file
	std::ifstream file1(folder_path + '/' + "food.ini");

	if (!file1.is_open()) std::cerr << "ERROR::ECOSYSTEM::CANNOT OPEN FILE: " + folder_path + '/' + Ecosystem::configFileName;

	this->food.resize(foodCount);

	float x, y;

	for (int i = 0; i < foodCount; i++)
	{
		file1 >> x;
		file1 >> y;
		this->food.push_back(new Food(x, y));
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
void Ecosystem::update(float dt)
{
	std::vector<double> brainInputs;

	for (int i = 0; i < 5; i++) brainInputs.push_back(0.0);

	for (auto& individual : this->individuals) individual->update(dt, brainInputs);
}

void Ecosystem::render(sf::RenderTarget& target)
{
	target.draw(this->border);
	target.draw(this->background);

	for (const auto& individual : this->individuals) individual->render(target);
	for (const auto& food : this->food) food->render(target);
}
