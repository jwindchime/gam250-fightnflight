// Author:   James Liao
// Copyright © 2017 DigiPen (USA) Corporation.
#include "CharacterHandler.h"
#include "Transform.h"
#include "BasicBody.h"
#include "Texture.h"
#include "Sprite.h"
#include "glm\vec2.hpp"
#include "EntityManager.h"
#include "PhysicsManager.h"
#include "DudeAI.h"
#include "ComponentFactory.h"
#include "LevelLoading.h"
#include <string>
#include <iostream>
#include <fstream>
#include <rapidjson/istreamwrapper.h>
#include "Score.h"
#include "ControllerHandler.h"
#include "SDL2\SDL.h"
#include "Time.h"
#include "MakeParticles.h"
#include "PopupText.h"
#include "WrapperObject.h"
#include "AdvancedBody.h"
#include "FistComponent.h"
#include "EventManager.h"

using namespace fb;
using namespace glm;

#define MAX_USERS 4
#define BOUNCE_MODIFIER 5

// Forward declarations
std::vector<Character *> CharacterManager::characters;
bool CharacterManager::isActive;
DocumentPtr CharacterManager::jsonGlobal;

// Each player needs their own instance of the Character class
void CharacterManager::Init()
{
  // Load each of the character JSON archetypes
  EntityManager::LoadArchetype("playerA.json");
  EntityManager::LoadArchetype("playerB.json");
  EntityManager::LoadArchetype("playerC.json");
  EntityManager::LoadArchetype("playerD.json");

  // Load the fist JSON archetypes (used when a fighter punches)
  EntityManager::LoadArchetype("fistA.json");
  EntityManager::LoadArchetype("fistB.json");

  // Create as many characters as controllers attached
  for (int i = 0; i < max(1, ControllerManager::GetNumPlayers()); i++)
  {
    if (characters.size() > 4)
      Logger::Msg("HF");

    characters.push_back(new Character(i));

    EntityPtr player;

    std::shared_ptr<Entity> fistEntity = std::make_shared<Entity>("fist");
    auto fistTrans = std::make_shared<cmp::Transform>();
    fistTrans->SetScale(1.1f, 1.1f);
    fistEntity->AttachComponent(fistTrans);
    

    switch (i)
    {
      case 0:
        player = EntityManager::CreateEntity("PlayerA");
        fistEntity->AttachComponent(std::make_shared<Sprite>("assets/img/RedFist.png", 1, RenderTexture::Layer::player_front_layer));
        break;
      case 1:
        player = EntityManager::CreateEntity("PlayerB");
        fistEntity->AttachComponent(std::make_shared<Sprite>("assets/img/BlueFist.png", 1, RenderTexture::Layer::player_front_layer));
        break;
      case 2:
        player = EntityManager::CreateEntity("PlayerC");
        fistEntity->AttachComponent(std::make_shared<Sprite>("assets/img/YellowFist.png", 1, RenderTexture::Layer::player_front_layer));
        break;
      case 3:
        player = EntityManager::CreateEntity("PlayerD");
        fistEntity->AttachComponent(std::make_shared<Sprite>("assets/img/GreyFist.png", 1, RenderTexture::Layer::player_front_layer));
        break;
    }

    // Set the entity name to match the player
    player->SetName("Player " + std::to_string(i));

    // Attach collider component
    CollisionLayer layer(user, world);
    std::shared_ptr<cmp::AdvancedBody> body(std::make_shared<cmp::AdvancedBody>(layer));
    player->AttachComponent(body);
    body->CreateDetectorSet(world);
    body->CreateDetectorSet(slime);
    body->CreateDetectorSet(king);
    body->CreateDetectorSet(ghost);
    body->CreateDetectorSet(goal);
    
    auto fistComp = std::make_shared<cmp::FistComponent>();
    fistEntity->AttachComponent(fistComp);
    fistEntity->SetParent(player);
    EntityManager::AddEntity(fistEntity);
    evt::EventManager::GetCharacterEventSubject().RegisterObserver(fistComp);
    //body->SetWorldFriction(0.2f);

    // Make side colliders for each character
    //CollisionLayer worldLayer(base, world);
    //CollisionLayer bottomLayer(base, world | slime | king | ghost);
    
    characters[i]->attachEntity(player);

    // Characters are not active until they are added to the entity manager
    isActive = false;

    /*
    for (unsigned i = 0; i < MAX_USERS; ++i)
      characters[i]->getEntity()->AttachComponent(std::make_shared<WrapperObject>(WrapperObject()));
    */
  }
}

