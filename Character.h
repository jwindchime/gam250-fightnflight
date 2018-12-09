// Copyright © 2017 DigiPen (USA) Corporation.
/*!
*******************************************************************************
\file    Character.h
\author  James Liao
\par     email: james.liao\@digipen.edu
\par     Course: GAM200F17-A
\brief   Implementation for all Characters in the game.
*******************************************************************************/

#pragma once
#include "Entity.h"
#include "Collider.h"
#include "CollisionLayer.h"
#include "Fist.h"
#include <set>

using namespace fb;

enum Direction;

class Character
{
  public:
    /*!
    *******************************************************************************
    \brief  Constructor
    \param  index
      The ID the character should be set to (int).
    *******************************************************************************/
    Character(int index);

    /*!
    *******************************************************************************
    \brief  Destructor
    *******************************************************************************/
    ~Character();

    /*!
    *******************************************************************************
    \brief   Virtual function for how the character should jump
    \return  None (void).
    *******************************************************************************/
    void jump(Direction direction);

    /*!
    *******************************************************************************
    \brief   Virtual function for how the character should perform a basic attack
    \param   direction
    The direction the attack should be performed in (Direction).
    \return  None (void).
    *******************************************************************************/
    void basicAttack(Direction direction);

    /*!
    *******************************************************************************
    \brief   Virtual function for how the character should perform a special attack
    \param   direction
    The direction the attack should be performed in (Direction).
    \return  None (void).
    *******************************************************************************/
    void specialAttack(Direction direction);

    /*!
    *******************************************************************************
    \brief   Virtual function for how the character should block attacks
    \return  None (void).
    *******************************************************************************/
    void block();

    /*!
    *******************************************************************************
    \brief   Virtual function for how the character should move
    \param   direction
      The direction the character should move in (Direction).
    \return  None (void).
    *******************************************************************************/
    void move(Direction direction);

    /*!
    *******************************************************************************
    \brief   Attach an entity to the character
    \param   entity
      Pointer to the entity this character will be responsible for (shared_ptr<fb::Entity>).
    \return  None (void).
    *******************************************************************************/
    void attachEntity(std::shared_ptr<Entity> entity);

    /*!
    *******************************************************************************
    \brief   returns the entity attached to the character
    \return  The attached entity (shared_ptr<Entity>).
    *******************************************************************************/
    std::shared_ptr<Entity> getEntity();
   
    /*!
    *******************************************************************************
    \brief   Return whether or not the character can jump
    \return  True if the character can jump, False otherwise (bool).
    *******************************************************************************/
    bool canJump();

    /*!
    *******************************************************************************
    \brief   Return whether or not the character is on the floor
    \return  True if the character is on the floor, False otherwise (bool).
    *******************************************************************************/
    bool isOnFloor();

    /*!
    *******************************************************************************
    \brief   Return whether or not the character has jumped for the first time
    \return  True if this is the first jump, False otherwise (bool).
    *******************************************************************************/
    bool isFirstJump();

    /*!
    *******************************************************************************
    \brief   Return whether or not the character has been hit
    \return  True if the character has been hit, False otherwise (bool).
    *******************************************************************************/
    bool canMove();

    /*!
    *******************************************************************************
    \brief   Lets the character jump again
    \return  None (void).
    *******************************************************************************/
    void removeLimiter();

    /*!
    *******************************************************************************
    \brief   Prevents the character from jumping
    \return  None (void).
    *******************************************************************************/
    void addLimiter();

    /*!
    *******************************************************************************
    \brief   Returns the character's ID
    \return  The character ID (int).
    *******************************************************************************/
    int GetID();

    /*!
    *******************************************************************************
    \brief   Set the character's onFloor flag
    \param   floor
      Whether or not the character should be on the ground (bool).
    \return  None (void).
    *******************************************************************************/
    void setOnFloor(bool floor);

    /*!
    *******************************************************************************
    \brief   Set the character's canJump_ flag
    \param   jump
      Whether or not the character can double jump (bool).
    \return  None (void).
    *******************************************************************************/
    void setJump(bool jump);

    /*!
    *******************************************************************************
    \brief   Set the character's firstJump flag
    \param   first
      Whether or not the character is jumping for the first time (bool).
    \return  None (void).
    *******************************************************************************/
    void setFirstJump(bool first);

    /*!
    *******************************************************************************
    \brief   Set the character's isHit flag
    \param   gotHit
      Whether or not the character got hit (bool).
    \return  None (void).
    *******************************************************************************/
    void setHit(bool gotHit);

    /*!
    *******************************************************************************
    \brief   Get the global acceleration
    \return  The acceleration (float).
    *******************************************************************************/
    static float GetAcceleration();

