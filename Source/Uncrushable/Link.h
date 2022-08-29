
#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"
#include "Particles/ParticleSystemComponent.h"

#include "Link.generated.h"


enum class ELinkType : uint8
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
	//copy constructor
	//= operator
	//destructor

	virtual void Tick(float DeltaTime) final;


	UFUNCTION(BlueprintCallable)
	void SetLinkType(uint8 num);

	UFUNCTION(BlueprintCallable)
	inline TArray<FText> GetKeyParameters() const;
	inline FText GetTypeInfo() const;

	inline void AddWorkload(short quantity, float last_packet_time);
	void AddWorkloadWithDelay(short _add_work, float delay_time);

protected:
	virtual void BeginPlay() final;

public:	

	UPROPERTY(BlueprintReadWrite)
	bool bIsAlive = true;

	UPROPERTY(BlueprintReadWrite)
	int MaxLength;

	ELinkType eLinkType;

	float speed_coef;
	short current_load = 0;
	short max_load;
	short interference_immunity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UParticleSystemComponent* particleSystem;
};