void CharacterManager::Update()
{
  // Only update characters while not paused
  if (Time::GetTimescale() == 0) { return; }



  // Update each character
  for (int i = 0; i < characters.size(); i++)
  {
    auto trans = characters[i]->getEntity()->GetComponent<fb::cmp::Transform>();
    auto body = characters[i]->getEntity()->GetComponent<cmp::AdvancedBody>();

    if(characters[i]->getSlimeBag().size() == 5 && RNG::Integer(0, 10) == 10)
      MakeSquishParticle(7.0f, trans->GetPosition());
    
    //inform the cam manager that this is an important object
    CamManager::DynamicPing(trans->GetPosition() + 0.5f * body->GetVelocity(), 5 - GetPlayerCount());

    // Update the punch cooldown
    if (characters[i]->GetPunchTimer() > 0)
    {
      characters[i]->TickPunchTimer(Time::GetDT());
    }

    // Vibrate the controller if the character is stunned
    if (!characters[i]->canMove())
    {
      ControllerManager::GetController(i)->VibrateController(0.0f, 1.0f, 0.2f);
    }

    //if moving upwards, pass through platforms
    if (body->GetVelocity().y > 0)
    {
      characters[i]->passThrough_ = true;
    }

    // If passThrough_ is true, don't check for fall-through platforms
    if(characters[i]->passThrough_)
    {
      CollisionLayer layer(user, world);
      body->SetLayer(layer);
    }
    else
    {
      CollisionLayer layer(user, world);
      body->SetLayer(layer);
    }

    // Check zone collider
    //CollisionResult result = PhysicsManager::RunCollision(*characters[i]->getZoneCollider());
    CollisionResult result = body->RunDetection(Layer::goal, cmp::AdvancedDetectors::BODY);
    bool correct = false;

    if(result.collision && Time::GetScaledDT())
    {
      unsigned counter = 0;
      unsigned size = result.numCollisions;
      while(counter < size)
      {
        //Make sure you are colliding with the zone that is turned on
        EntityPtr zone = result.boxCollisions[counter]->GetParent().lock();

        std::pair<unsigned, unsigned> currentZone = DudeAI::getCurrentZone();

        if (currentZone.first == DudeAI::getZoneId(zone))
        {
          correct = true;
          break;
        }
        counter++;
      }

      if(correct)
      {
        //Check for any slimes to drop off
        if (characters[i]->getSlimeBagWeight())
        {
          // Bigger vibration the more slimes you have (continuous)
          ControllerManager::GetController(i)->VibrateController(0.2f * characters[i]->getSlimeBagWeight(), 0.0f, 0.1f);

          if (characters[i]->getZoneTimer() > 0)
          {
            characters[i]->tickZoneTimer(Time::GetDT());
          }

          if(characters[i]->getZoneTimer() <= 0)
          {
              characters[i]->setZoneTimer(1.0f);
              int score = characters[i]->popSlime();
              if(score == 5)
              {
                evt::EventManager::GetCharacterEventSubject().Notify(evt::CharacterEvent(nullptr, evt::slimeGold));
              }
              else
              {
                evt::EventManager::GetCharacterEventSubject().Notify(evt::CharacterEvent(nullptr, evt::slimeDeliver));
              }
              if (i == 0)
                Score::AddScore(score, Score::player1);
              else if(i == 1)
                Score::AddScore(score, Score::player2);
              else if (i == 2)
                Score::AddScore(score, Score::player3);
              else if (i == 3)
                Score::AddScore(score, Score::player4);

              if (int weight = characters[i]->getSlimeBagWeight())
              {
                PopupNumber::Make(weight, PopupText::TeamColors[i], characters[i]->getEntity()->GetComponent<cmp::Transform>()->GetPosition(), 3.0f, 1.0f);
                ScreenspacePopupText::Make(std::to_string(weight), PopupText::TeamColors[i], { -0.8 + i * 1.6 / 3, -0.5 }, 0, 1.0f, true, i);
                // Bigger vibration the more slimes you have (pulse)
                //ControllerManager::GetController(i)->VibrateController(0.2f * weight, 0.0f, 0.1f);
              }
              else
              {
                PopupText::Make("EMPTY!", PopupText::UI_ColorRed, characters[i]->getEntity()->GetComponent<cmp::Transform>()->GetPosition(), 3.0f, 1.0f, true);
                ScreenspacePopupText::Make("EMPTY!", PopupText::UI_ColorRed, { -0.8 + i * 1.6 / 3, -0.5 }, 0, 1.0f, true, i);
                //MakeSquishParticle(5, )

                // Stop vibrating the controller when there's nothing left to turn in
                ControllerManager::GetController(i)->StopVibration();
              }
          }
          else if (characters[i]->getZoneTimer() <= 0)
          {
            characters[i]->setZoneTimer(1.0f);
            PopupText::Make("EMPTY!", PopupText::UI_ColorRed, characters[i]->getEntity()->GetComponent<cmp::Transform>()->GetPosition(), 3.0f, 1.0f);
          }
        }
      }
    }

    //Check top collider
    result = body->RunDetection(world, cmp::TOP);
    if (result.collision)
    {
      characters[i]->addLimiter();
    }

    //Check left collider
    result = body->RunDetection(world, cmp::LEFT);
    if (result.collision)
    {
      characters[i]->setHit(false);
      auto body = characters[i]->getEntity()->GetComponent<cmp::AdvancedBody>();
      characters[i]->removeLimiter();
    }

    //Check right collider
    result = body->RunDetection(world, cmp::RIGHT);
    if (result.collision)
    {
      characters[i]->setHit(false);
      auto body = characters[i]->getEntity()->GetComponent<cmp::AdvancedBody>();
      characters[i]->removeLimiter();
    }

    //Check bottom collider
    result = body->RunDetection(slime, cmp::BOTTOM);
    if (result.collision)
    {
      bool hitAlien = false;

      // Run through the box collisions, see what gets hit
      for (int j = 0; j < 5; j++)
      {
        const BoxCollider* collider = result.boxCollisions[j];

        if (collider /* && body->GetVelocity().y <= 0 */)
        {
          std::shared_ptr<Entity> entity = collider->GetParent().lock();

          if (entity->GetName() == "aliendude" || entity->GetName() == "aliengolden")
          {
            hitAlien = true;
            DudeAI::DestroyDude(entity, characters[i], user);
          }
        }
      }

      // If we hit an alien, do a special hop
      if (hitAlien)
      {
        body->SetVelocity(vec2(body->GetVelocity().x, fmaxf(0.0f, body->GetVelocity().y) + Character::GetJumpSpeed() / BOUNCE_MODIFIER));

        // Show a slime effect
        MakeJumpParticle(2.0f / 5.0f, CharacterManager::GetCharacter(i)->getEntity()->GetComponent<cmp::Transform>()->GetPosition());

        // Bigger vibration the more slimes you have
        ControllerManager::GetController(i)->VibrateController(0.2f * characters[i]->getSlimeBagWeight(), 0.0f, 0.15f);

        // Hopping off of a slime will count as your first jump from the ground
        characters[i]->setFirstJump(true);
        characters[i]->setJump(true);
      }
    }
    
    /*
    result = body->RunDetection(king, cmp::BOTTOM);
    if (result.collision)
    {
      bool hitBigAlien = false;

      // Run through the box collisions, see what gets hit
      for (int j = 0; j < 5; j++)
      {
        const BoxCollider* collider = result.boxCollisions[j];

        if (collider) //&& body->GetVelocity().y <= 0 )
        {
          std::shared_ptr<Entity> entity = collider->GetParent().lock();

          if (entity->GetName() == "aliengiant")
          {
            hitBigAlien = true;
          }
        }
      }

      // If we hit a big alien, do a big hop
      if (hitBigAlien)
      {
        body->SetVelocity(vec2(body->GetVelocity().x, Character::GetJumpSpeed() / 2.0f));

        // slight vibration
        ControllerManager::GetController(i)->VibrateController(0.2f, 0.0f, 0.15f);

        // Hopping off of a slime will count as your first jump from the ground
        characters[i]->setFirstJump(true);
        characters[i]->setJump(true);
      }
    }
    */

    bool touch = false;

    // Check for floor collision
    result = body->RunDetection(world, cmp::BOTTOM);
    if (result.collision)
    {
      touch = true;
    }

    // If the character should not pass through platforms, check for platform collisions
    if (!characters[i]->passThrough_)
    {
      // Check for platform collision
      result = body->RunDetection(ghost, cmp::BOTTOM);
      if (result.collision)
      {
        touch = true;
      }

    }

    if (touch)
    {
      // Set values only when character starts touching the floor (not continuous)
      if (!characters[i]->isOnFloor()) // && !characters[i]->passThrough_)
      {
        auto trans = characters[i]->getEntity()->GetComponent<cmp::Transform>();

        vec2 pos = trans->GetPosition() - vec2(0, (trans->GetScale().y / 2));

        MakeLandingParticle(3.0f, pos, false);

        // Turn off gravity
        body->SetAcceleration(vec2(0.0f, 0.0f));
        body->SetVelocity(glm::vec2(body->GetVelocity().x, 0));
        

        // The character is now on the floor
        characters[i]->setOnFloor(true);

        // The character is set to double-jump again
        characters[i]->setFirstJump(false);

        // The character can move again if they were hit-stunned
        characters[i]->setHit(false);

        // Character can hop around while holding the jump button
        characters[i]->removeLimiter();
      }
    }
    else
    {
      // If we were touching the floor but aren't anymore, turn gravity back on
      if (characters[i]->isOnFloor())
      {
        // Turn on gravity
        body->SetAcceleration(vec2(0.0f, Character::GetGravity()));

        // The character is no longer on the floor
        characters[i]->setOnFloor(false);

        // Character can double-jump in the air
        characters[i]->setJump(true);

        // If the character walks off the platform, they can double-jump
        if (body->GetVelocity().y <= 0)
        {
          characters[i]->setFirstJump(true);
        }
      }
    }
  }
}

