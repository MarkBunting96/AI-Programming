#include "bot.h"
#include "staticmap.h"
#include "dynamicobjects.h"
#include "renderer.h"
#include "PathFinding.h"
#include <cmath>

			

void Bot::Update(float frametime)
{
	//Team 0 uses either the ProcessAI function or the ProcessEnhanced AI function
	if (m_iOwnTeamNumber == 0)
		ProcessAI();
		//ProcessEnhancedAI();
	else
		ProcessAIBadly();

	// Check for respawn
	if(this->m_dTimeToRespawn>0)
	{
		m_dTimeToRespawn-=frametime;
		this->m_Velocity.set(0,0);
		if(m_dTimeToRespawn<=0)
		{
			PlaceAt(StaticMap::GetInstance()->GetSpawnPoint(m_iOwnTeamNumber));
			Reload();
		}
	}
	else
	{
		// Apply physics ***************************************************************
		
		// Disable movement for aiming bot
		if(this->m_bAiming == true)
		{
			m_Acceleration = -m_Velocity.unitVector()*MAXIMUMSPEED;
		}

		// Clamp acceleration to maximum
		if(m_Acceleration.magnitude()>MAXIMUMACCELERATION)
		{
			m_Acceleration=m_Acceleration.unitVector()*MAXIMUMACCELERATION;
		}

		// Accelerate
		m_Velocity+=m_Acceleration*frametime;

		// Clamp speed to maximum
		if(m_Velocity.magnitude()>MAXIMUMSPEED)
		{
			m_Velocity=m_Velocity.unitVector()*MAXIMUMSPEED;
		}

		if(m_Velocity.magnitude()>10.0)
		{
			m_dDirection = m_Velocity.angle();
		}

		// Move
		m_Position+=m_Velocity*frametime;

		// Check for collision
		Circle2D pos(m_Position, 8);

		if(StaticMap::GetInstance()->IsInsideBlock(pos))
		{
			// Collided. Move back to where you were.
			m_Position-=m_Velocity*frametime;
			m_Velocity.set(0,0);
		}

    // Check for resupply
    if ((StaticMap::GetInstance()->GetClosestResupplyLocation(m_Position) - m_Position).magnitude()<20.0f)
    {
      if (m_iAmmo <= 0 )
      {
        Renderer::GetInstance()->ShowReload(m_Position);
      }
      Reload();
    }

		 //Handle shooting ***********************************************

		if(	m_dTimeToCoolDown>0)
			m_dTimeToCoolDown-=frametime;

		if(m_bAiming == true)
		{
			// If target is not valid
			if(m_iAimingAtTeam<0 ||m_iAimingAtTeam>=NUMTEAMS 
				||m_iAimingAtBot<0 ||m_iAimingAtBot>=NUMBOTSPERTEAM  )
			{
				m_bAiming = false;
				m_dAccuracy =0;
			}
			// else if target is dead
			else if(!DynamicObjects::GetInstance()->GetBot(m_iAimingAtTeam, m_iAimingAtBot).IsAlive())
			{
				m_bAiming = false;
				m_dAccuracy =0;
			}
      else if (m_dTimeToCoolDown>0 || m_iAmmo <= 0)
			{
				// Can't shoot. Waiting to cool down or no ammo
				Bot& targetBot = DynamicObjects::GetInstance()->
												GetBot(m_iAimingAtTeam, m_iAimingAtBot);
				if(m_Velocity.magnitude()<=10.0)
				{
					m_dDirection = (targetBot.m_Position-m_Position).angle();
				}
			}
			else		// Valid target
			{
				Bot& targetBot = DynamicObjects::GetInstance()->
												GetBot(m_iAimingAtTeam, m_iAimingAtBot);
				
				if(m_Velocity.magnitude()<=10.0)
				{
					m_dDirection = (targetBot.m_Position-m_Position).angle();
				}

				// Do we have line of sight?
				if(StaticMap::GetInstance()->IsLineOfSight(this->m_Position, targetBot.m_Position))
				{
					float range = (targetBot.m_Position-m_Position).magnitude();
					
					if(range<1.0)
						range = 1.0;		// Probably aiming at yourself
											// Otherwise suicide causes a divide by zero error.
											// That's the real reason why the church is against it.

					float peakAccuracy = sqrt(ACCURATERANGE / range);
					
					// Only gain accuracy if nearly stopped
					if(m_Velocity.magnitude()<MAXBOTSPEED/3)
					{
						m_dAccuracy += (peakAccuracy - m_dAccuracy)*frametime;
					}

					if(m_bFiring==true && m_dTimeToCoolDown<=0)
					{
						// Take the shot
						m_bFiring = false;
						m_iAmmo--;
						m_dTimeToCoolDown = TIMEBETWEENSHOTS;
						int damage=0;

						while(damage<100 && (rand()%1000) < int(m_dAccuracy*1000))
						{
							damage+=20;
						}

						targetBot.TakeDamage(damage);

						m_dAccuracy/=2.0;

						Vector2D tgt = targetBot.m_Position;
						if(damage==0)	// you missed
						{
							// Make it look like a miss
							tgt+=Vector2D(rand()%30-15.0f, rand()%30-15.0f);
						}
						Renderer::GetInstance()->AddShot(m_Position, tgt);
						Renderer::GetInstance()->AddBloodSpray(targetBot.m_Position, targetBot.m_Position-m_Position, damage/5);
					}

				}
				else		// No line of sight
				{
					m_bAiming = false;
					m_dAccuracy =0;				
				}

			}			// End valid target
		}
		else			// Not aiming
		{
			m_dAccuracy =0;
		}
	}
}

