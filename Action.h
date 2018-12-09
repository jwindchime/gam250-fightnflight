// Copyright © 2017 DigiPen (USA) Corporation.
/*!
*******************************************************************************
\file    Action.h
\author  James Liao
\par     email: james.liao\@digipen.edu
\par     Course: GAM200F17-A
\brief   Implementation for Command Pattern actions for controller inputs.
*******************************************************************************/

#include "Character.h"

#pragma once

enum Direction;

//! Abstract Action class
class Action
{
  public:
    virtual ~Action() {} //!< Virtual destructor
    virtual void execute(Character* character) = 0; //!< Virtual function for linking action to character
    void setDirection(Direction direction) { direction_ = direction; }; //!< Stores the direction to be used by certan actions
    Direction getDirection() { return direction_; } //!< Returns the direction stored in this action
  protected:
    Direction direction_; //!< Direction that will be used by certain actions
};

//! Action class for handling jumping
class Jump : public Action
{
  public:
    Jump(Direction direction);

    void execute(Character* character);
};

//! Action class for handling basic attacks
class BasicAttack : public Action
{
  public:
    BasicAttack(Direction direction);

    void execute(Character* character);
};

//! Action class for handling special attacks
class SpecialAttack : public Action
{
  public:
    SpecialAttack(Direction direction);

    void execute(Character* character);
};

//! Action class for handling blocking
class Block : public Action
{
  public:
    void execute(Character* character);

    void setDirection(Direction direction) {}
};

//! Action class for handling movement
class Move : public Action
{
  public:
    Move(Direction direction);

    void execute(Character* character);
};