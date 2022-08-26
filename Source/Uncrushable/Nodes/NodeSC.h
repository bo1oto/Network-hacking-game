
#pragma once

#include "CoreMinimal.h"
#include <map>
#include <list>
#include <tuple>
#include "NodeBase.h"
#include "NodeSC.generated.h"

UCLASS()
class UNCRUSHABLE_API ANodeSC : public ANodeBase
{
	GENERATED_BODY()
	
	bool have_recovery_system = false;
	FTimerHandle helpTimer;

	void AcceptPacket(APacket* packet) final;
	void GeneratePacket(int chance) final;

	void SaveThisWorld();
	struct ApocalypseRescueKit final
	{
		int list_size = 0;
		std::list<std::pair<FTimerHandle*, bool>> list_apocalypse_timers;
		std::map<short, int> map_id_vec;
	};
	ApocalypseRescueKit* sApocalypseRescueKit;
public:
	ANodeSC();
	void BeginPlay() final;
	void Tick(float DeltaTime) final;

	static int id_counter;//SC - 30-49
};