void Bot::Reload()
{
  m_iAmmo = MAXAMMO;
}

Vector2D Bot::GetLocation()
{
	return m_Position;
}

float Bot::GetDirection()
{
	return this->m_dDirection;
}

Vector2D Bot::GetVelocity()
{
	return m_Velocity;
}

void Bot::SetOwnNumbers(int teamNo, int botNo)
{
	m_iOwnTeamNumber = teamNo;
	m_iOwnBotNumber = botNo;
}

void Bot::PlaceAt(Vector2D position)
{
	m_Position = position;	// Current world coordinates
	m_Velocity.set(0,0);	// Current velocity
	m_dDirection=0;			// Direction. Mainly useful when stationary
	m_bAiming=false;		// If true, bot is aiming and cannot move
	m_dTimeToCoolDown=0;	// Countdown until the time the bot can shoot again
	m_dTimeToRespawn=0;		// Countdown until the bot can respawn. If zero or below, bot is alive
	m_Acceleration.set(0,0);
	m_bFiring=false;
	m_dAccuracy =0;
	m_iHealth=100;
}

bool Bot::IsAlive()
{
	if(m_dTimeToRespawn<=0)
		return true;
	else
		return false;
}

Bot::Bot()
{
	// I suggest you do nothing here.
	// Remember that the rest of the world may not have been created yet.
	PlaceAt(Vector2D(0,0));		// Places bot at a default location
	m_dTimeToRespawn=1;
}

int Bot::GetHealth()
{
	return m_iHealth;
}

double Bot::GetAccuracy()
{	
	if(m_bAiming== true)
		return m_dAccuracy;
	else
		return 0;
}

void Bot::SetTarget(int targetTeamNo, int targetBotNo)
{

	if(m_iAimingAtTeam!=targetTeamNo || m_iAimingAtBot!=targetBotNo)
	{

		m_iAimingAtTeam = targetTeamNo;

		m_iAimingAtBot = targetBotNo;

		m_dAccuracy=0;
	}

	m_bAiming = true;
}

// Stops the bot from aiming, so it can move again
void Bot::StopAiming()
{
	m_bAiming = false;
}

// Call this to set the bot to shoot, if it can.
// Once a shot it taken, the bot will not shoot again unless told to do so.
void Bot::Shoot()
{
	m_bFiring = true;
}

// Returns the number of the team of the bot being aimed at.
// Returns a negative number if no bot is being aimed at.
int Bot::GetTargetTeam()
{
	if(m_bAiming== true)
		return m_iAimingAtTeam;
	else
		return -1;
}

