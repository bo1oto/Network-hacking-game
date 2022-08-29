
#pragma once

#include "CoreMinimal.h"

#include "NodeBase.h"

#include "NodePC.generated.h"


UCLASS()
class UNCRUSHABLE_API ANodePC : public ANodeBase
{
	GENERATED_BODY()
	
public:
	ANodePC();
	void BeginPlay() final;

	virtual void GeneratePacket(int chance) override final;

private:
	virtual void AcceptPacket(APacket* packet) override final;

public:
	static int id_counter;//PC - 70-89

	UPROPERTY(BlueprintReadWrite, Category = "GameRules")
	bool bHasPhysicalConnection = false;
};
