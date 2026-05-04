#pragma once

#include "CoreMinimal.h"
#include "Misc/OutputDevice.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"

#include "Sockets.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Common/UdpSocketBuilder.h"

class FUDPLogOutputDevice : public FOutputDevice, public FRunnable
{
public:
	FUDPLogOutputDevice();
	virtual ~FUDPLogOutputDevice();

	// FOutputDevice
	virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category) override;

	// FRunnable
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;

private:
	void InitSocket();
	void DestroySocket();
	void Send(const FString& Msg);

	bool BuildBroadcastAddress(TSharedPtr<FInternetAddr>& OutAddr);

private:
	FSocket* Socket = nullptr;
	TSharedPtr<FInternetAddr> Addr;

	FRunnableThread* Thread = nullptr;
	FThreadSafeBool bRunning = false;
	FThreadSafeBool bSocketReady = false;
};