// Returns the number of the bot being aimed at.
// Returns a negative number if no bot is being aimed at.
int Bot::GetTargetBot()
{
	if(m_bAiming== true)
		return m_iAimingAtBot;
	else
		return -1;
}

void Bot::TakeDamage(int amount)
{
	m_iHealth-=amount;

	if(m_iHealth<=0 && m_dTimeToRespawn<=0)
	{
		m_dTimeToRespawn = RESPAWNTIME;
	}

	// Being shot at puts you off your aim. 
	// Even more so if you are hit

	if(amount>0)
		m_dAccuracy=0;
	else
		m_dAccuracy/=2;
}

// ****************************************************************************

// This is your function. Use it to set up any states at the beginning of the game
// and analyse the map.
// Remember that bots have not spawned yet, so will not be in their
// starting positions.
// Eventually, this will contain very little code - it just sets a starting state
// and calls methods to analyse the map
void Bot::StartAI()
{
	//Set up the map which will be used for the creation of nodes and edges used for A* pathfinding
	Rectangle2D map;
	//Places a rectangle around the map
	map.PlaceAt(1700, -1700, -1700, 1700);
	//Creates all the nodes
	m_graph.AnalyseMap(map);

	//creates all the edges
	m_graph.CheckEdges();
	//initialises the current state and previous state to no state
	currentState = NOSTATE;
	previousState = NOSTATE;
}

//Original Process AI function, without fuzzy logic. Set up to control the bots in a better more efficient
//way than the processAI badly function. Uses states and behaviours to control the bots.
void Bot::ProcessAI()
{
	//Turns WallAvoid on as a behaviour
	m_Acceleration += WallAvoid();

	//If statement which checks if the bots current state is no state, then changes to the capture state
	if (currentState == NOSTATE)
	{
		ChangeState(CAPTURE);
	}

	//if statement which checks and runs the capture state
	if (currentState == CAPTURE)
	{
		CaptureState();
	}

	//if statement which checks and runs the defend state
	if (currentState == DEFEND)
	{
		DefendState();
	}

	//if statement which checks and runs the attack state, stops aiming if not in the attack state
	//(prevents bots from aiming at bots out of range
	if (currentState == ATTACK)
	{
		AttackState();
	}
	else
	{
		StopAiming();
	}

	//if statement which checks and runs rearm state
	if (currentState == REARM)
	{
		RearmState();
	}

	//if statement which checks if the closest enemy is in sight and if they are in range of the enemy and if they are alive, then changes to the attack state
	if (StaticMap::GetInstance()->IsLineOfSight(m_Position, GetClosestEnemy()->m_Position) && (m_Position - GetClosestEnemy()->m_Position).magnitude() < 400
		&& GetClosestEnemy()->IsAlive())
	{
		this->ChangeState(ATTACK);
	}

	//if statement which checks if the bot has any ammo, then changes to the rearm state
	if (this->m_iAmmo <= 0)
	{
		this->ChangeState(REARM);
	}


	//Commented out code used to draw the nodes and/or edges, will draw all nodes and edges, used for debugging
	//m_graph.DrawNodes();
	////m_graph.DrawEdges();

	////Calls drawPath for all bots, each node of the bots path is drawn in a colour based on the bot number.
	////used for debugging
	//drawPath(0);
	//drawPath(1);
	//drawPath(2);
	//drawPath(3);
	//drawPath(4);
	//drawPath(5);	
}

