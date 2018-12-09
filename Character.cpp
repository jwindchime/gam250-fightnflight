// Author:   James Liao
// Copyright © 2017 DigiPen (USA) Corporation.
#include "Character.h"
#include "PhysicsManager.h"
#include "CharacterHandler.h"
#include "ControllerHandler.h"
#include "Transform.h"
#include "EventManager.h"
#include "MakeParticles.h"
#include "Sprite.h"
#include "DudeAI.h"
#include "PopupText.h"
#include "AdvancedBody.h"
#include "PunchFX.h"

#include "ControllerHandler.h"

#define JUMP_MOD 5
#define PUNCH_COOLDOWN 0.5f

// If any of these values go through, something went wrong with the JSON loading
float Character::acceleration = 0.0f;
float Character::jumpSpeed = 0.0f;
float Character::maxSpeed = 0.0f;
float Character::gravity = 0.0f;
int Character::drag = 1;

Character::Character(int index)
{
  entity_ = NULL;
  canJump_ = false;
  canPunch_ = false;
  onFloor_ = true;
  isHit_ = false;
  terminalVelocity = false;
  firstJump = false;
  id = index;
  passThrough_ = false;
  currentSlimeScore = 0;
  slimeBagCapacity = 0;
  zoneTimer = 1.0f;
  punchTimer = 0.0f;
  slimeBagCapacity = 5;
}

Character::~Character()
{
  // Sets the attached entity to NULL but DOES NOT delete the entity itself
  entity_ = NULL;
}

void Character::jump(Direction direction)
{
  // Allows the character to continue moving while in the "jumping" state
  move(direction);

  // Prevent player from jumping past the max speed
  if (terminalVelocity)
  {
    return;
  }

  // Standard values to be used by all characters
  float xSpeed = maxSpeed;
  float ySpeed = jumpSpeed;
  float xScale = 0.8f;

  auto body = entity_->GetComponent<cmp::AdvancedBody>();
  vec2 newVelocity = body->GetVelocity();

  if (direction != Down)
  {
    if (onFloor_ || canJump_ || firstJump) { newVelocity.y = fminf(newVelocity.y + ySpeed / JUMP_MOD, ySpeed); }

    if (onFloor_ && !firstJump)
    {
      // Play the jump sound
      PlayJumpSound();

      // Show a jump effect
      MakeJumpParticle(2.0f / 5.0f, entity_->GetComponent<cmp::Transform>()->GetPosition());

      entity_->AttachComponent(std::make_shared<PunchFX>(PunchFX(0.0f, 0.25f, 0.5f)));
    }
    else
    {
      // Check if we're on the left wall
      if (body->RunDetection(world, cmp::LEFT).collision)
      {
        // Play the wall jump sound
        PlayWallJumpSound();

        // Show a jump effect
        //MakeJumpParticle(2.0f / 5.0f, entity_->GetComponent<cmp::Transform>()->GetPosition());

        // Squish n Stretch
        entity_->AttachComponent(std::make_shared<PunchFX>(PunchFX(0.0f, 0.25f, 0.5f)));

        // Jump more vertically if the player is holding into the wall
        if (direction == Left)
        {
          newVelocity.x = xSpeed * xScale;
          newVelocity.y = ySpeed * 1.0f;
        }

        // Jump more horizontally if the player is not holding into the wall
        else
        {
          newVelocity.x = xSpeed;
          newVelocity.y = ySpeed * 1.0f;
        }

        entity_->GetComponent<Sprite>()->SetFlipped(direction == Left);

        // Prevent the player from double-jumping immediately off a wall
        firstJump = false;
      }

      // Check if we're on the right wall
      else if (body->RunDetection(world, cmp::RIGHT).collision)
      {
        // Play the wall jump sound
        PlayWallJumpSound();

        // Show a jump effect
        //MakeJumpParticle(2.0f / 5.0f, entity_->GetComponent<cmp::Transform>()->GetPosition());

        // Squish n Stretch
        entity_->AttachComponent(std::make_shared<PunchFX>(PunchFX(0.0f, 0.25f, 0.5f)));

        // Jump more vertically if the player is holding into the wall
        if (direction == Right)
        {
          newVelocity.x = -xSpeed * xScale;
          newVelocity.y = ySpeed * 1.0f;
        }
        // Jump more horizontally if the player is not holding into the wall
        else
        {
          newVelocity.x = -xSpeed;
          newVelocity.y = ySpeed * 1.0f;
        }

        entity_->GetComponent<Sprite>()->SetFlipped(direction != Right);

        // Prevent the player from double-jumping immediately off a wall
        firstJump = false;
      }
      else if (canJump_ && firstJump)
      {
        // Play the double jump sound
        PlayDoubleJumpSound();

        // Show a jump effect

        auto trans = entity_->GetComponent<cmp::Transform>();

        vec2 pos = trans->GetPosition() - vec2(0, (trans->GetScale().y / 2.0f));

        MakeDoubleJumpParticle(2.5f, pos, !entity_->GetComponent<Sprite>()->IsFlipped());
        
        entity_->AttachComponent(std::make_shared<PunchFX>(PunchFX(0.0f, 0.25f, 0.5f)));

        newVelocity.y = fmaxf(newVelocity.y, ySpeed / JUMP_MOD);
        newVelocity.y = fminf(newVelocity.y + ySpeed / JUMP_MOD, ySpeed);

        canJump_ = false;
      }
    }
  }
  else
  {
    //Jump down
    this->passThrough_ = true;

    //Prevents the player from double-jumping mid fall-through until they release the button
    terminalVelocity = true;
  }

  if (newVelocity.y >= ySpeed)
  {
    terminalVelocity = true;
  }

  body->SetVelocity(newVelocity);
}

