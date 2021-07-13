// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include <vector>
#include "Uncrushable/Link.h"
#include "Uncrushable/Packet.h"
#include "NodeBase.generated.h"

enum NodeType
{
	DataStorage = 0,
	Security,
	TechnicalInfrastructure,
	PersonalComputer,
	ExternalOutput
};
enum Politic
{
	OnlyAllowed = 0,
	NotForbidden
	//Может что то про тревогу, но это потом
};
UENUM(BlueprintType)
enum NodeState
{
	Working = 0,
	Offline,
	Captured,
	Overloaded
};

UCLASS()
class UNCRUSHABLE_API ANodeBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ANodeBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Graphics")
	UStaticMeshComponent* Mesh;

	UFUNCTION(BlueprintCallable, Category = "Link")
	static void AddLink(ALink* _link, ANodeBase* sourceNode, ANodeBase* targetNode);
	UFUNCTION(BlueprintCallable, Category = "Nodes")
	FString GetInfo();
	UFUNCTION(BlueprintCallable, Category = "Nodes")
	FText GetTypeInfo();

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<NodeState> nodeState;
private:
	short workload = 0;

protected:
	void BeginPlay() override;

	static bool IsAlarm;
	static Politic politic;
	static int sameSignChance;
	static int upSignChance;
	static int behaviorChance;
	static int healChance;
	static int killChance;

	void SendAlarmPacket();
	inline void AddWorkload(short quantity);

	virtual void GeneratePacket(int chance);

	void AddWorkloadWithDelay(short _add_work, float delay_time);
public:	
	// Called every frame
	void Tick(float DeltaTime) override;

	struct Protection final
	{
		/* Тогда обычные узлы имеют:
		*	1. Сигнатурную проверку (туда же входит эвристика) +
		*	2. Обнаружение на основе поведения (себя ?и соседей?)
		*		Если на узле присутствует шпионское ПО, то при каждом отправлении пакета во внешние выходы, есть шанс обнаружения такого ПО
		*	3. Анти-спам фильтр (он идёт отдельно) +
		*	4. Обработка пакетов из неизвестных источников +-
		*	5. Вроде нормально, но вроде чего-то не хватает
		*/
		int size;
		bool spamFilter = false, isOn = true, behaviorAnalizer = false;
		std::vector<Signature> threatSigns;
		bool SignatureCheck(APacket* packet);
		int SourceTargetCheck(APacket* packet);
	};
	Protection* sProtection;

	int vlan = 0;
	int id;
	NodeType nodeType;

	struct NodeLink final
	{
		ALink* link;
		ANodeBase* targetNode;
	};
	std::vector<NodeLink*> nodeLinks = {};

	std::vector<ANodeBase*> ComputeNodePath(ANodeBase* sender, int _id, int counter = 0);
	int FindRouter(int _vlan, int counter = 0);
	ANodeBase* CheckNeighbour(int node_id);
	ANodeBase* CheckNeighbour(NodeType _nodeType);
	std::vector<ANodeBase*>* DeterminePath(int node_id);

	
	void SendPacket(APacket* packet, std::vector<ANodeBase*>* vec, std::vector<ANodeBase*>::iterator it);
	virtual void CheckPacket(APacket* packet, std::vector<ANodeBase*>* vec, std::vector<ANodeBase*>::iterator it);
	virtual void AcceptPacket(APacket* packet);


	UFUNCTION(BlueprintCallable, Category = "Graphics")
	static void FillPacketTemp(TSubclassOf<APacket> temp);
	static TSubclassOf<APacket> packetTemp;
	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	UMaterialInterface* main_mat;

	UFUNCTION(BlueprintCallable, Category = "GameRules")
	void SetVLAN(int num);


private:

	struct SpyInfo
	{
		FTimerHandle spyTimer;
		int stolen_key_info = 0;
		int spy_id = -2;
	};
	SpyInfo* sSpyInfo;

public:
	struct Information
	{
		std::vector<ANodeBase*> vec_net_id;
		short key_info_count = 0;
	};
	Information* sInformation;

	UFUNCTION(BlueprintCallable, Category = "GameRules")
	void AddInformation(ANodeBase* node_ptr);
	UFUNCTION(BlueprintCallable, Category = "GameRules")
	void AddKeyInfo();
	UFUNCTION(BlueprintCallable)
	bool ContainInfo();
};
