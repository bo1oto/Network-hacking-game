
#pragma once

#include "CoreMinimal.h"
#include "NodeBase.h"
#include "NodeTI.generated.h"

UCLASS()
class UNCRUSHABLE_API ANodeTI : public ANodeBase
{
	GENERATED_BODY()

public:
	ANodeTI();
	void BeginPlay() final;

	bool CheckIDInTable(int _id);

	static int id_counter;//TI - 10-29
private:
	static int vlan_counter;

	void AcceptPacket(APacket* packet) final;

	struct Routing final
	{
		Routing(int vlan_num, std::vector<int> vec = {});
	public:
		int vlan;
		std::vector<int> id_numbers;
	};
	std::vector<Routing*> routingTable;
	
	UFUNCTION(BlueprintCallable)
	void CreateVLAN(ANodeTI* node);
	void FillVLAN(std::vector<int>* id_vec, int vlan_num);
};
