#include "Level.h"
#include "Game.h"

Game::Game():
window(VideoMode(800, 600, 32), "Heroes tsy"),
view(Vector2f(400.0f, 300.0f), Vector2f(800.0f, 600.0f)),
level(0)
{
	window.setFramerateLimit(30);
	window.setView(view);
	window.setKeyRepeatEnabled(false);
	globalTime = 0.0f;
	level = new Level();
	level->Game = this;
	level->Init();
	flag = false;
	playerTurn = 0;
}

Game::~Game()
{
	delete level;
}

void Game::play()
{
	cout << "Player" << playerTurn + 1 << " turn\n\n";

	while (window.isOpen())
	{
		eventHandler.readEvents(window);
		
		float time = clock.restart().asSeconds();
		globalTime = globalTime + time;
		Update(time);
		window.clear();
		if (level != nullptr)
		{
			level-> Render( &window );
		}
		hud.display(view, window);
		window.display();


		int selectedFieldIndex = 0;
		
		if (eventHandler.unitSelected && !flag)
		{
			selectedUnitIndex = selectedUnit(eventHandler.mousePosition);

			if (selectedUnitIndex != -1)
			{
				cout << "Selected  " << level->players[playerTurn]->units[selectedUnitIndex]->getType() << endl;
				flag = true;
			}
			else
			{
				cout << "Wrong select! Try again" << endl;
				eventHandler.unitSelected = false;
				eventHandler.fieldSelected = false;
				eventHandler.numberOfClicks = 0;
			}

			if (selectedUnitIndex != -1)
			{
				DrawRange(level->players[playerTurn]->units[selectedUnitIndex]);
			}
		}

		if (eventHandler.fieldSelected && eventHandler.unitSelected)
		{
			selectedFieldPos = selectedField(eventHandler.mousePosition, selectedFieldIndex);

			if (selectedFieldPos != Vector2f(0,0))
			{
				level->players[playerTurn]->units[selectedUnitIndex]->field = level->fields[selectedFieldIndex];
				//cout << "Selected field x: " << selectedFieldPos.x << ", y:" << selectedFieldPos.y << endl;
			}
			else
			{
				cout << "Wrong select! Try again" << endl;
				eventHandler.fieldSelected = false;
				eventHandler.numberOfClicks = 1;
			}
		}

		if (selectedFieldPos != Vector2f(0, 0) && selectedUnitIndex != -1 && eventHandler.fieldSelected && eventHandler.unitSelected)
		{
			for (int i = 0; i < level->players[!playerTurn]->units.size(); i++)
			{
				if (level->players[!playerTurn]->units[i]->getRenderer()->GetBounds().intersects(level->fields[selectedFieldIndex]->getRenderer()->GetBounds()))
				{
					level->players[playerTurn]->units[selectedUnitIndex]->attack(level->players[!playerTurn]->units[i]);
				}
			}

			if (playerTurn == 1)
			{
				selectedFieldPos.x = selectedFieldPos.x + 55.0f;
			}

			level->players[playerTurn]->decide(level->players[!playerTurn], level->players[playerTurn]->units[selectedUnitIndex], selectedFieldPos);

			for each (Entity *field in level->fields)
			{
				field->render = false;
			}

			eventHandler.fieldSelected = false;
			eventHandler.unitSelected = false;
			flag = false;
			playerTurn = !playerTurn;
			cout << "Player" << playerTurn + 1 << " Turn\n\n";
		}
	}
}

void Game::Update(float &time)
{
	if (level != nullptr)
	{
		level->Update(time);
	}
}

int Game::selectedUnit(Vector2f mousePosition)
{
	int cursorPrecision = 1;
	FloatRect cursorRect(mousePosition.x, mousePosition.y, cursorPrecision, cursorPrecision);

	for (int j = 0; j < level->players[playerTurn]->units.size(); j++)
	{
		FloatRect unitSpriteRect = level->players[playerTurn]->units[j]->getRenderer()->GetBounds();

		if (unitSpriteRect.intersects(cursorRect))
		{
			return j;
		}
	}

	return -1;
}

Vector2f Game::selectedField(Vector2f mousePosition, int &index)
{
	for (int i = 0; i < level->ColumnsNumber*level->RawsNumber; i++)
	{
		if (level->fields[i]->getRenderer()->GetBounds().contains(mousePosition) && level->fields[i]->render)
		{
			index = i;
			return level->fields[i]->GetPosition();
		}
	}
	return Vector2f(0, 0);
}

void Game::DrawRange(Unit *unit)
{
	for (int i = 0; i < level->fields.size(); i++)
	{
		float distance = sqrt((pow((unit->field->GetPosition().x - level->fields[i]->GetPosition().x), 2) +
			pow((unit->field->GetPosition().y - level->fields[i]->GetPosition().y), 2)));

		//float distance = sqrt((pow((unit->GetPosition().x - level->fields[i]->GetPosition().x), 2) +
			//pow((unit->GetPosition().y - level->fields[i]->GetPosition().y), 2)));

		if (distance < unit->getMovementSpeed() * 44.0f + 12.0f)
		{
			level->fields[i]->render = true;
		}
	}
}