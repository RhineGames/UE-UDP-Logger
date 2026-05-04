#include "UDPLoggerModule.h"
#include "UDPLogOutputDevice.h"

#include "Misc/OutputDevice.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogUDPLogger, Log, All);

void FUDPLoggerModule::StartupModule()
{
    UE_LOG(LogUDPLogger, Log, TEXT("UDPLogger: Startup"));

    OutputDevice = new FUDPLogOutputDevice();

    if (GLog)
    {
        GLog->AddOutputDevice(OutputDevice);
        UE_LOG(LogUDPLogger, Log, TEXT("UDPLogger: OutputDevice registered"));
    }
    else
    {
        UE_LOG(LogUDPLogger, Error, TEXT("UDPLogger: GLog is null"));
    }
}

void FUDPLoggerModule::ShutdownModule()
{
    UE_LOG(LogUDPLogger, Log, TEXT("UDPLogger: Shutdown"));

    if (GLog && OutputDevice)
    {
        GLog->RemoveOutputDevice(OutputDevice);
        UE_LOG(LogUDPLogger, Log, TEXT("UDPLogger: OutputDevice removed"));
    }

    delete OutputDevice;
    OutputDevice = nullptr;
}

IMPLEMENT_MODULE(FUDPLoggerModule, UDPLogger)