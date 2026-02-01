#ifndef PLAYER_H
#define PLAYER_H 

#include <GameObject/Creature.h>

class Player {
public:
	Creature creature;

	void frontPlayer(glm::vec3 front);
};

#endif // PLAYER
#pragma once