//Similar to above function but uses fuzzy logic when deciding to attack. Enhanced version of the above function
//causes the bots to seek enemies instead of only attacking when in range with fuzzy values instead of boolean values.
void Bot::ProcessEnhancedAI()
{
	//Turns WallAvoid on as a behaviour
	m_Acceleration += WallAvoid();

	//If statement which checks if the bots current state is no state, then changes to the capture state
	if (currentState == NOSTATE)
	{
		ChangeState(CAPTURE);
	}

	//if statement which checks and runs the capture state
	if (currentState == CAPTURE)
	{
		CaptureState();
	}

	//if statement which checks and runs the defend state
	if (currentState == DEFEND)
	{
		DefendState();
	}

	//if statement which checks and runs the attack state, stops aiming if not in the attack state
	//(prevents bots from aiming at bots out of range
	if (currentState == ATTACK)
	{
		AttackState();
	}
	else
	{
		StopAiming();
	}

	//if statement which checks and runs rearm state
	if (currentState == REARM)
	{
		RearmState();
	}

	//Fuzzy logic for attack state, calls the fuzzy beatability function which will return
	//a float between 0 and 1, then runs the attack state if the value is above 0.5
	if (FuzzyBeatability() > 0.5)
	{
		this->ChangeState(ATTACK);
	}


	//if statement which checks if the bot has any ammo, then changes to the rearm state
	if (this->m_iAmmo <= 0)
	{
		this->ChangeState(REARM);
	}


	//Commented out code used to draw the nodes and/or edges, will draw all nodes and edges, used for debugging
	//m_graph.DrawNodes();
	//m_graph.DrawEdges();

	//Calls drawPath for all bots, each node of the bots path is drawn in a colour based on the bot number.
	//used for debugging
	drawPath(0);
	drawPath(1);
	drawPath(2);
	drawPath(3);
	drawPath(4);
	drawPath(5);
}

//FuzzyBeatability Function, used to generate fuzzy values rather than boolean values to decide if the enemy
//should enter the attack state. Generates a float between 0 and 1 for the friendly bots healthyness, the enemy bots
//weakness, and the closeness between the bots, and gets the average of the three values and returns that average.
float Bot::FuzzyBeatability()
{
	//On a scale of 0-1 how healthy is the friendly bot
	float fHealthyness = m_iHealth / 100;
	//on a scale of 0-1 how weak is the closest enemy bot.
	float eWeakness = 1 - GetClosestEnemy()->m_iHealth / 100;
	//on a scale of 0-1 how close is the enemy bot
	float closeness = 1 - (((m_Position - GetClosestEnemy()->m_Position).magnitude()) / 800);

	//if statement which prevents the closeness from ever being above 1
	if (((m_Position - GetClosestEnemy()->m_Position).magnitude()) > 800)
	{
		closeness = 0;
	}

	//beatability is the average of the 3 above values
	float beatability = ((fHealthyness + eWeakness + closeness) / 3);

	//beatability is returned
	return beatability;
}

//returns a pointer to the closest enemy by cycling through each enemy bot and determening the closest one
Bot* Bot::GetClosestEnemy()
{
	//closest enemy index/botnumber, initialised to 0
	int closestEnemy = 0;
	//sets the range to be the distance between the bot and the initialial enemy
	double range = (DynamicObjects::GetInstance()->GetBot(1, 0).m_Position - m_Position).magnitude();

	//if the bot is dead, increase the range by a large amount (allows a new closest enemy to be found)
	if (!DynamicObjects::GetInstance()->GetBot(1, 0).IsAlive())
	{
		range += 10000000;
	}

	//For loop which loops through the rest of the bots and determines which ones is closest using the method
	//above.
	for (int i = 1; i<6; i++)
	{
		double nextRange = (DynamicObjects::GetInstance()->GetBot(1, i).m_Position - m_Position).magnitude();
		if (!DynamicObjects::GetInstance()->GetBot(1, i).IsAlive())
		{
			nextRange += 10000000;
		}

		//if the range is smallet than the current range, set the new enemy index
		if (nextRange<range)
		{
			closestEnemy = i;
			range = nextRange;
		}
	}

	//return the enemy with the index of closest enemy
	return &DynamicObjects::GetInstance()->GetBot(1, closestEnemy);
}

//Takes a state as a parameter and checks that it is not already in that state and changes the state and sets the previous state
void Bot::ChangeState(State nextState)
{
	if (nextState != currentState)
	{
		previousState = currentState;
		currentState = nextState;
	}
}

