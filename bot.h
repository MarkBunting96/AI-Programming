#pragma once
#include "shapes.h"
#include "rules.h"
#include "PathFinding.h"

enum State{ATTACK, DEFEND, CAPTURE, REARM, NOSTATE};

class Bot
{
protected:
	Vector2D m_Position;			// Current world coordinates
	Vector2D m_Velocity;			// Current velocity
	Vector2D m_Acceleration;
	float m_dDirection;				// Direction the bot is pointing in. radians anticlockwise from south
	int m_iAmmo;					// Not currently used
	bool m_bFiring;
	bool m_bAiming;					// If true, bot is aiming and cannot move
	float m_dTimeToCoolDown;		// Countdown until the time the bot can shoot again
	float m_dTimeToRespawn;			// Countdown until the bot can respawn. If zero or below, bot is alive
	int m_iAimingAtTeam;			// Team number of the bot being aimed at
	int m_iAimingAtBot;				// Number of the bot being aimed at
	int m_iOwnTeamNumber;
	int m_iOwnBotNumber;
	float m_dAccuracy;				// Accuracy of the current firing solution (1==100%)
	int m_iHealth;					// Health (up to 100)

	Vector2D targetPoint;			// Used only by placeholder AI. Delete this.

	Graph m_graph;					//graph used to pathfind

	std::vector<Vector2D> path;		//vector of nodes for the bot to follow

public:
	Bot();	

	// Runs once each frame. Handles physics, shooting, and calls
	// ProcessAI, processenhancedAI and processAI badly
	void Update(float frametime);

	void Reload();

	//the current and previous state of the bot
	State currentState;
	State previousState;

	// Returns the location of the bot
	Vector2D GetLocation();

	// Returns the velocity of the bot
	Vector2D GetVelocity();
	
	// Returns the direction the bot is pointing. In radians anticlockwise from south
	float GetDirection();

	// Restarts the bot in a new location, with full heath, etc
	void PlaceAt(Vector2D position);	
	
	// Returns true if the bot is currently not respawning
	bool IsAlive();					
	
	// This is your function. Use it to set up any states at the beginning of the game
	// and analyse the map.
	// Remember that bots have not spawned yet, so will not be in their
	// starting positions.
	void StartAI();

	//Original Process AI function, without fuzzy logic. Set up to control the bots in a better more efficient
	//way than the processAI badly function. Uses states and behaviours to control the bots.
	void ProcessAI();	

	//Similar to above function but uses fuzzy logic when deciding to attack. Enhanced version of the above function
	//causes the bots to seek enemies instead of only attacking when in range with fuzzy values instead of boolean values.
	void ProcessEnhancedAI();

	// This is a quick n' dirty AI for team 2.
	// Try to make team 1 better than this.
	// Will be called once each frame from Update
	void ProcessAIBadly();
			
	// Returns the number of the team of the bot being aimed at.
	// Returns a negative number if no bot is being aimed at.
	int GetTargetTeam();	

	// Returns the number of the bot being aimed at.
	// Returns a negative number if no bot is being aimed at.
	int GetTargetBot();

	// Sets the bots own team number and bot number.
	// No need to call this
	void SetOwnNumbers(int teamNo, int botNo);

	// Returns the current health of the bot
	int GetHealth();

	// Returns the current accuracy of the bot.
	// Accuracy is the probability of hitting the current target.
	// If the bot is not aiming, this will be zero.
	double GetAccuracy();

	// Sets the target of the current bot.
	// This will reset the accuracy to zero if the target bot 
	// has changed.
	// It also sets the bot to aiming mode, which means it will stop
	// moving.
	// If you want the bot to stop aiming, just set mbAiming = false
	// or use the StopAiming method
	void SetTarget(int targetTeamNo, int targetBotNo);

	// Call this to set the bot to shoot, if it can.
	// Once a shot it taken, the bot will not shoot again unless told to do so.
	void Shoot();

	// Delivers the specified damage to the bot.
	// If this drops health below zero, the bot will die
	// and will respawn at the spawn point after a specified interval
	void TakeDamage(int amount);

	// Stops the bot from aiming, so it can move again
	void StopAiming();

	// Returns a vector of a target the bot will move towards
	Vector2D Seek(Vector2D target);

	// Similar to Seek but will move in the opposite direction
	Vector2D Flee(Vector2D target);

	// Similar to seek, howerever, the bot slows down to a stop
	// as it reaches the target vector
	Vector2D Arrive(Vector2D target);

	// Chooses a point ahead of the target and calculates a rough
	// intercept time, and predict the position of the target at that time
	Vector2D Pursue(Vector2D targetPos, Vector2D targetVel);

	//Similar to Pursue but will use Flee instead of Seek at the end
	Vector2D Evade (Vector2D targetPos, Vector2D targetVel);

	//Uses a circle collider to avoid walls
	Vector2D WallAvoid();

	//Uses the A* algorithm to follow a path
	Vector2D FollowPath();

	//returns a pointer to the closest enemy by cycling through each enemy bot and determening the closest one
	Bot* GetClosestEnemy();

	//FuzzyBeatability Function, used to generate fuzzy values rather than boolean values to decide if the enemy
	//should enter the attack state. Generates a float between 0 and 1 for the friendly bots healthyness, the enemy bots
	//weakness, and the closeness between the bots, and gets the average of the three values and returns that average.
	float FuzzyBeatability();

	//Takes a state as a parameter and checks that it is not already in that state and changes the state and sets the previous state
	void ChangeState(State nextState);
	
	//capture state, gives the bot a specific domination point to go capture if not owned by the bots team.
	void CaptureState();

	//defend state, makes the bot stay near one of their domination points if they own it.
	void DefendState();

	//attack state used to travel to the closest bot, stop, aim and shoot at it
	void AttackState();

	//Rearm state, used to tell the bot to find hte nearest resupply point and aquire more ammo.
	void RearmState();

	//draw path function, used for debugging, but draws each node in the bots path using their number as a colour
	void drawPath(int botNum);


};