    /*!
    *******************************************************************************
    \brief   Get the global jump speed
    \return  The jump speed (float).
    *******************************************************************************/
    static float GetJumpSpeed();

    /*!
    *******************************************************************************
    \brief   Get the global max speed
    \return  The max spee (float).
    *******************************************************************************/
    static float GetMaxSpeed();

    /*!
    *******************************************************************************
    \brief   Get the global gravity
    \return  The gravity (float).
    *******************************************************************************/
    static float GetGravity();

    /*!
    *******************************************************************************
    \brief   Get the global air resistance
    \return  The air resistance (int).
    *******************************************************************************/
    static int GetDrag();

    /*!
    *******************************************************************************
    \brief   Set the global acceleration
    \param   speed
      The new acceleration (float).
    \return  None (void).
    *******************************************************************************/
    static void SetAcceleration(float speed);

    /*!
    *******************************************************************************
    \brief   Set the global jump speed
    \param   speed
      The new jump speed (float).
    \return  None (void).
    *******************************************************************************/
    static void SetJumpSpeed(float speed);

    /*!
    *******************************************************************************
    \brief   Set the global max speed
    \param   speed
      The new max speed (float).
    \return  None (void).
    *******************************************************************************/
    static void SetMaxSpeed(float speed);

    /*!
    *******************************************************************************
    \brief   Set the global gravity
    \param   speed
      The new gravity (float).
    \return  None (void).
    *******************************************************************************/
    static void SetGravity(float grav);

    /*!
    *******************************************************************************
    \brief   Set the global air resistance
    \param   speed
      The new air resistance (int).
    \return  None (void).
    *******************************************************************************/
    static void SetDrag(int resistance);

    /*!
    *******************************************************************************
    \brief   Get the punch cooldown timer
    \return  None (void).
    *******************************************************************************/
    float GetPunchTimer();

    /*!
    *******************************************************************************
    \brief   Ticks the punch cooldown timer down.
    \param   dt
      The change in time since last update (float).
    \return  None (void).
    *******************************************************************************/
    void TickPunchTimer(float dt);

    /*!
    *******************************************************************************
    \brief   Resets the punch cooldown timer
    \return  None (void).
    *******************************************************************************/
    void ResetPunchTimer();

    int getSlimeBagWeight();

    float getZoneTimer();

    void tickZoneTimer(float dt);

    void setZoneTimer(float time);

    int getSlimeBagCapacity();

    const std::vector<int> & getSlimeBag();

    int addSlime(int weight);

    void clearSlimeBagWeight();

    void removeSlime(int weight);

    int popSlime();

    bool passThrough_; //!< Is the player able to pass through passable platforms?

  private:
    /*!
    *******************************************************************************
    \brief   Play a jump sound
    \return  None (void).
    *******************************************************************************/
    void PlayJumpSound();

    /*!
    *******************************************************************************
    \brief   Play a double jump sound
    \return  None (void).
    *******************************************************************************/
    void PlayDoubleJumpSound();

    /*!
    *******************************************************************************
    \brief   Play a wall jump sound
    \return  None (void).
    *******************************************************************************/
    void PlayWallJumpSound();

    /*!
    *******************************************************************************
    \brief   Play a punch sound
    \return  None (void).
    *******************************************************************************/
    void PlayPunchHitSound();

    void PlayPunchMissSound();
    
    void PlaySlimePickupSound();

    void PlaySlimeFullSound();
    
   

    /*!
    *******************************************************************************
    \brief   Play a move sound
    \return  None (void).
    *******************************************************************************/
    void PlayMoveSound();

    std::shared_ptr<Entity> entity_; //!< The entity the character should be acting upon

    bool canJump_; //!< Whether or not the player can double jump
    bool canPunch_; //!< Whether or not the player can punch
    bool onFloor_; //!< Whether or not the player is standing on a platform
    bool isHit_; //!< Whether or not the player has been hit (with a punch)
    bool terminalVelocity; //!< Whether or not the player has reached the maximum jump speed
    bool firstJump; //!< Whether or not the player has jumped once
    int id; //!< The character's ID
    int slimeBagCapacity; //!< How many slimes the character can hold.
    int slimeBagSize; //!< How many slimes the character is holding.
    int currentSlimeScore; //!< How many slimes the character is holding score wise.
    float zoneTimer; //!< How long the player needs to be in the zone.
    float punchTimer; //!< Cooldown between punches
    std::vector<int> slimeBag; //!< The bag of slimes.

    static float acceleration; //!< How much character speed increases per tick
    static float jumpSpeed; //!< The jump speed of the character
    static float maxSpeed; //!< The max speed of the character
    static float gravity; //!< How strong gravity will act
    static int drag; //!< How much character speed should be cut in midair
};
