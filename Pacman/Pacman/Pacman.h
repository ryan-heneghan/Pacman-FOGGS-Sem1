#pragma once

// If Windows and not in Debug, this will run without a console window
// You can use this to output information when debugging using cout or cerr
#ifdef WIN32 
	#ifndef _DEBUG
		#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
	#endif
#endif

// Just need to include main header file
#include "S2D/S2D.h"

// Reduces the amount of typing by including all classes in S2D namespace
using namespace S2D;

// Structure Definition
struct Player
{
	// Data to represent Pacman
	Vector2* position;
	Rect* sourceRect;
	Texture2D* texture;
	int direction;
	int frame;
	int currentFrameTime;
	float speedMultiplier;
	bool dead;
	int highScore;
	int score;
	int munchiesCollected;
};

struct Enemy
{
	// Data to represent Munchie/Cherry
	Vector2* position;
	Rect* sourceRect;
	Texture2D* texture;
	int frameCount;
	int frame;
	int currentFrameTime;

	#define MUNCHIECOUNT 50
};

struct MovingEnemy
{
	// Data to represent Ghost
	Vector2* position;
	Rect* sourceRect;
	Texture2D* texture;
	int direction;
	int frame;
	int currentFrameTime;
	float speed;

	#define GHOSTCOUNT 4
};

struct Menu
{
	// Data for Menu
	Vector2* stringPosition;
	Rect* rectangle;
	Texture2D* background;
	bool introPlayed;
};

// Declares the Pacman class which inherits from the Game class.
// This allows us to overload the Game class methods to help us
// load content, draw and update our game.
class Pacman : public Game
{
private:
	// Declaring structures
	Player* _pacman;
	Enemy* _munchies[MUNCHIECOUNT];
	Menu* _menu;
	Enemy* _cherry;
	MovingEnemy* _ghosts[GHOSTCOUNT];
	SoundEffect* _eatFruit;
	SoundEffect* _eatMunchie;
	SoundEffect* _deathSound;
	SoundEffect* _winSound;
	SoundEffect* _startSound;

	// Input methods
	void Input(int elapsedTime, Input::KeyboardState* keyState, Input::MouseState* mouseState);
	void RestartGame(Input::KeyboardState* state, Input::Keys spaceBar);

	// Check methods
	void CheckPaused(Input::KeyboardState* state, Input::Keys pauseKey);
	void CheckStarted(Input::KeyboardState* state, Input::Keys spaceBar);
	void CheckViewportCollision();

	// Update methods
	void UpdatePacman(int elapsedTime);
	void UpdateMunchie(Enemy*, int elapsedTime);
	void UpdateGhost(MovingEnemy*, int elapsedTime);

	// Variables for menus
	bool _paused;
	bool _pKeyDown;
	bool _started;
	bool _completed;

	// Position for String
	Vector2* _stringPosition;

	// Constant data for Game Variables
	const float _cPacmanSpeed;
	const int _cPacmanFrameTime;
	const int _cMunchieFrameTime;

	// Collision methods
	void CheckMunchieCollision();
	void CheckCherryCollision();
	void CheckGhostCollision();


public:
	/// <summary> Constructs the Pacman class. </summary>
	Pacman(int argc, char* argv[]);

	/// <summary> Destroys any data associated with Pacman class. </summary>
	virtual ~Pacman();

	/// <summary> All content should be loaded in this method. </summary>
	void virtual LoadContent();

	/// <summary> Called every frame - update game logic here. </summary>
	void virtual Update(int elapsedTime);

	/// <summary> Called every frame - draw game here. </summary>
	void virtual Draw(int elapsedTime);
};