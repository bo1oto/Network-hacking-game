#include "Link.h"


ALink::ALink()
{
	PrimaryActorTick.bCanEverTick = false;
	particleSystem = CreateDefaultSubobject<UParticleSystemComponent>("PS_Link");
}

void ALink::InitializeLink(uint8 num)
{
	switch (num) {
	case 0: {
		eLinkType = ELinkType::TwistedPair;
		speed_coef = 0.75f;
		max_load = 100;
		interference_immunity = 30;
		MaxLength = 750;
		break;
	}
	case 1: {
		eLinkType = ELinkType::OpticalFiber;
		speed_coef = 1.25f;
		max_load = 80;
		interference_immunity = 10;
		MaxLength = 1500;
		break;
	}
	default: {
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Can't initialize link!");
		break;
	}
	}
}

void ALink::BeginPlay()
{
	Super::BeginPlay();
}

void ALink::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

TArray<FText> ALink::GetKeyParameters() const
{
	// link type, loading, interference_immunity, state
	TArray<FText> parameters {
		GetTypeInfo(),
		FText::FromString(TEXT("Loading: ") + FString::FromInt(current_load) + TEXT("/") + FString::FromInt(max_load)),
		FText::FromString(FString::FromInt(interference_immunity)),
		bIsAlive ? FText::FromString(TEXT("Alive")) : FText::FromString(TEXT("Dead"))
	};
	return parameters;
}

FText ALink::GetTypeInfo() const
{
	switch (eLinkType) {
	case ELinkType::TwistedPair:	return FText::FromString(TEXT("TwistedPair"));
	case ELinkType::OpticalFiber:	return FText::FromString(TEXT("OpticalFiber"));
	default:						return FText::FromString(TEXT("WUT!?"));
	}
}

void ALink::AddWorkload(int quantity, float last_packet_time)
{
	current_load += quantity;
	if (current_load > max_load) {
		bIsAlive = false;
		FTimerHandle unload_timer = FTimerHandle();
		GetWorldTimerManager().SetTimer(unload_timer, [this] () -> void
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Link unloaded");
			bIsAlive = true;
		}, 
		1.0f, false, last_packet_time);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Link overloaded");
	}
}

void ALink::AddTemporaryWorkload(int _add_work, float delay_time)
{
	AddWorkload(_add_work, delay_time);
	FTimerHandle unloading_timer = FTimerHandle();
	GetWorldTimerManager().SetTimer(unloading_timer, [_add_work, this] () -> void
	{
		AddWorkload(-_add_work);
	}, 
	1.0f, false, delay_time);
}
