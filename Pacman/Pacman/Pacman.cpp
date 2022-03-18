#include "Pacman.h"

#include <sstream>

#include <iostream>

// Constructor Method
Pacman::Pacman(int argc, char* argv[]) : Game(argc, argv), _cPacmanSpeed(0.1f), _cPacmanFrameTime(150), _cMunchieFrameTime(10000)
{
	// Initialise member variables
	_pacman = new Player();
	_menu = new Menu();
	_cherry = new Enemy();

	_eatFruit = new SoundEffect();
	_eatMunchie = new SoundEffect();
	_deathSound = new SoundEffect();
	_winSound = new SoundEffect();
	_startSound = new SoundEffect();

	// Local variable
	int i;
	for (i = 0; i < GHOSTCOUNT; i++)
	{
		_ghosts[i] = new MovingEnemy();
		_ghosts[i]->direction = rand() % 4;
		_ghosts[i]->speed = 0.05f;
		_ghosts[i]->currentFrameTime = 0;
		_ghosts[i]->frame = 0;
	}

	// Local variable
	for (i = 0; i < MUNCHIECOUNT; i++)
	{
		_munchies[i] = new Enemy();
		_munchies[i]->frameCount = rand() % 1;
		_munchies[i]->currentFrameTime = 0;
		_munchies[i]->frame = rand() % 500 + 50;
	}

	// Remaining variable initialisation
	_paused = false;
	_pKeyDown = false;
	_started = false;
	_completed = false;

	// Pacman Variables
	_pacman->direction = 0;
	_pacman->currentFrameTime = 0;
	_pacman->frame = 0;
	_pacman->speedMultiplier = 1.0f;
	_pacman->dead = false;
	_pacman->highScore = 0;
	_pacman->score = 0;
	_pacman->munchiesCollected = 0;

	// Menu variable
	_menu->introPlayed = false;

	//Initialise important Game aspects
	Audio::Initialise();
	Graphics::Initialise(argc, argv, this, 1024, 768, false, 25, 25, "Pacman", 60);
	Input::Initialise();

	// Start the Game Loop - This calls Update and Draw in game loop
	Graphics::StartGameLoop();
}

// Destructor method
Pacman::~Pacman()
{
	delete _pacman->texture;
	delete _pacman->sourceRect;
	delete _pacman->position;
	delete _pacman;

	// Clean up the Texture
	delete _munchies[0]->texture;
	int nCount;
	for (nCount = 0; nCount < MUNCHIECOUNT; nCount++)
	{
		delete _munchies[nCount]->texture;
		delete _munchies[nCount]->sourceRect;
		delete _munchies[nCount];
	}
	// Order of operation is important, array deleted last
	delete[] _munchies;

	delete _cherry->texture;
	delete _cherry->sourceRect;
	delete _cherry;

	// Ghost texture
	delete 	_ghosts[0]->texture;
	for (nCount = 0; nCount < GHOSTCOUNT; nCount++)
	{
		delete 	_ghosts[nCount]->sourceRect;
		delete 	_ghosts[nCount]->position;
		delete 	_ghosts[nCount];
	}

	// Delete sounds
	delete _eatFruit;
	delete _eatMunchie;
	delete _deathSound;
	delete _winSound;
	delete _startSound;
}

