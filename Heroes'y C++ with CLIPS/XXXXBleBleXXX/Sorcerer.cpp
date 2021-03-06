#include "Sorcerer.h"


Sorcerer::Sorcerer()
{
	type = "Sorcerer";
	maxHealthPoints = 7;
	currentHealthPoints = 7;
	armor = 1;
	strength = 4;
	range = 4;
	movementSpeed = 2;
	inteligence = 70;
	isAlive = true;
	isSelected = false;
	img = new Image();
	img->loadFromFile("Data\\graphics\\battle.png");
	img->createMaskFromColor(Color(0, 255, 255, 255));
	texture = new Texture;
	texture->loadFromImage(*img);
	Renderer.SetTexture(*texture);
	Renderer.Sprite.setTextureRect(IntRect(42, 905, 100, 120)); // left, top, width, height
	Renderer.Sprite.setScale(1, 1);

	Renderer.Sprite.setPosition(Vector2f(75, 190));
}


Sorcerer::~Sorcerer()
{
	delete img;
	delete texture;
}
void Sorcerer::Update()
{
}

void Sorcerer::attack(Unit *targetUnit)
{
	int inteligenceTest = rand() % 100 + 1;
	int damage = strength;
	if (inteligenceTest <= inteligence)
	{
		damage += rand() % 3 + 1;
		//cout << "sila ciosu: " << damage << endl;
		message = "sila ciosu: " + to_string(damage) + "\n";
		damage -= targetUnit->getArmor();
		//cout << "Twoj cios trafil przeciwnika zadajac " << damage << " obrazen" << endl;
		message = message + "Twoj cios trafil przeciwnika zadajac " + to_string(damage) + " obrazen" + "\n";
		if (targetUnit->getCurrentHealthPoints() > 0)
		{
			targetUnit->setCurrentHealthPoints(damage);
		}
		if (targetUnit->getCurrentHealthPoints() <= 0)
		{
			//cout << "przeciwnik zginal!" << endl;
			message = message + "przeciwnik zginal!" + "\n";
			targetUnit->isAlive = false;
		}
	}
	else
	{
		//cout << "Chybiles" << endl;
		message = message + "Chybiles" + "\n";
	}
}
