#pragma once

#include "Modules/ModuleManager.h"

class FUDPLogOutputDevice;

class FUDPLoggerModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    FUDPLogOutputDevice* OutputDevice = nullptr;
};