
#pragma once

#include "CoreMinimal.h"

#include "NodeBase.h"

#include "NodeDS.generated.h"


UCLASS()
class UNCRUSHABLE_API ANodeDS : public ANodeBase
{
	GENERATED_BODY()

public:
	ANodeDS();

	void BeginPlay() final;

	virtual void AcceptPacket(APacket* packet) override final;

public:
	static int id_counter;//DS - 50-69
};
