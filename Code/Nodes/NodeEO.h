
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

	static int id_counter;//EO - 0-5
private:
	void AcceptPacket(APacket* packet) final;
	
};
