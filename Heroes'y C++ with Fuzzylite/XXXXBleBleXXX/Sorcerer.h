#pragma once

#include "Unit.h"


using namespace std;
using namespace sf;

class Sorcerer : public Unit
{
public:
	Sorcerer();
	virtual ~Sorcerer();
	void move();
	virtual void Update() override;
	void attack(Unit *targetUnit);
private:
	Image *img;
	Texture *texture;
	int inteligence;
};

