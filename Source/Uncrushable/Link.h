
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
	virtual void BeginPlay() final;

public:	
	virtual void Tick(float DeltaTime) final;

	UFUNCTION(BlueprintCallable)
	TArray<FText> GetKeyParameters() const;
	FText GetTypeInfo() const;
	UPROPERTY(BlueprintReadWrite)
	bool isAlive = true;
	UPROPERTY(BlueprintReadWrite)
	int max_length;
	LinkType linkType;

	float speed_coef;
	short current_load = 0;
	short max_load;
	short interference_immunity;

	inline void AddWorkload(short quantity, float last_packet_time);
	void AddWorkloadWithDelay(short _add_work, float delay_time);

	UFUNCTION(BlueprintCallable)
	void SetLinkType(int num);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UParticleSystemComponent* particleSystem;
};