void CharacterManager::Shutdown()
{
  for(auto iter = characters.begin(); iter != characters.end(); ++iter)
  {
    delete *iter;
  }

  characters.clear();

  // Characters are no longer active, do not execute character-related actions
  isActive = false;
}

bool CharacterManager::LoadGlobals(std::string filename)
{
  // Load in the JSON file
  std::ifstream file("assets/json/" + filename);

  if (!file.is_open())
  {
    Logger::Msg("Failed to load character globals", Error);
    return false;
  }

  IStreamWrapper fileWrapper(file);
  jsonGlobal = std::make_shared<Document>();
  jsonGlobal->ParseStream(fileWrapper);

  if (jsonGlobal->HasParseError())
  {
    Logger::Msg("Failed to parse JSON: " + jsonGlobal->GetParseError(), Error);
    return false;
  }

  // If the file is missing any of the essential component members, return false
  if (!jsonGlobal->HasMember("acceleration") || !jsonGlobal->HasMember("jumpspeed") || !jsonGlobal->HasMember("maxspeed") || !jsonGlobal->HasMember("gravity") || !jsonGlobal->HasMember("drag"))
  {
    Logger::Msg("Failed to parse JSON: Missing one or more global values");
    return false;
  }

  // Load the global values
  Character::SetAcceleration((*jsonGlobal)["acceleration"].GetFloat());
  Character::SetJumpSpeed((*jsonGlobal)["jumpspeed"].GetFloat());
  Character::SetMaxSpeed((*jsonGlobal)["maxspeed"].GetFloat());
  Character::SetGravity((*jsonGlobal)["gravity"].GetFloat());
  Character::SetDrag((*jsonGlobal)["drag"].GetInt());

  return true;
}