void Character::basicAttack(Direction direction)
{
  move(direction);

  // Do nothing if the punch is still on cooldown
  if (punchTimer > 0)
  {
    return;
  }

  ResetPunchTimer();

  // Setup a temporary hitbox
  CollisionLayer hitLayer(base, user | king);
  BoxCollider* hitBox;
  CollisionResult result;
  auto transform = entity_->GetComponent<cmp::Transform>();

  hitBox = PhysicsManager::GetBoxCollider();
  hitBox->SetParent(entity_);
  hitBox->SetLayer(hitLayer);
  hitBox->SetBound(false);

  float hitBoxSize = 1.2f;
  float posScale = 0.8f;

  // Set the hitbox center depending on where the player wants to punch
  switch (direction)
  {
    case Left:
      hitBox->SetDimensions({ hitBoxSize, 0.5f });
      hitBox->SetCenter({ -hitBoxSize * posScale, 0.0f });
      break;

    case Right:
      hitBox->SetDimensions({ hitBoxSize, 0.5f });
      hitBox->SetCenter({ hitBoxSize * posScale, 0.0f });
      break;

    case Up:
      hitBox->SetDimensions({ 0.5f, hitBoxSize });
      hitBox->SetCenter({ 0.0f, hitBoxSize * posScale });
      break;

    case Down:
      hitBox->SetDimensions({ 0.5f, hitBoxSize });
      hitBox->SetCenter({ 0.0f, -hitBoxSize  * posScale });
      break;

    default:
      if (entity_->GetComponent<Sprite>()->IsFlipped())
      {
        hitBox->SetDimensions({ hitBoxSize, 0.5f });
        hitBox->SetCenter({ -hitBoxSize * posScale, 0.0f });
      }
      else
      {
        hitBox->SetDimensions({ hitBoxSize, 0.5f });
        hitBox->SetCenter({ hitBoxSize * posScale, 0.0f });
      }
  }

//        evt::CharacterEvent charEvent;
//    charEvent.characterEntity = entity_;
//    charEvent.type = evt::punch;
 
  if (!entity_->GetComponent<Sprite>()->IsFlipped())
    MakePunchParticle(5.0f, transform->GetPosition() + glm::vec2(0.5f * transform->GetScale().x, 0.0f));
  else
    MakePunchParticle(5.0f, transform->GetPosition() - glm::vec2(0.5f * transform->GetScale().x, 0.0f));
    

  // Check if the hitbox hits anything
  result = PhysicsManager::RunCollision(*hitBox);
  if (result.collision)
  {
    // boxCollisions is an array of 5 BoxColliders, there's probably a less magical way of doing this
    for (int i = 0; i < 5; i++)
    {
      const BoxCollider* collider = result.boxCollisions[i];

      // If a collider exists, do stuff
      if (collider)
      {
        std::shared_ptr<Entity> entity = collider->GetParent().lock();

        if (entity != nullptr)
        {
          // Get the Transform of the entity so we can see its position
          vec2 position = entity->GetComponent<cmp::Transform>()->GetPosition();

          int charID = 5;

          // Get the ID of the character we're hitting (there should be a better way to do this)
          if (entity->GetName() == "Player 0" && id != 0)
          {
            charID = 0;
          }
          else if (entity->GetName() == "Player 1" && id != 1)
          {
            charID = 1;
          }
          else if (entity->GetName() == "Player 2" && id != 2)
          {
            charID = 2;
          }
          else if (entity->GetName() == "Player 3" && id != 3)
          {
            charID = 3;
          }
          else if (entity->GetName() == "aliengiant")
          {
            charID = 4;
          }

          if (charID < 4)
          {
            // Prevent the fighter from perma-stunning players to a degree
            if (CharacterManager::GetCharacter(charID)->canMove())
            {
              // Play the punch sound
              PlayPunchHitSound();
              CamManager::CamShake::Set(0.05f, 0.015f);
              ControllerManager::GetController(charID)->VibrateController(0.5f, 1.0f, 0.2f);
              ControllerManager::GetController(id)->VibrateController(0.5f, 1.0f, 0.2f);

              // Knockback speeds
              float xSpeed = maxSpeed * 1.5f;
              float ySpeed = jumpSpeed / 2;

              auto body = entity->GetComponent<cmp::AdvancedBody>();

              // Push characters based on which direction we're hitting
              switch (direction)
              {
              case Right:
                body->SetVelocity({ xSpeed, ySpeed });
                break;

              case Left:
                body->SetVelocity({ -xSpeed, ySpeed });
                break;

              default:
                // If the entity is to the right of us, push the entity to the right
                if (position.x > entity_->GetComponent<cmp::Transform>()->GetPosition().x)
                {
                  body->SetVelocity({ xSpeed, ySpeed });
                }

                // Otherwise, push the entity to the left
                else
                {
                  body->SetVelocity({ -xSpeed, ySpeed });
                }
              }

              body->GetParent().lock()->AttachComponent(std::make_shared<PunchFX>(PunchFX(10.0f, 0.25f, 0.5f)));
            }

            Character* player = CharacterManager::GetCharacter(charID);
            // Set that the character has been hit
            player->setHit(true);

            // Dropping slimes go here
            bool superPunch = false;
            int max = 2;
            if (superPunch)
              max = 5;

            for (int i = 0; i < max; i++)
            {
              int weight = 0;
              weight = player->popSlime();
              glm::vec2 position = player->getEntity()->GetComponent<fb::cmp::Transform>()->GetPosition();

              if (weight == 5)
              {
                DudeAI::dropSlime(position, SLIMEGOLDEN, "CharacterTrail" + std::to_string(player->id + 1));
              }
              else if (weight == 1)
              {
                DudeAI::dropSlime(position, SLIMENORMAL, "CharacterTrail" + std::to_string(player->id + 1));
             }
             
            }
          }

          //Punched big slime
          else if (charID == 4)
          {
            // Play the punch sound
            PlayPunchHitSound();

            // Vibrate the controller
            ControllerManager::GetController(id)->VibrateController(0.5f, 1.0f, 0.2f);
            CamManager::CamShake::Set(0.05f, 0.015f);

            //Shoot a slime to the players feet
            glm::vec2 playerPosition = getEntity()->GetComponent<fb::cmp::Transform>()->GetPosition();
            glm::vec2 entityPosition = entity->GetComponent<fb::cmp::Transform>()->GetPosition();
            glm::vec2 force = playerPosition - entityPosition;

            DudeAI::DamageDude(entity, 2, force);
          }
          else
          {
            PlayPunchMissSound();
          }
        }
      }
    }
  }
  else
  {
    PlayPunchMissSound();
  }

  // Return the hit box collider once we're done with it
  PhysicsManager::FreeBoxCollider(hitBox->GetID());
}

