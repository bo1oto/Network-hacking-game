
#pragma once

#include "Link.h"


ALink::ALink()
{
	PrimaryActorTick.bCanEverTick = false;
	particleSystem = CreateDefaultSubobject<UParticleSystemComponent>("PS_Link");
}

void ALink::SetLinkType(uint8 num)
{
	switch (num)
	{
	case 0:
		eLinkType = ELinkType::TwistedPair;
		speed_coef = 0.75f;
		max_load = 100;
		interference_immunity = 30;
		MaxLength = 750;
		break;
	case 1:
		eLinkType = ELinkType::OpticalFiber;
		speed_coef = 1.25f;
		max_load = 80;
		interference_immunity = 10;
		MaxLength = 1500;
		break;
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
	TArray<FText> arr;
	arr.Add(GetTypeInfo());
	arr.Add(FText::FromString(TEXT("Loading: ") + FString::FromInt(current_load) + TEXT("/") + FString::FromInt(max_load)));
	arr.Add(FText::FromString(FString::FromInt(interference_immunity)));
	if (bIsAlive)
		arr.Add(FText::FromString(TEXT("Alive")));
	else 
		arr.Add(FText::FromString(TEXT("Dead")));
	return arr;
}

FText ALink::GetTypeInfo() const
{
	switch (eLinkType)
	{
	case ELinkType::TwistedPair: return FText::FromString(TEXT("TwistedPair")); break;
	case ELinkType::OpticalFiber: return FText::FromString(TEXT("OpticalFiber")); break;
	default: return FText::FromString(TEXT("WUT!?"));
	}
}

inline void ALink::AddWorkload(short quantity, float last_packet_time = 0.0f)
{
	current_load += quantity;
	if (current_load > max_load)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Link overloaded");
		bIsAlive = false;
		FTimerHandle unload_timer = FTimerHandle();
		GetWorldTimerManager().SetTimer(unload_timer, [this]
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Link unloaded");
			bIsAlive = true;
		}, 1.0f, false, last_packet_time);
	}
}

void ALink::AddWorkloadWithDelay(short _add_work = 0, float delay_time = 0.0f)
{
	AddWorkload(_add_work, delay_time);
	FTimerHandle unloading_timer = FTimerHandle();
	GetWorldTimerManager().SetTimer(unloading_timer, [_add_work, this]
	{
		AddWorkload(-_add_work);
	}, 1.0f, false, delay_time);
}