//Rearm state, used to tell the bot to find hte nearest resupply point and aquire more ammo.
void Bot::RearmState()
{
	//target Vector2D created, will hold the nearest resupply location
	Vector2D target;

	//target set to the nearest resupply location to the bot
	target = StaticMap::GetInstance()->GetClosestResupplyLocation(this->m_Position);

	//path is set using the Pathfind function between the bot and the target
	this->path = m_graph.Pathfind(m_Position, target);

	//if the path is not empty, and the target is not in sight, use the FollowPath behaviour,
	//else, use the seek behaviour, passing in the target
	if ((path.size() > 0) && (!(StaticMap::GetInstance()->IsLineOfSight(m_Position, target))))
	{
		this->m_Acceleration += FollowPath();
	}
	else
	{
		this->m_Acceleration += Seek(target);
	}

	//if the ammo has been resupplied, change to the capture state
	if (this->m_iAmmo > 0)
	{
		this->ChangeState(CAPTURE);
	}

}

//defend state, makes the bot stay near one of their domination points if they own it.
void Bot::DefendState()
{
	//target domination point created, will hold that bots target dom point
	DominationPoint target;

	//if else statements which assign 2 bots per domination point
	if (m_iOwnBotNumber == 0 || m_iOwnBotNumber == 1)
	{
		target = DynamicObjects::GetInstance()->GetDominationPoint(0);
	}
	else if (m_iOwnBotNumber == 2 || m_iOwnBotNumber == 3)
	{
		target = DynamicObjects::GetInstance()->GetDominationPoint(1);
	}
	else if (m_iOwnBotNumber == 4 || m_iOwnBotNumber == 5)
	{
		target = DynamicObjects::GetInstance()->GetDominationPoint(2);
	}

	//if the domination point is owned by the bots team, follow the path to the target using the behaviours,
	//else change to the capture state
	if (target.m_OwnerTeamNumber == 0)
	{
		this->path = m_graph.Pathfind(m_Position, target.m_Location);

		if ((path.size() > 0) && (!(StaticMap::GetInstance()->IsLineOfSight(m_Position, target.m_Location))))
		{
			this->m_Acceleration += FollowPath();
		}
		else
		{
			this->m_Acceleration += Arrive(target.m_Location);
		}
	}
	else
	{
		ChangeState(CAPTURE);
	}

}

//attack state used to travel to the closest bot, stop, aim and shoot at it
void Bot::AttackState()
{
	//path is set from the bot, to the closest enemy bot
	this->path = m_graph.Pathfind(m_Position, GetClosestEnemy()->m_Position);

	//if statement which makes the player travel towards the player, if there is none in sight, change to capture state
	if ((path.size() > 0) && (!(StaticMap::GetInstance()->IsLineOfSight(m_Position, GetClosestEnemy()->m_Position))))
	{
		this->m_Acceleration += FollowPath();
	}
	else if (StaticMap::GetInstance()->IsLineOfSight(m_Position, GetClosestEnemy()->m_Position))
	{
		this->m_Acceleration += Seek(GetClosestEnemy()->m_Position);
	}
	else
	{
		this->ChangeState(CAPTURE);
	}

	//if statement which checks if the closest enemy is alive, in sight, and in a certain range, then halts the movement, sets
	//that enemy as the target, checks the accuracy and shoots. else, stops aiming and changes to the capture state
	if (StaticMap::GetInstance()->IsLineOfSight(m_Position, GetClosestEnemy()->m_Position)&& GetClosestEnemy()->IsAlive() && 
			(GetClosestEnemy()->m_Position - m_Position).magnitude() < 400)
	{
		//halts the player
		this->m_Acceleration = -m_Velocity;

		//Sets the target as the closest enemy
		this->SetTarget(1, GetClosestEnemy()->m_iOwnBotNumber);

		//performs accuracy checks then shoots
		if (m_dAccuracy>0.7 || (GetClosestEnemy()->m_bAiming && GetClosestEnemy()->m_dTimeToCoolDown<0.1 && m_dAccuracy>0.3))
		{
			Shoot();
		}
	}
	else
	{
		//stops aiming so the player can move
		this->StopAiming();
		//changes to the capture state
		this->ChangeState(CAPTURE);
	}
}

