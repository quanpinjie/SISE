#include "Level.h"
#include "Game.h"
#include "..\Clipse/Projects/Source/CLIPS/clips.h"
#include <ctime>
#include <sstream>

Game::Game():
level(nullptr),
window(VideoMode(800, 600, 32), "Heroes tsy"),
view(Vector2f(400.0f, 300.0f), Vector2f(800.0f, 600.0f))
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
	hud = new Hud(view, window);
	selectedFieldPos = Vector2f(0.0f,0.0f);
	selectedUnitIndex = -1;
	previousUnitIndex = -1;
	selectedFieldIndex = -1;
}

Game::~Game()
{
	delete level;
	delete hud;
}

bool Game::OpponentUnit(Unit* unit)
{
	for (int i = 0; i < level->players[!playerTurn]->units.size(); i++)
	{
		if (unit == level->players[!playerTurn]->units[i])
		{
			return true;
		}
	}
	return false;
}

void Game::play(myCLIPSRouter &theRouter, CLIPSCPPEnv &theEnv)
{
	cout << "Player" << playerTurn + 1 << " turn\n\n";
	hud->WriteGameState("Player" + to_string(playerTurn + 1) + " turn\n\n");

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
		hud->display(playerTurn);
		window.display();

		
		if ((eventHandler.unitSelected && !flag) || playerTurn == 1)
		{
			
			if (playerTurn == 0)
				selectedUnitIndex = selectedUnit(eventHandler.mousePosition);
			else
			{
				//GenerateRandomNumberForClipse(0, 4, theEnv);

				UpdateUnitsInClips(theEnv);
				ComputeUnitSelectionInCLips(theEnv);

				selectedUnitIndexStr = GetSelectedUnitFromClips(theRouter, theEnv);
				selectedUnitIndex = atoi(selectedUnitIndexStr.c_str());

				UpdateAvailableFieldsInCLips(ComputeAvailableFields(), theEnv);
				ComputeFieldSelectionInClips(theEnv);

				selectedFieldIndexStr = GetSelectedFieldFromClips(theRouter, theEnv);
				selectedFieldIndex = atoi(selectedFieldIndexStr.c_str());

				cout << "selectedUnitIndex = " << selectedUnitIndex << "; " << "selectedFieldIndex = " << selectedFieldIndex << endl;

				Time delayTime = milliseconds(1000);
				sleep(delayTime);
			}

			previousUnitIndex = selectedUnitIndex;

			if (previousUnitIndex != -1)
				level->players[!playerTurn]->units[previousUnitIndex]->message = "";

			if (selectedUnitIndex != -1)
			{
				//cout << "Selected  " << level->players[playerTurn]->units[selectedUnitIndex]->getType() << endl;
				hud->WriteGameState("Selected  " + level->players[playerTurn]->units[selectedUnitIndex]->getType());
				flag = true;
				DrawRange(level->players[playerTurn]->units[selectedUnitIndex]);
			}
			else
			{
				//cout << "Wrong select! Try again" << endl;
				hud->WriteGameState("Wrong select! Try again");
				eventHandler.unitSelected = false;
				eventHandler.fieldSelected = false;
				eventHandler.numberOfClicks = 0;
				flag = false;
			}
		}

		if (eventHandler.fieldSelected && eventHandler.unitSelected || playerTurn == 1)
		{
			if (playerTurn == 0)
			{
				selectedFieldPos = selectedField(eventHandler.mousePosition, selectedFieldIndex);
			}
			else
			{
				selectedFieldPos = level->fieldsPositions[selectedFieldIndex];
				cout << "selectedFieldPos = " << selectedFieldPos.x << ", " << selectedFieldPos.y << endl;
			}

			for each (Field *field in level->fields)
			{
				if (field == level->players[playerTurn]->units[selectedUnitIndex]->field)
				{
					field->hasUnit = false;
					break;
				}
			}

			bool fieldExist = false;
			if (selectedFieldIndex != -1 && selectedFieldPos != Vector2f(0, 0))
			{
				fieldExist = true;
			}

			bool fieldHasUnit = false;
			if (selectedFieldIndex != -1 && level->fields[selectedFieldIndex]->hasUnit)
			{
				fieldHasUnit = true;
			}

			bool fieldInRange = false;
			if (selectedFieldIndex != -1 && level->fields[selectedFieldIndex]->inRange)
			{
				fieldInRange = true;
			}

			if (fieldExist)
			{
				if (fieldInRange)
				{
					if (!fieldHasUnit)
					{
						level->players[playerTurn]->units[selectedUnitIndex]->field = level->fields[selectedFieldIndex];
						level->fields[selectedFieldIndex]->unit = level->players[playerTurn]->units[selectedUnitIndex];

						selectedFieldPos = level->fieldsPositions[selectedFieldIndex];

						if (playerTurn == 1)
						{
							selectedFieldPos.x = selectedFieldPos.x + 55.0f;
						}
						level->players[playerTurn]->decide(level->players[!playerTurn], level->players[playerTurn]->units[selectedUnitIndex], selectedFieldPos);

						level->fields[level->players[playerTurn]->units[selectedUnitIndex]->field->index]->hasUnit = false;
						level->fields[level->players[playerTurn]->units[selectedUnitIndex]->field->index]->render = true;
						level->fields[selectedFieldIndex]->hasUnit = true;
					}
					else
					{
						if (OpponentUnit(level->fields[selectedFieldIndex]->unit))
						{
							for (int i = 0; i < level->players[!playerTurn]->units.size(); i++)
							{
								if (level->fields[selectedFieldIndex] == level->players[!playerTurn]->units[i]->field)
								{

									if (playerTurn == 0 && selectedFieldIndex > 11)
									{
										selectedFieldIndex -= 11;
									}
									else if (selectedFieldIndex < 154)
									{
										selectedFieldIndex += 11;
									}

									level->players[playerTurn]->units[selectedUnitIndex]->field = level->fields[selectedFieldIndex];
									level->fields[selectedFieldIndex]->unit = level->players[playerTurn]->units[selectedUnitIndex];

									selectedFieldPos = level->fieldsPositions[selectedFieldIndex];

									if (playerTurn == 1)
									{
										selectedFieldPos.x = selectedFieldPos.x + 55.0f;
									}
									level->players[playerTurn]->decide(level->players[!playerTurn], level->players[playerTurn]->units[selectedUnitIndex], selectedFieldPos);

									level->fields[selectedFieldIndex]->hasUnit = true;

									level->players[playerTurn]->units[selectedUnitIndex]->attack(level->players[!playerTurn]->units[i]);

									break;
								}
							}
						}
					}

				}
				else
				{
					if (level->players[playerTurn]->units[selectedUnitIndex]->getType() == "Healer" && fieldHasUnit && !OpponentUnit(level->fields[selectedFieldIndex]->unit))
					{
						level->players[playerTurn]->units[selectedUnitIndex]->heal(level->fields[selectedFieldIndex]->unit);
					}

					else if (level->players[playerTurn]->units[selectedUnitIndex]->getType() == "Archer" && fieldHasUnit && OpponentUnit(level->fields[selectedFieldIndex]->unit))
					{
						level->players[playerTurn]->units[selectedUnitIndex]->attack(level->fields[selectedFieldIndex]->unit);
					}

					else
					{
						hud->WriteGameState("Field out of range!");
						eventHandler.fieldSelected = false;
						eventHandler.numberOfClicks = 1;
					}
				}
			}
			else
			{
				//cout << "Wrong select! Try again" << endl;
				hud->WriteGameState("There is no such field!");
				eventHandler.fieldSelected = false;
				eventHandler.numberOfClicks = 1;
			}
		}

		if ((selectedFieldPos != Vector2f(0, 0) && selectedUnitIndex != -1 && eventHandler.fieldSelected && eventHandler.unitSelected) || playerTurn == 1)
		{
			for each (Entity *field in level->fields)
			{
				field->render = false;
				//field->inRange = false;
			}

			eventHandler.fieldSelected = false;
			eventHandler.unitSelected = false;
			flag = false;
			playerTurn = !playerTurn;
			//cout << "Player" << playerTurn + 1 << " Turn\n\n";
			hud->WriteGameState("Player" + to_string(playerTurn + 1) + " turn\n\n");
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
	FloatRect cursorRect(mousePosition.x, mousePosition.y, static_cast<float>(cursorPrecision), static_cast<float>(cursorPrecision));

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
		if (level->fields[i]->getRenderer()->GetBounds().contains(mousePosition))
		{
			index = i;
			return level->fieldsPositions[i];
		}
	}
	return Vector2f(0, 0);
}