void Pacman::LoadContent()
{
	// Load Pacman
	_pacman->texture = new Texture2D();
	_pacman->texture->Load("Textures/Pacman.tga", false);
	_pacman->position = new Vector2(350.0f, 350.0f);
	_pacman->sourceRect = new Rect(0.0f, 0.0f, 32, 32);

	// Load Munchie
	int i;
	Texture2D* munchieTex = new Texture2D();
	munchieTex->Load("Textures/Munchie.png", false);
	for (i = 0; i < MUNCHIECOUNT; i++)
	{
		_munchies[i]->texture = new Texture2D();
		_munchies[i]->texture = munchieTex;
		_munchies[i]->position = new Vector2(((rand() % Graphics::GetViewportWidth())), (rand() % Graphics::GetViewportHeight()));
		_munchies[i]->sourceRect = new Rect(0.0f, 0.0f, 12, 12);
	}

	// Load Cherry
	_cherry->texture = new Texture2D();
	_cherry->texture->Load("Textures/Cherry.png", false);
	_cherry->position = new Vector2(455.0f, 400.0f);
	_cherry->sourceRect = new Rect(0.0f, 0.0f, 32, 32);

	// Load Ghost Characterss
	Texture2D* ghostTex = new Texture2D();
	ghostTex->Load("Textures/GhostBlue.png", false);
	for (i = 0; i < GHOSTCOUNT; i++)
	{
		_ghosts[i]->texture = ghostTex;
		_ghosts[i]->position = new Vector2(((rand() % Graphics::GetViewportWidth())), (rand() % Graphics::GetViewportHeight()));
		_ghosts[i]->sourceRect = new Rect(0.0f, 0.0f, 20, 20);
	}

	// Load sounds
	_eatFruit->Load("Sounds/EatFruit.wav");
	_eatMunchie->Load("Sounds/Pop.wav");
	_deathSound->Load("Sounds/Death.wav");
	_winSound->Load("Sounds/Win.wav");
	_startSound->Load("Sounds/Intro.wav");

	// Set string position
	_stringPosition = new Vector2(10.0f, 25.0f);

	// Set Menu Paramters
	_menu->background = new Texture2D();
	_menu->background->Load("Textures/Transparency.png", false);
	_menu->rectangle = new Rect(0.0f, 0.0f, Graphics::GetViewportWidth(), Graphics::GetViewportHeight());
	_menu->stringPosition = new Vector2(Graphics::GetViewportWidth() / 2.0f, Graphics::GetViewportHeight() / 2.0f);
}

void Pacman::Update(int elapsedTime)
{
	// Gets the current state of the keyboard
	Input::KeyboardState* keyboardState = Input::Keyboard::GetState();

	// Gets the current state of the mouse
	Input::MouseState* mouseState = Input::Mouse::GetState();

	// Game 'completes' when 49 munchies are collected
	if (_pacman->munchiesCollected >= 49)
	{
		_completed = true;
	}

	if (!_completed)
	{
		// if pacman is dead, none of the games major functions will work
		if (!_pacman->dead)
		{
			CheckStarted(keyboardState, Input::Keys::SPACE);
			if (_started)
			{
				CheckPaused(keyboardState, Input::Keys::P);
				// These functions will run if the game is not paused
				if (!_paused)
				{
					Input(elapsedTime, keyboardState, mouseState);
					UpdatePacman(elapsedTime);
					CheckViewportCollision();
					CheckMunchieCollision();
					CheckCherryCollision();
					CheckGhostCollision();

					int i;
					for (i = 0; i < GHOSTCOUNT; i++)
					{
						UpdateGhost(_ghosts[i], elapsedTime);
					}

					for (i = 0; i < MUNCHIECOUNT; i++)
					{
						UpdateMunchie(_munchies[i], elapsedTime);
					}
				}
			}
			else if (!_started && !_menu->introPlayed) // Plays intro music if hasnt played already and game hasnt started
			{
				Audio::Play(_startSound);
				_menu->introPlayed = true;
			}
		}
		else
		{
			RestartGame(keyboardState, Input::Keys::SPACE);
		}
	}
	else
	{
		RestartGame(keyboardState, Input::Keys::SPACE);
	}
}

void Pacman::Input(int elapsedTime, Input::KeyboardState* keyState, Input::MouseState* mouseState)
{
	// Pacman speeds up when LEFT SHIFT is pressed
	float pacmanSpeed = _cPacmanSpeed * elapsedTime * _pacman->speedMultiplier;

	// Checks if D key is pressed
	if (keyState->IsKeyDown(Input::Keys::D))
	{
		_pacman->position->X += pacmanSpeed; //Moves Pacman across X axis
		_pacman->direction = 0;
		_pacman->currentFrameTime += elapsedTime;
	}
	// Checks if A key is pressed
	else if (keyState->IsKeyDown(Input::Keys::A))
	{
		_pacman->position->X -= pacmanSpeed; //Moves Pacman across X axis
		_pacman->direction = 2;
		_pacman->currentFrameTime += elapsedTime;
	}
	// Checks if W key is pressed
	else if (keyState->IsKeyDown(Input::Keys::W))
	{
		_pacman->position->Y -= pacmanSpeed; //Moves Pacman across Y axis
		_pacman->direction = 3;
		_pacman->currentFrameTime += elapsedTime;
	}
	// Checks if S key is pressed
	else if (keyState->IsKeyDown(Input::Keys::S))
	{
		_pacman->position->Y += pacmanSpeed; //Moves Pacman across Y axis
		_pacman->direction = 1;
		_pacman->currentFrameTime += elapsedTime;
	}
}

