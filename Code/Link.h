
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Particles/ParticleSystemComponent.h"
#include "Link.generated.h"

enum LinkType
{
	TwistedPair,
	OpticalFiber
};

UCLASS()
class UNCRUSHABLE_API ALink : public AActor
{
	GENERATED_BODY()
	
public:	
	ALink();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	bool isAlive;
	LinkType linkType;

	UFUNCTION(BlueprintCallable)
	void SetLinkType(int num);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UParticleSystemComponent* particleSystem;
};