Vector2f Game::selectedField(int index)
{
	return level->fieldsPositions[index];
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
			if (level->fields[i]->hasUnit == false)
			{
				level->fields[i]->render = true;
			}
			level->fields[i]->inRange = true;
		}

		/*Vector2f a = Vector2f(unit->field->ColumnIndex, unit->field->RowIndex);

		int c = level->fields[i]->ColumnIndex;
		int r = level->fields[i]->RowIndex;

		if (ComputeDistanceHexGrid(a, Vector2f(c, r)) <= unit->getMovementSpeed())
		{
			if (level->fields[i]->hasUnit == false)
			{
				level->fields[i]->render = true;
			}
			level->fields[i]->inRange = true;
		}*/
	}
}

void Game::setDecisionInfo(string newDecision)
{
	this->decisionInfo = newDecision;
}

void Game::ComputeUnitSelectionInCLips(CLIPSCPPEnv &theEnv)
{
	/*theEnv.Load("selectUnitRules.clp");
	theEnv.Run(-1);*/
	ostringstream ss;
	ss << "(assert (decision (selectedUnitIndex " << rand()%4 << ")(selectedFieldIndex 0)))";
	string str1 = ss.str();
	theEnv.Eval(strdup(str1.c_str()));
	//theEnv.Eval("(facts)");
}