void Pacman::CheckPaused(Input::KeyboardState* state, Input::Keys pauseKey)
{
	// Pause toggle
	if (state->IsKeyDown(Input::Keys::P) && !_pKeyDown)
	{
		_pKeyDown = true;
		_paused = !_paused;
	}

	if (state->IsKeyUp(Input::Keys::P))
		_pKeyDown = false;
}

void Pacman::CheckStarted(Input::KeyboardState* state, Input::Keys spaceBar)
{
	if (state->IsKeyDown(Input::Keys::SPACE))
	{
		_started = true;
	}
}

void Pacman::UpdatePacman(int elapsedTime)
{
	// Pacmans speed increases every 1000 points
	if (_pacman->score >= 1000)
	{
		_pacman->speedMultiplier = (_pacman->score / 2000) + 1;
	}

	// High score updates when surpassed
	if (_pacman->score >= _pacman->highScore)
	{
		_pacman->highScore = _pacman->score;
	}

	_pacman->sourceRect->Y = _pacman->sourceRect->Height * _pacman->direction; //Changes the way Pacman is facing

	// Pacman frame count for sprites
	if (_pacman->currentFrameTime > _cPacmanFrameTime)
	{
		_pacman->frame++;

		if (_pacman->frame >= 2)
			_pacman->frame = 0;

		_pacman->currentFrameTime = 0;
	}

	_pacman->sourceRect->X = _pacman->sourceRect->Width * _pacman->frame; //Changes Pacmans biting animation frame
}

void Pacman::UpdateMunchie(Enemy*, int elapsedTime)
{
	// updates all munchie sprites
	int i;
	for (i = 0; i < MUNCHIECOUNT; i++)
	{

		// Munchies frame count for sprites
		_munchies[i]->currentFrameTime += elapsedTime;

		if (_munchies[i]->currentFrameTime > _cMunchieFrameTime)
		{
			_munchies[i]->frameCount++;

			if (_munchies[i]->frameCount >= 2)
				_munchies[i]->frameCount = 0;

			_munchies[i]->currentFrameTime = 0;
		}

		_munchies[i]->sourceRect->X = _munchies[i]->sourceRect->Width * _munchies[i]->frame; //Initialises munchie animation frame
	}
}

void Pacman::UpdateGhost(MovingEnemy*, int elapsedTime)
{
	int i;
	for (i = 0; i < GHOSTCOUNT; i++)
	{
		// Ghost uses time based animation
		_ghosts[i]->currentFrameTime += elapsedTime;

		if (_ghosts[i]->direction == 0) // Move right
		{
			_ghosts[i]->position->X += _ghosts[i]->speed * elapsedTime;
		}
		else if (_ghosts[i]->direction == 3) // Move down
		{
			_ghosts[i]->position->Y -= _ghosts[i]->speed * elapsedTime;
		}
		else if (_ghosts[i]->direction == 2) // Move left
		{
			_ghosts[i]->position->X -= _ghosts[i]->speed * elapsedTime;
		}
		else if (_ghosts[i]->direction == 1) // Move left
		{
			_ghosts[i]->position->Y += _ghosts[i]->speed * elapsedTime;
		}

		if (_ghosts[i]->position->X + _ghosts[i]->sourceRect->Width >= Graphics::GetViewportWidth() && (_ghosts[i]->direction == 2 || _ghosts[i]->direction == 0)) // Hits right edge
		{
			_ghosts[i]->position->X -= 5;
			_ghosts[i]->direction = 2; // Change direction
		}
		else if (_ghosts[i]->position->X <= 0) // Hits left edge
		{
			_ghosts[i]->position->X += 5;
			_ghosts[i]->direction = 0; // Change direction
		}

		if (_ghosts[i]->position->Y + _ghosts[i]->sourceRect->Height >= Graphics::GetViewportHeight() && (_ghosts[i]->direction == 1 || _ghosts[i]->direction == 3)) // Hits bottom edge
		{
			_ghosts[i]->position->Y -= 5;
			_ghosts[i]->direction = 3; // Change direction
		}
		else if (_ghosts[i]->position->Y <= 0) // Hits top edge
		{
			_ghosts[i]->position->Y += 5;
			_ghosts[i]->direction = 1; // Change direction
		}

		_ghosts[i]->sourceRect->Y = _ghosts[i]->sourceRect->Height * _ghosts[i]->direction; //Changes the way the ghost is facing

		// Ghosts frame count for sprites
		if (_ghosts[i]->currentFrameTime > _cPacmanFrameTime)
		{
			_ghosts[i]->frame++;

			if (_ghosts[i]->frame >= 2)
				_ghosts[i]->frame = 0;

			_ghosts[i]->currentFrameTime = 0;
		}

		_ghosts[i]->sourceRect->X = _ghosts[i]->sourceRect->Width * _ghosts[i]->frame; //Changes ghost moving animation frame
	}
}