void Character::specialAttack(Direction direction)
{
  
}

void Character::block()
{
  
}

void Character::move(Direction direction)
{
  // Get the character's current velocity
  auto body = entity_->GetComponent<cmp::AdvancedBody>();
  vec2 newVelocity = body->GetVelocity();

  // Get the character's sprite
  std::shared_ptr<Sprite> sprite = entity_->GetComponent<Sprite>();

  // standard values to be used recurringly in the function
  float dSpeed = acceleration;
  float topSpeed = maxSpeed;

  if (direction != Down)
  {
    passThrough_ = false;
  }

  switch (direction)
  {
    // Move left
    case Left:
      // If the character is on the floor, move normally
      if (onFloor_)
      {
        newVelocity.x -= dSpeed;
      }

      // Cut movement speed in half if we are in the air
      else
      {
        newVelocity.x -= dSpeed / drag;
      }

      // Sprite is now facing left
      if (sprite)
        sprite->SetFlipped(true);

      //Clamp the velocity
      newVelocity.x = fmaxf(newVelocity.x, -topSpeed);

      break;

    // Move right
    case Right:
      // If the character is on the floor, move normally
      if (onFloor_)
      {
        newVelocity.x += dSpeed;
      }

      // Cut movement speed in half if we are in the air
      else
      {
        newVelocity.x += dSpeed / drag;
      }

      // Sprite is now facing right
      if (sprite)
        sprite->SetFlipped(false);

      //Clamp the velocity
      newVelocity.x = fminf(newVelocity.x, topSpeed);

      break;

    // Center, don't move
    default:
      if (onFloor_)
      {
        newVelocity.x = 0.0f;
      }
  }

  // Set the character's updated velocity
  body->SetVelocity(newVelocity);
}

