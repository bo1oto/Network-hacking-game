
#pragma once

#include "CoreMinimal.h"
#include "NodeBase.h"
#include "NodeTI.generated.h"

UCLASS()
class UNCRUSHABLE_API ANodeTI : public ANodeBase
{
	GENERATED_BODY()

	static int vlan_counter;

	struct Routing final
	{
		Routing(int vlan_num);
		int vlan;
		std::vector<int> id_numbers;
	};
	std::vector<Routing*> routingTable;
	
	UFUNCTION(BlueprintCallable)
	void CreateVLAN(ANodeTI* node);
	void FillVLAN(std::vector<int>& id_vec, int vlan_num);
	
	void AcceptPacket(APacket* packet) final;
public:
	ANodeTI();
	void BeginPlay() final;

	bool CheckIDInTable(int _id) const;

	static int id_counter;//TI - 10-29
};
