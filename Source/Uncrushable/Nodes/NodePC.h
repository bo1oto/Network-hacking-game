
#pragma once

#include "CoreMinimal.h"
#include "NodeBase.h"
#include "NodePC.generated.h"

UCLASS()
class UNCRUSHABLE_API ANodePC : public ANodeBase
{
	GENERATED_BODY()
	
	void AcceptPacket(APacket* packet) final;
public:
	ANodePC();
	void BeginPlay() final;
	UPROPERTY(BlueprintReadWrite, Category = "GameRules")
	bool have_phys_connection = false;

	static int id_counter;//PC - 70-89

	void GeneratePacket(int chance) final;
};