//capture state, gives the bot a specific domination point to go capture if not owned by the bots team.
void Bot::CaptureState()
{
	//target domination point created, will hold that bots target dom point
	DominationPoint target;

	//if else statements which assign 2 bots per domination point
	if (m_iOwnBotNumber == 0 || m_iOwnBotNumber == 1)
	{
		target = DynamicObjects::GetInstance()->GetDominationPoint(0);
	}
	else if (m_iOwnBotNumber == 2 || m_iOwnBotNumber == 3)
	{
		target = DynamicObjects::GetInstance()->GetDominationPoint(1);
	}
	else if (m_iOwnBotNumber == 4 || m_iOwnBotNumber == 5)
	{
		target = DynamicObjects::GetInstance()->GetDominationPoint(2);
	}

	//if the target is not owned by the bots team, set a path to the target, else, change to the defend state
	if (target.m_OwnerTeamNumber != 0)
	{
		this->path = m_graph.Pathfind(m_Position, target.m_Location);

		if ((path.size() > 0) && (!(StaticMap::GetInstance()->IsLineOfSight(m_Position, target.m_Location))))
		{
			this->m_Acceleration += FollowPath();
		}
		else
		{
			this->m_Acceleration += Seek(target.m_Location);
		}
	}
	else
	{
		this->ChangeState(DEFEND);
	}	
}

//draw path function, used for debugging, but draws each node in the bots path using their number as a colour
void Bot::drawPath(int botNum)
{
	if (m_iOwnBotNumber == botNum)
	{
		for (int i = 0; i < path.size(); i++)
		{
			Renderer::GetInstance()->DrawDot(path[i], botNum);
		}
	}
}