void Game::ComputeFieldSelectionInClips(CLIPSCPPEnv &theEnv)
{
	/*theEnv.Load("selectFieldRules.clp");
	theEnv.Run(-1);*/
	ostringstream ss;
	vector<int> tab = ComputeAvailableFields();
	int index;
	index = tab[(rand()%tab.size())];
	ss << "(assert (decision (selectedUnitIndex " << selectedUnitIndex << ")(selectedFieldIndex " << index << ")))";
	string str1 = ss.str();
	theEnv.Eval(strdup(str1.c_str()));
	//theEnv.Eval("(facts)");
}

string Game::GetSelectedUnitFromClips(myCLIPSRouter &theRouter, CLIPSCPPEnv &theEnv)
{
	ostringstream ss;
	ss << "(do-for-all-facts ((?f decision)) TRUE (printout output1 ?f:selectedUnitIndex)))";
	string str1 = ss.str();
	theEnv.Eval(strdup(str1.c_str()));

	return theRouter.str;
}

string Game::GetSelectedFieldFromClips(myCLIPSRouter &theRouter, CLIPSCPPEnv &theEnv)
{
	ostringstream ss;
	ss << "(do-for-all-facts ((?f decision)) TRUE (printout output1 ?f:selectedFieldIndex)))";
	string str1 = ss.str();
	theEnv.Eval(strdup(str1.c_str()));

	return theRouter.str;
}