void Pacman::CheckViewportCollision()
{
	// Checks if Pacman is trying to disapear
	if (_pacman->position->X + _pacman->sourceRect->Width > Graphics::GetViewportWidth()) //1024 is game width
	{
		// Pacman hit right wall - reset his position
		_pacman->position->X = 1024 - _pacman->sourceRect->Width;
	}
	if (_pacman->position->X + _pacman->sourceRect->Width < 32)
	{
		// Pacman hit left wall - reset his position
		_pacman->position->X = 32 - _pacman->sourceRect->Width;
	}
	if (_pacman->position->Y + _pacman->sourceRect->Height > Graphics::GetViewportHeight()) //768 is game height
	{
		// Pacman hit bottom wall - reset his position
		_pacman->position->Y = 768 - _pacman->sourceRect->Height;
	}
	if (_pacman->position->Y + _pacman->sourceRect->Height < 32)
	{
		// Pacman hit top wall - reset his position
		_pacman->position->Y = 32 - _pacman->sourceRect->Height;
	}
}

void Pacman::CheckMunchieCollision()
{
	// Local Variables
	int i = 0;
	int bottom1 = _pacman->position->Y + _pacman->sourceRect->Height;
	int bottom2 = 0;
	int left1 = _pacman->position->X;
	int left2 = 0;
	int right1 = _pacman->position->X + _pacman->sourceRect->Width;
	int right2 = 0;
	int top1 = _pacman->position->Y;
	int top2 = 0;

	for (i = 0; i < MUNCHIECOUNT; i++)
	{
		// Populate variables with Munchie data
		bottom2 = _munchies[i]->position->Y + _munchies[i]->sourceRect->Height;
		left2 = _munchies[i]->position->X;
		right2 = _munchies[i]->position->X + _munchies[i]->sourceRect->Width;
		top2 = _munchies[i]->position->Y;

		if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2))
		{
			Audio::Play(_eatMunchie);
			_pacman->munchiesCollected++;
			_pacman->score += 100;
			_munchies[i]->position->Y += 1000;
			
			if (_pacman->munchiesCollected >= 49)
			{
				Audio::Play(_winSound);
			}

			i = MUNCHIECOUNT;
		}
	}
}

void Pacman::CheckCherryCollision()
{
	// Local Variables
	int i = 0;
	int bottom1 = _pacman->position->Y + _pacman->sourceRect->Height;
	int bottom2 = 0;
	int left1 = _pacman->position->X;
	int left2 = 0;
	int right1 = _pacman->position->X + _pacman->sourceRect->Width;
	int right2 = 0;
	int top1 = _pacman->position->Y;
	int top2 = 0;

	for (i = 0; i < MUNCHIECOUNT; i++)
	{
		// Populate variables with cherry data
		bottom2 = _cherry->position->Y + _cherry->sourceRect->Height;
		left2 = _cherry->position->X;
		right2 = _cherry->position->X + _cherry->sourceRect->Width;
		top2 = _cherry->position->Y;

		if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2))
		{
			_pacman->score += 300;
			Audio::Play(_eatFruit);
			_cherry->position = new Vector2(((rand() % Graphics::GetViewportWidth())), (rand() % Graphics::GetViewportHeight()));
			i = MUNCHIECOUNT;
		}
	}
}

void Pacman::CheckGhostCollision()
{
	// Local Variables
	int i = 0;
	int bottom1 = _pacman->position->Y + _pacman->sourceRect->Height;
	int bottom2 = 0;
	int left1 = _pacman->position->X;
	int left2 = 0;
	int right1 = _pacman->position->X + _pacman->sourceRect->Width;
	int right2 = 0;
	int top1 = _pacman->position->Y;
	int top2 = 0;

	for (i = 0; i < GHOSTCOUNT; i++)
	{
		// Populate variables with Ghost data
		bottom2 = _ghosts[i]->position->Y + _ghosts[i]->sourceRect->Height;
		left2 = _ghosts[i]->position->X;
		right2 = _ghosts[i]->position->X + _ghosts[i]->sourceRect->Width;
		top2 = _ghosts[i]->position->Y;

		if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2))
		{
			Audio::Play(_deathSound);
			_pacman->dead = true;
			i = GHOSTCOUNT;
		}
	}
}

