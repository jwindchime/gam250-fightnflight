// Copyright © 2017 DigiPen (USA) Corporation.
/*!
*******************************************************************************
\file    CharacterHandler.h
\author  James Liao
\par     email: james.liao\@digipen.edu
\par     Course: GAM200F17-A
\brief   This system manages the characters and event interpretation.
*******************************************************************************/

#pragma once

#include "Character.h"
#include "Entity.h"
#include "glm\vec2.hpp"
#include "EntityManager.h"
#include <vector>


namespace fb
{
  class CharacterManager
  {
    public:
      /*!
      *******************************************************************************
      \brief   Initialize the Character list
      \return  None (void).
      *******************************************************************************/
      static void Init();

      /*!
      *******************************************************************************
      \brief   Update the Character attributes
      \return  None (void).
      *******************************************************************************/
      static void Update();

      /*!
      *******************************************************************************
      \brief   Clear the Character list. DOES NOT delete attached entities.
      \return  None (void).
      *******************************************************************************/
      static void Shutdown();

      /*!
      *******************************************************************************
      \brief   Loads the global values used by all characters from a JSON file.
      \param  filename
        The name of the JSON file we want to load (std::string).
      \return  True if the load was successful, false otherwise (bppl).
      *******************************************************************************/
      static bool LoadGlobals(std::string filename);

      /*!
      *******************************************************************************
      \brief   Adds the Character entities to the entity manager
      \return  None (void).
      *******************************************************************************/
      static void AddToEntityManager();

      /*!
      *******************************************************************************
      \brief   Removes the Character entities from the entity manager
      \return  None (void).
      *******************************************************************************/
      static void RemoveFromEntityManager();

      /*!
      *******************************************************************************
      \brief   Attach an entity to the specified character
      \param   id
        The ID to check for (int).
      \param   entity
        The entity this character is responsible for (Entity *).
      \return  None (void).
      *******************************************************************************/
      static void AttachEntity(int id, std::shared_ptr<fb::Entity> entity);

      /*!
      *******************************************************************************
      \brief   Find a character based on ID
      \param   id
        The ID to check for (int).
      \return  Pointer to the character or NULL if none found (Character *).
      *******************************************************************************/
      static Character* GetCharacter(int id);


      /*!
      *******************************************************************************
      \brief   Get the JSON document for the global variables
      \return  Pointer to the JSON document holding the global variables (DocumentPtr).
      *******************************************************************************/
      static DocumentPtr GetJsonGlobal();

      /*!
      *******************************************************************************
      \brief   Returns whether or not the characters are currently active
      \return  True if characters are active, false otherwise. (bool).
      *******************************************************************************/
      static bool Active();

      /*!
      *******************************************************************************
      \brief   Sets the specified characters position
      \param   id
        ID for the player to set (int).
      \param   position
        Position at which to set the character (vec2).
      \return  None (void).
      *******************************************************************************/
      static void SetCharacterPosition(int id, glm::vec2 position);

      static void ResetSlimeBags();

      static void SetActive(bool);
      static int GetPlayerCount();

  private:
      static std::vector<Character*> characters;  //!< Holds the characters currently being played
      static bool isActive; //!< Whether or not the characters are currently active in the gamestate
      static DocumentPtr jsonGlobal; //!< Holds the JSON for the global variables
  };
}