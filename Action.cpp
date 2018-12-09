// Author:   James Liao
// Copyright © 2017 DigiPen (USA) Corporation.
#include "Action.h"

// Jump action class
Jump::Jump(Direction direction)
{
  // Jump requires a direction to check if character wants to jump down through a platform
  direction_ = direction;
}

void Jump::execute(Character* character)
{
  character->jump(direction_);
}


// Basic attack action class
BasicAttack::BasicAttack(Direction direction)
{
  direction_ = direction;
}

void BasicAttack::execute(Character* character)
{
  character->basicAttack(direction_);
}


// Special attack action class
SpecialAttack::SpecialAttack(Direction direction)
{
  direction_ = direction;
}

void SpecialAttack::execute(Character* character)
{
  character->specialAttack(direction_);
}


// Block action class
void Block::execute(Character* character)
{
  character->block();
}


// Move action class
Move::Move(Direction direction)
{
  direction_ = direction;
}

void Move::execute(Character* character)
{
  character->move(direction_);
}