//Bad Enemy AI, written badly, meant to be beat by my AI code most of the time
void Bot::ProcessAIBadly()
{
	
	// This is all placeholder code.
	// Delete it all and write your own

	DominationPoint targetDP = DynamicObjects::GetInstance()
		->GetDominationPoint(m_iOwnBotNumber%3);
	
	// Find closest enemy
	int closestEnemy = 0;
	double range = (DynamicObjects::GetInstance()->GetBot
		(1-m_iOwnTeamNumber, 0).m_Position - m_Position).magnitude();
  if (DynamicObjects::GetInstance()->GetBot(1 - m_iOwnTeamNumber, 0).m_iHealth < 0)
    range += 1000;

	for(int i=1;i<NUMBOTSPERTEAM;i++)
	{
		double nextRange = (DynamicObjects::GetInstance()->GetBot
			(1-m_iOwnTeamNumber, i).m_Position - m_Position).magnitude();
    if (DynamicObjects::GetInstance()->GetBot(1 - m_iOwnTeamNumber, 0).m_iHealth < 0)
      nextRange += 1000;
		if(nextRange<range)
		{
			closestEnemy = i;
			range=nextRange;
		}
	}

	Bot& targetEnemy = DynamicObjects::GetInstance()->
		GetBot(1-m_iOwnTeamNumber, closestEnemy);

  Circle2D loc;
  loc.PlaceAt(m_Position, 30);
  
  // Out of ammo?
  if (m_iAmmo<=0)
  {
    // Find closest resupply
    Vector2D closestSupply = StaticMap::GetInstance()->GetClosestResupplyLocation(m_Position);

    if (StaticMap::GetInstance()->IsLineOfSight(m_Position, closestSupply))
    {
      targetPoint = closestSupply;
    }
    else
    {
      // Pick a random point
      Vector2D randomPoint(rand() % 4000 - 2000.0f, rand() % 4000 - 2000.0f);

      // Is it better than current target point?
      double currentValue = (m_Position - targetPoint).magnitude()
        + (closestSupply - targetPoint).magnitude()*1.2;
      if (!StaticMap::GetInstance()->IsLineOfSight(m_Position, targetPoint))
      {
        currentValue += 2500;
      }
      if (!StaticMap::GetInstance()->IsLineOfSight(closestSupply, targetPoint))
      {
        currentValue += 1000;
      }
      // Is it too close to a block?
      Circle2D loc;
      loc.PlaceAt(randomPoint, 20);
      if (StaticMap::GetInstance()->IsInsideBlock(loc))
      {
        currentValue += 500;
      }

      double randomValue = (m_Position - randomPoint).magnitude()
        + (closestSupply - randomPoint).magnitude()*1.2;
      if (!StaticMap::GetInstance()->IsLineOfSight(m_Position, randomPoint))
      {
        randomValue += 1500;
      }
      if (!StaticMap::GetInstance()->IsLineOfSight(closestSupply, randomPoint))
      {
        randomValue += 1000;
      }

      if (randomValue < currentValue)
        // Set as target point
      {
        targetPoint = randomPoint;
      }
    }

    Vector2D desiredVelocity = (targetPoint - m_Position).unitVector() * MAXBOTSPEED;
    m_Acceleration = (desiredVelocity - m_Velocity);
//   	Renderer::GetInstance()->DrawDot(targetPoint);
    StopAiming();
    // Bounce off walls

    Circle2D bounds(m_Position, 50);

    if (StaticMap::GetInstance()->IsInsideBlock(bounds))
    {
      m_Acceleration = StaticMap::GetInstance()->GetNormalToSurface(bounds)*MAXIMUMACCELERATION;
    }
  }

	 //Closest enemy within range?
	else if( StaticMap::GetInstance()->IsLineOfSight( m_Position, targetEnemy.m_Position)
		&& targetEnemy.IsAlive() && range<400)
	{
		// Kill it
		SetTarget(1-m_iOwnTeamNumber, closestEnemy);

    if (m_dAccuracy>0.7 || (targetEnemy.m_bAiming && targetEnemy.m_dTimeToCoolDown<0.1 &&m_dAccuracy>0.3))
		{
			Shoot();
		}
	}

  else if (StaticMap::GetInstance()->IsInsideBlock(loc))    // Too close to a wall
  {
    loc.PlaceAt(targetPoint, 30);
    if ((m_Position - targetPoint).magnitude()<45
      || StaticMap::GetInstance()->IsInsideBlock(loc))
    {
      targetPoint = m_Position + Vector2D(rand() % 60 - 30.0f, rand() % 60 - 30.0f);
    }
  }
	else    // Go to domination point
	{
    // If dom point is visible, within 400 units and owned
    if (StaticMap::GetInstance()->IsLineOfSight(m_Position, targetDP.m_Location)
      && (m_Position-targetDP.m_Location).magnitude()<400 &&
      targetDP.m_OwnerTeamNumber == m_iOwnTeamNumber)
    {
      // Stand and kill nearest target
      m_Acceleration = -m_Velocity;
      SetTarget(1 - m_iOwnTeamNumber, closestEnemy);
      if (m_dAccuracy>0.5)
      {
        Shoot();
      }
    }
    // If dom point is visible
		// Dock
		else if(StaticMap::GetInstance()->IsLineOfSight( m_Position, targetDP.m_Location))
		{
			// Target is dom point
      targetPoint = targetDP.m_Location;
      StopAiming();
		}
    else // Navigate to it
		{
			// Pick a random point
			Vector2D randomPoint(rand()%4000-2000.0f, rand()%4000-2000.0f);

			// Is it better than current target point?
			double currentValue = (m_Position - targetPoint).magnitude()
				+ (targetDP.m_Location - targetPoint).magnitude()*1.2;
			if(!StaticMap::GetInstance()->IsLineOfSight(m_Position, targetPoint))
			{
				currentValue+=2500;
			}
			if(!StaticMap::GetInstance()->IsLineOfSight(targetDP.m_Location, targetPoint))
			{
				currentValue+=1000;
			}
      // Is it too close to a block?
      Circle2D loc;
      loc.PlaceAt(randomPoint, 20);
      if (StaticMap::GetInstance()->IsInsideBlock(loc))
      {
        currentValue += 500;
      }

			double randomValue = (m_Position - randomPoint).magnitude()
					+ (targetDP.m_Location - randomPoint).magnitude()*1.2;
			if(!StaticMap::GetInstance()->IsLineOfSight(m_Position, randomPoint))
			{
				randomValue+=1500;
			}
			if(!StaticMap::GetInstance()->IsLineOfSight(targetDP.m_Location, randomPoint))
			{
				randomValue+=1000;
			}

			if(randomValue< currentValue)
			// Set as target point
			{
				targetPoint = randomPoint;
			}
      StopAiming();
		}

		// Head for target point
	//	Renderer::GetInstance()->DrawDot(targetPoint);
		Vector2D desiredVelocity = (targetPoint-m_Position).unitVector() * MAXBOTSPEED;
		m_Acceleration = (desiredVelocity-m_Velocity);

		// Bounce off walls
		Circle2D bounds(m_Position, 50);
		if(StaticMap::GetInstance()->IsInsideBlock(bounds))
		{
			m_Acceleration= StaticMap::GetInstance()->GetNormalToSurface(bounds)*MAXIMUMACCELERATION;
		}

	}
	
}