void CharacterManager::AddToEntityManager()
{
  // Add each character to the entity manager
  for (int i = 0; i < characters.size(); i++)
  {
    if(i == 0 || i == 2)
      GetCharacter(i)->clearSlimeBagWeight();

    EntityManager::AddEntity(GetCharacter(i)->getEntity());
  }

  // Characters are now active
  isActive = true;
}

void CharacterManager::RemoveFromEntityManager()
{
  // Remove each character from the entity manager
  for (int i = 0; i < characters.size(); i++)
  {
    EntityManager::RemoveEntity(GetCharacter(i)->getEntity());
  }

  // Characters are now inactive
  isActive = false;

}

void CharacterManager::AttachEntity(int id, std::shared_ptr<fb::Entity> entity)
{
  characters[id]->attachEntity(entity);
}

Character* CharacterManager::GetCharacter(int id)
{
  return characters[id];
}

DocumentPtr CharacterManager::GetJsonGlobal()
{
  return jsonGlobal;
}

bool CharacterManager::Active()
{
  return isActive;
}

void CharacterManager::SetCharacterPosition(int id, vec2 position)
{
  characters[id]->getEntity()->GetComponent<cmp::Transform>()->SetPosition(position);
}

void CharacterManager::ResetSlimeBags()
{
  for (int i = 0; i < characters.size(); i++)
  {
    characters[i]->clearSlimeBagWeight();
  }
}

void CharacterManager::SetActive(bool set)
{
  isActive = set;
}

int CharacterManager::GetPlayerCount()
{
  return characters.size();
}
