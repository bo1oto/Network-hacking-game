// Fill out your copyright notice in the Description page of Project Settings.

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
	// Sets default values for this actor's properties
	ALink();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool isAlive;
	LinkType linkType;

	UFUNCTION(BlueprintCallable)
	void SetLinkType(int num);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UParticleSystemComponent* particleSystem;
};
