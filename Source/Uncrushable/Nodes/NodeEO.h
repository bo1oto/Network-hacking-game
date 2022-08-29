
#pragma once

#include "CoreMinimal.h"

#include "NodeBase.h"

#include "NodeEO.generated.h"


UCLASS()
class UNCRUSHABLE_API ANodeEO : public ANodeBase
{
	GENERATED_BODY()

public:
	ANodeEO();

	void BeginPlay() final;

private:
	virtual void AcceptPacket(APacket* packet) override final;

public:
	static int id_counter;//EO - 0-5

};
