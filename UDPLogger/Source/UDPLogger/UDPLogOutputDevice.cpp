#include "UDPLogOutputDevice.h"
#include "Logging/LogMacros.h"
#include "HAL/PlatformProcess.h"

DEFINE_LOG_CATEGORY_STATIC(LogUDPLoggerDevice, Log, All);

FUDPLogOutputDevice::FUDPLogOutputDevice()
{
	bRunning = true;
	Thread = FRunnableThread::Create(this, TEXT("UDPLoggerThread"));
}

FUDPLogOutputDevice::~FUDPLogOutputDevice()
{
	bRunning = false;

	if (Thread)
	{
		Thread->WaitForCompletion();
		delete Thread;
		Thread = nullptr;
	}

	DestroySocket();
}

bool FUDPLogOutputDevice::Init()
{
	return true;
}

uint32 FUDPLogOutputDevice::Run()
{
	while (bRunning)
	{
		if (!bSocketReady)
		{
			InitSocket();

			if (bSocketReady)
			{
				UE_LOG(LogUDPLoggerDevice, Log, TEXT("UDPLogger: Socket initialized"));
			}
			else
			{
				UE_LOG(LogUDPLoggerDevice, Warning, TEXT("UDPLogger: Socket init failed, retrying..."));
				FPlatformProcess::Sleep(2.0f);
				continue;
			}
		}

		FPlatformProcess::Sleep(1.0f);
	}

	return 0;
}

void FUDPLogOutputDevice::Stop()
{
	bRunning = false;
}

void FUDPLogOutputDevice::InitSocket()
{
	DestroySocket();

	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SocketSubsystem)
	{
		return;
	}

	Socket = FUdpSocketBuilder(TEXT("UDPLoggerSocket"))
		.AsReusable()
		.WithBroadcast();

	if (!Socket)
	{
		return;
	}

	if (!BuildBroadcastAddress(Addr))
	{
		UE_LOG(LogUDPLoggerDevice, Warning, TEXT("UDPLogger: Failed to build broadcast address"));
		DestroySocket();
		return;
	}

	bSocketReady = true;
}

bool FUDPLogOutputDevice::BuildBroadcastAddress(TSharedPtr<FInternetAddr>& OutAddr)
{
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SocketSubsystem)
	{
		return false;
	}

	bool bCanBind = false;

	TSharedRef<FInternetAddr> LocalAddr =
		SocketSubsystem->GetLocalHostAddr(*GLog, bCanBind);

	uint32 IP = 0;
	LocalAddr->GetIp(IP);

	// Fallback wenn Unreal Mist liefert
	if (IP == 0)
	{
		UE_LOG(LogUDPLoggerDevice, Warning, TEXT("UDPLogger: Invalid local IP, fallback to global broadcast"));

		OutAddr = SocketSubsystem->CreateInternetAddr();

		bool bValid = false;
		OutAddr->SetIp(TEXT("255.255.255.255"), bValid);
		OutAddr->SetPort(7777);

		return bValid;
	}

	// Standard /24 subnet
	uint32 SubnetMask = 0xFFFFFF00;

	uint32 Broadcast = (IP & SubnetMask) | (~SubnetMask);

	OutAddr = SocketSubsystem->CreateInternetAddr();
	OutAddr->SetIp(Broadcast);
	OutAddr->SetPort(7777);

	UE_LOG(LogUDPLoggerDevice, Log, TEXT("UDPLogger: Using broadcast %u.%u.%u.%u"),
		(Broadcast >> 24) & 0xFF,
		(Broadcast >> 16) & 0xFF,
		(Broadcast >> 8) & 0xFF,
		Broadcast & 0xFF
	);

	return true;
}

void FUDPLogOutputDevice::DestroySocket()
{
	bSocketReady = false;

	if (Socket)
	{
		Socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
		Socket = nullptr;
	}
}

void FUDPLogOutputDevice::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category)
{
	if (!bSocketReady)
	{
		return;
	}

	FString Timestamp = FDateTime::Now().ToString(TEXT("%H:%M:%S"));

	FString Line = FString::Printf(
		TEXT("[%s][%s] %s"),
		*Timestamp,
		*Category.ToString(),
		V
	);

	Send(Line);
}

void FUDPLogOutputDevice::Send(const FString& Msg)
{
	if (!Socket || !Addr.IsValid())
	{
		return;
	}

	FTCHARToUTF8 Convert(*Msg);

	int32 BytesSent = 0;

	Socket->SendTo(
		(uint8*)Convert.Get(),
		Convert.Length(),
		BytesSent,
		*Addr
	);
}