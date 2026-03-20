#pragma once
#include "../../StepTimer.h"

class IScene
{
public:
    virtual ~IScene() = default;
    virtual void Initialize()                       = 0;
    virtual void Update(const DX::StepTimer& timer) = 0;
    virtual void Render()                           = 0;
    virtual void Finalize()                         = 0;
};
