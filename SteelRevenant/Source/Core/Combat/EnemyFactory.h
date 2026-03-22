#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "../../Action/EnemyFactory.h"

namespace Core::Combat
{
    // Forwarding alias to unify include paths between core and action layers.
    using EnemyFactory = Action::EnemyFactory;
}