void Pacman::RestartGame(Input::KeyboardState* state, Input::Keys spaceBar)
{
	if (state->IsKeyDown(Input::Keys::SPACE))
	{
		LoadContent();

		if (_pacman->dead) // Player loses score and speed
		{
			_pacman->score = 0;
			_pacman->speedMultiplier = 1;
		}

		// Variables hat need to be changed for the game to function after restarting
		_pacman->direction = 0;
		_pacman->dead = false;
		_completed = false;
		_pacman->munchiesCollected = 0;
	}
}

void Pacman::Draw(int elapsedTime)
{
	// Allows us to easily create a string
	std::stringstream stream;
	// Displays score and high score
	stream << "HIGH SCORE: " << _pacman->highScore << endl;
	stream << "SCORE: " << _pacman->score;

	SpriteBatch::BeginDraw(); // Starts Drawing

	// Checks if pacman is dead
	if(!_pacman->dead)
		SpriteBatch::Draw(_pacman->texture, _pacman->position, _pacman->sourceRect); // Draws Pacman

	SpriteBatch::Draw(_cherry->texture, _cherry->position, _cherry->sourceRect); // Draws Cherry

	int i;
	for (i = 0; i < GHOSTCOUNT; i++)
	{
		if (i == 0)
		{
			_ghosts[i]->texture->Load("Textures/GhostBlue.png", false);
			SpriteBatch::Draw(_ghosts[i]->texture, _ghosts[i]->position, _ghosts[i]->sourceRect); // Draws Blue Ghost
		}
		if (i == 1)
		{
			_ghosts[i]->texture->Load("Textures/GhostRed.png", false);
			SpriteBatch::Draw(_ghosts[i]->texture, _ghosts[i]->position, _ghosts[i]->sourceRect); // Draws Red Ghost
		}
		if (i == 2)
		{
			_ghosts[i]->texture->Load("Textures/GhostOrange.png", false);
			SpriteBatch::Draw(_ghosts[i]->texture, _ghosts[i]->position, _ghosts[i]->sourceRect); // Draws Orange Ghost
		}
		if (i == 3)
		{
			_ghosts[i]->texture->Load("Textures/GhostPink.png", false);
			SpriteBatch::Draw(_ghosts[i]->texture, _ghosts[i]->position, _ghosts[i]->sourceRect); // Draws Pink Ghost
		}
	}

	for (i = 0; i < MUNCHIECOUNT; i++)
	{
		SpriteBatch::Draw(_munchies[i]->texture, _munchies[i]->position, _munchies[i]->sourceRect); // Draws Munchie
	}

	for (i = 0; i < MUNCHIECOUNT; i++)
	{
		// Changes munchie aniation frame
		if (_munchies[i]->frameCount == 0)
		{
			_munchies[i]->frame = 0;
		}
		else
		{
			_munchies[i]->frame = 1;

			if (_munchies[i]->frameCount >= 2)
				_munchies[i]->frameCount = 0;
		}
	}
	
	// Draws start screen
	if (!_started)
	{
		std::stringstream menuStream; menuStream << "PRESS SPACE" << endl << "   TO START!";
		SpriteBatch::Draw(_menu->background, _menu->rectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menu->stringPosition, Color::Blue);
	}

	// Draws pause screen
	if (_paused)
	{
		std::stringstream menuStream; menuStream << "PAUSED!";
		SpriteBatch::Draw(_menu->background, _menu->rectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menu->stringPosition, Color::Red);
	}

	// Draws game over screen
	if (_pacman->dead)
	{
		std::stringstream menuStream; menuStream << "GAME OVER!" << endl << "HIGH SCORE: " << _pacman->highScore << endl << "SCORE: " << _pacman->score << endl << endl << "PRESS SPACE TO RESTART";
		SpriteBatch::Draw(_menu->background, _menu->rectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menu->stringPosition, Color::Red);
	}

	// Draws level cmplete screen
	if (_completed)
	{
		std::stringstream menuStream; menuStream << "LEVEL COMPLETE!" << endl << "HIGH SCORE: " << _pacman->highScore << endl << "SCORE: " << _pacman->score << endl << endl << "PRESS SPACE TO CONTINUE";
		SpriteBatch::Draw(_menu->background, _menu->rectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menu->stringPosition, Color::Red);
	}

	// Draws String
	SpriteBatch::DrawString(stream.str().c_str(), _stringPosition, Color::Green);
	SpriteBatch::EndDraw(); // Ends Drawing
}