vector<int> Game::ComputeAvailableFields()
{
	Vector2f a = Vector2f(level->players[playerTurn]->units[selectedUnitIndex]->field->ColumnIndex, level->players[playerTurn]->units[selectedUnitIndex]->field->RowIndex);
	vector<int> tab;
	for (int i = 0; i < level->ColumnsNumber*level->RawsNumber; i++)
	{
		int c = level->fields[i]->ColumnIndex;
		int r = level->fields[i]->RowIndex;
		if (ComputeDistanceHexGrid(a, Vector2f(c, r)) < level->players[playerTurn]->units[selectedUnitIndex]->getMovementSpeed())
		{
			int index = level->fields[i]->index;
			tab.push_back(index);
		}
	}
	return tab;
}

int Game::ComputeDistanceHexGrid(Vector2f A, Vector2f B)
{
	// compute distance as we would on a normal grid
	Vector2f distance;
	distance.x = A.x - B.x;
	distance.y = A.y - B.y;

	// compensate for grid deformation
	// grid is stretched along (-n, n) line so points along that line have
	// a distance of 2 between them instead of 1

	// to calculate the shortest path, we decompose it into one diagonal movement(shortcut)
	// and one straight movement along an axis
	Vector2f diagonalMovement;
	int lesserCoord = abs(distance.x) < abs(distance.y) ? abs(distance.x) : abs(distance.y);
	diagonalMovement.x = (distance.x < 0) ? -lesserCoord : lesserCoord; // keep the sign
	diagonalMovement.y = (distance.y < 0) ? -lesserCoord : lesserCoord; // keep the sign

	Vector2f straightMovement;

	// one of x or y should always be 0 because we are calculating a straight
	// line along one of the axis
	straightMovement.x = distance.x - diagonalMovement.x;
	straightMovement.y = distance.y - diagonalMovement.y;

	// calculate distance
	size_t straightDistance = abs(straightMovement.x) + abs(straightMovement.y);
	size_t diagonalDistance = abs(diagonalMovement.x);

	// if we are traveling diagonally along the stretch deformation we double
	// the diagonal distance
	if ((diagonalMovement.x < 0 && diagonalMovement.y > 0) ||
		(diagonalMovement.x > 0 && diagonalMovement.y < 0))
	{
		diagonalDistance *= 2;
	}

	return straightDistance + diagonalDistance;
}

void Game::UpdateAvailableFieldsInCLips(vector<int> tab, CLIPSCPPEnv &theEnv)
{
	ostringstream ss;
	for (int i = 0; i < tab.size(); i++)
	{
		ss << "(assert (field (index " << tab[i] << ")))";
		string str = ss.str();
		theEnv.Eval(strdup(str.c_str()));
	}
}

void Game::UpdateUnitsInClips(CLIPSCPPEnv &theEnv)
{
	for (int i = 1; i < 2; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			ostringstream ss;
			string assertUnits;
			ss << "(assert(unit(type " << level->players[i]->units[j]->getType() << ")("
				<< "id " << j << ")(" <<
				"hp " << level->players[i]->units[j]->getCurrentHealthPoints() << ")(" <<
				"isAlive " << level->players[i]->units[j]->isAlive << ")(" <<
				"range " << level->players[i]->units[j]->getRange() << ")(" <<
				"fieldIndex " << level->players[i]->units[j]->field->index << ")(" <<
				"movementSpeed " << level->players[i]->units[j]->getMovementSpeed() << ")(";

			if (playerTurn == 0 && i == 0 || playerTurn == 1 && i == 1)
			{
				ss << "isMine " << TRUE << ")))";
			}
			else if (playerTurn == 0 && i == 1 || playerTurn == 1 && i == 0)
			{
				ss << "isMine " << FALSE << ")))";
			}

			assertUnits = ss.str();
			theEnv.Eval(strdup(assertUnits.c_str()));
		}
	}
}

void Game::GenerateRandomNumberForClipse(int from, int to, CLIPSCPPEnv &theEnv)
{
	int random = ((rand() % to) + from);

	ostringstream ss;
	ss << "(assert (randomNumber (value " << random << ")))";
	string str = ss.str();
	theEnv.Eval(strdup(str.c_str()));
}