//Seek behaviour, tages a vector2D as a target and applies physics to return a vector
//that will move the player towards the target
Vector2D Bot::Seek(Vector2D target)
{
	Vector2D desiredVelocity = (target - m_Position).unitVector() * MAXBOTSPEED ;

	Vector2D behaviourAccn = desiredVelocity - m_Velocity;
	return behaviourAccn;
}

//flee behaviour, takes a vector2D as a target, similar to seek but returns the oppposite vector
//to move the player away from the vector
Vector2D Bot::Flee(Vector2D target)
{
	Vector2D desiredVelocity = (target - m_Position).unitVector() * MAXBOTSPEED;

	Vector2D behaviourAccn = desiredVelocity - m_Velocity;
	behaviourAccn.XValue *= -1;
	return behaviourAccn;
}

//Arrive behaviour, similar to seek, but slows the player down when target is almost reached
Vector2D Bot::Arrive(Vector2D target)
{
	float slowDown = ((target - m_Position).magnitude() / 2);
	
	if (slowDown >= MAXBOTSPEED)
	{
		slowDown = MAXBOTSPEED;
	}

	Vector2D desiredVelocity = (target - m_Position).unitVector() * slowDown;

	Vector2D behaviourAccn = desiredVelocity - m_Velocity;
	return behaviourAccn;
}

//pursue behaviour, takes a target position and a target velocity and
//predicts where the target will be positioned and moves seeks the predicted area
Vector2D Bot::Pursue(Vector2D targetPos, Vector2D targetVel)
{
	double distance = (targetPos - m_Position).magnitude();

	float time = distance / MAXBOTSPEED;

	Vector2D target = (targetPos + (targetVel *time));

	return Seek(target);
}

//Evade, similar to pursue but uses flee instead of seek
Vector2D Bot::Evade(Vector2D targetPos, Vector2D targetVel)
{
	double distance = (targetPos - m_Position).magnitude();

	float time = distance / MAXBOTSPEED;

	Vector2D target = (targetPos + targetVel)* time;

	return Flee(target);
}

//Wall avoid behaviour, used to avoid walls, creates a circle infront of the player which will detect 
//collisions with walls and return appropriate normal vectors.
Vector2D Bot::WallAvoid()
{
	Circle2D b1;
	b1.PlaceAt(m_Position, 80.0f);
	
	return (650.0f * StaticMap::GetInstance()-> GetNormalToSurface(b1));
}

//follow path behaviour, seeks the players path node by node and pops the current node
//if a certain distance from it, if there is no path, halts the player.
Vector2D Bot::FollowPath()
{
	Vector2D answer;
	int s = path.size();

	if (!path.empty())
	{		
		answer = Seek(path[s-1]);

		if (((((m_Position - path[s - 1]).magnitude()) < 50)))
		{
			path.pop_back();
		}
	}
	else
	{
		answer = Seek(m_Position);
	}

	return answer;
}

