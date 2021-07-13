
#include "Link.h"

ALink::ALink()
{
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