void Character::attachEntity(std::shared_ptr<fb::Entity> entity)
{
  entity_ = entity;
}

std::shared_ptr<Entity> Character::getEntity()
{
  return entity_;
}

bool Character::canJump()
{
  return canJump_;
}

bool Character::isOnFloor()
{
  return onFloor_;
}

bool Character::isFirstJump()
{
  return firstJump;
}

bool Character::canMove()
{
  return !isHit_;
}

void Character::removeLimiter()
{
  terminalVelocity = false;
}

void Character::addLimiter()
{
  terminalVelocity = true;
}

int Character::GetID()
{
  return id;
}

void Character::setOnFloor(bool floor)
{
  onFloor_ = floor;
}

void Character::setJump(bool jump)
{
  canJump_ = jump;
}

void Character::setFirstJump(bool first)
{
  firstJump = first;
}

void Character::setHit(bool gotHit)
{
  isHit_ = gotHit;
}

float Character::GetAcceleration()
{
  return acceleration;
}

float Character::GetJumpSpeed()
{
  return jumpSpeed;
}

float Character::GetMaxSpeed()
{
  return maxSpeed;
}

float Character::GetGravity()
{
  return gravity;
}

int Character::GetDrag()
{
  return drag;
}

void Character::SetAcceleration(float speed)
{
  acceleration = speed;
}

void Character::SetJumpSpeed(float speed)
{
  jumpSpeed = speed;
}

void Character::SetMaxSpeed(float speed)
{
  maxSpeed = speed;
}

void Character::SetGravity(float grav)
{
  gravity = grav;
}

void Character::SetDrag(int resistance)
{
  drag = resistance;
}

float Character::GetPunchTimer()
{
  return punchTimer;
}

void Character::TickPunchTimer(float dt)
{
  punchTimer -= dt;
}

void Character::ResetPunchTimer()
{
  punchTimer = PUNCH_COOLDOWN;
}

int Character::getSlimeBagWeight()
{
  return currentSlimeScore;
}

