
#pragma once

#include "CoreMinimal.h"

#include "NodeBase.h"

#include "NodeTI.generated.h"


UCLASS()
class UNCRUSHABLE_API ANodeTI : public ANodeBase
{
	GENERATED_BODY()


	struct Routing final
	{
		Routing(int vlan_num);
		int VLAN;
		std::vector<int> id_numbers;
	};
	
public:
	ANodeTI();

	void BeginPlay() final;

	bool CheckIDInTable(int _id) const;

private:
	virtual void AcceptPacket(APacket* packet) override final;

	UFUNCTION(BlueprintCallable)
	void CreateVLAN(ANodeTI* node);
	void FillVLAN(std::vector<int>& id_vec, int vlan_num);


public:
	static int id_counter;//TI - 10-29

private:
	static int vlan_counter;

	std::vector<Routing*> routingTable;

};
