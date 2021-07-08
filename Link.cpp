// Fill out your copyright notice in the Description page of Project Settings.


#include "Link.h"

// Sets default values
ALink::ALink()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	particleSystem = CreateDefaultSubobject<UParticleSystemComponent>("PS_Link");

}
void ALink::BeginPlay()
{
	Super::BeginPlay();
	isAlive = true;
	
}
void ALink::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALink::SetLinkType(int num)
{
	switch (num)
	{
	case 0: linkType = LinkType::TwistedPair; break;
	case 1: linkType = LinkType::OpticalFiber; break;
	}
}