float Character::getZoneTimer()
{
  return zoneTimer;
}

void Character::tickZoneTimer(float dt)
{
  zoneTimer -= dt;
}

void Character::setZoneTimer(float time)
{
  zoneTimer = time;
}

int Character::getSlimeBagCapacity()
{
  return slimeBagCapacity;
}

const std::vector<int>& Character::getSlimeBag()
{
  return slimeBag;
}

int Character::addSlime(int weight)
{
  if (slimeBagSize + 1 <= slimeBagCapacity)
  {
    slimeBagSize += 1;
    currentSlimeScore += weight;
    slimeBag.push_back(weight);

    if (weight > 1)
      std::sort(slimeBag.begin(), slimeBag.end(), std::greater_equal<int>());
    PlaySlimePickupSound();
    return 0;
  }
  else
  {
    if (weight == 5 && slimeBag.back() == 1)
    {
      slimeBag.pop_back();
      currentSlimeScore -= 1;

      slimeBag.push_back(weight);
      currentSlimeScore += 5;

      std::sort(slimeBag.begin(), slimeBag.end(), std::greater_equal<int>());
      return 1;
    }
    else
      PlaySlimeFullSound();
      return weight;
  }
}

void Character::clearSlimeBagWeight()
{
  currentSlimeScore = 0;
  slimeBagSize = 0;
  slimeBag.clear();
}

void Character::removeSlime(int weight)
{
  for (unsigned i = 0; i < slimeBag.size(); ++i)
  {
    if (slimeBag[i] == weight)
    {
      slimeBagSize -= 1;
      slimeBag[i] = 0;
      currentSlimeScore -= weight;
      return;
    }
  }
}

int Character::popSlime()
{
  int result = 0;
  if (currentSlimeScore)
    result = slimeBag.back();
  currentSlimeScore -= result;
  if (result)
  {
    slimeBag.pop_back();
    slimeBagSize -= 1;
  }

  return result;
}

void Character::PlayJumpSound()
{
  evt::CharacterEvent jumpEvent;
  jumpEvent.type = evt::jump;
  jumpEvent.characterEntity = entity_;
  evt::EventManager::GetCharacterEventSubject().Notify(jumpEvent);
}

void Character::PlayDoubleJumpSound()
{
  evt::CharacterEvent jumpEvent;
  jumpEvent.type = evt::doubleJump;
  jumpEvent.characterEntity = entity_;
  evt::EventManager::GetCharacterEventSubject().Notify(jumpEvent);
}

void Character::PlayWallJumpSound()
{
  evt::CharacterEvent jumpEvent;
  jumpEvent.type = evt::wallJump;
  jumpEvent.characterEntity = entity_;
  evt::EventManager::GetCharacterEventSubject().Notify(jumpEvent);
}

void Character::PlayPunchHitSound()
{
  evt::CharacterEvent punchEvent;
  punchEvent.type = evt::punchHit;
  punchEvent.characterEntity = entity_;
  evt::EventManager::GetCharacterEventSubject().Notify(punchEvent);
}

void Character::PlayPunchMissSound()
{
  evt::CharacterEvent punchEvent;
  punchEvent.type = evt::punchMiss;
  punchEvent.characterEntity = entity_;
  evt::EventManager::GetCharacterEventSubject().Notify(punchEvent);
}

void Character::PlaySlimePickupSound()
{
  evt::CharacterEvent punchEvent;
  punchEvent.type = evt::slimePickup;
  punchEvent.characterEntity = entity_;
  evt::EventManager::GetCharacterEventSubject().Notify(punchEvent);
}

void Character::PlaySlimeFullSound()
{
  evt::CharacterEvent punchEvent;
  punchEvent.type = evt::slimeFull;
  punchEvent.characterEntity = entity_;
  evt::EventManager::GetCharacterEventSubject().Notify(punchEvent);
}

void Character::PlayMoveSound()
{
  evt::CharacterEvent moveEvent;
  moveEvent.type = evt::jump;
  moveEvent.characterEntity = entity_;
  evt::EventManager::GetCharacterEventSubject().Notify(moveEvent);
}
