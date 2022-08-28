
#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"

#include <vector>
#include "Uncrushable/Link.h"
#include "Uncrushable/Packet.h"

#include "NodeBase.generated.h"

enum class NodeType : uint8
{
	DataStorage = 0,
	Security,
	TechnicalInfrastructure,
	PersonalComputer,
	ExternalOutput
};

enum class EPolitic : uint8
{
	OnlyAllowed = 0,
	NotForbidden
	//Maybe more later
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

protected:
	virtual void BeginPlay() override;
public:
	void Tick(float DeltaTime) override;

private:
	int workload = 0;

	struct FSpyInfo final
	{
		FTimerHandle spyTimer;
		int stolen_key_info = 0;
		std::vector<short> stolen_roots;
		int spy_id = -2;
	};
	FSpyInfo* sSpyInfo;

protected:
	static bool IsAlarm;
	static EPolitic politic;
	static int sameSignChance;
	static int upSignChance;
	static int behaviorChance;
	static int healChance;
	static int killChance;

	void SendAlarmPacket();

	inline void AddWorkload(int quantity);

	void AddWorkloadWithDelay(short _add_work, float delay_time);

	virtual void GeneratePacket(int chance);

public:
	ANodeBase();

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<NodeState> nodeState;

	UPROPERTY(BlueprintReadOnly)
	int vlan = 0;

	UPROPERTY(BlueprintReadOnly)
	int id;

	NodeType nodeType;

	void ComputeNodePath(const ANodeBase* sender, int _id, std::vector<ANodeBase*>& path, int counter = 0);

	int FindRouter(int _vlan, int counter = 0) const;

	ANodeBase* CheckNeighbour(int node_id) const;
	ANodeBase* CheckNeighbour(NodeType _nodeType) const;

	void DeterminePath(int node_id, std::vector<ANodeBase*> path);

	struct FProtection final
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
		bool bSpamFilter = false, bIsOn = true, behaviorAnalizer = false;
		std::vector<Signature> threatSigns;
		bool SignatureCheck(const APacket& packet) const;
		int SourceTargetCheck(const APacket& packet) const;
	};

	struct FNodeLink final
	{
		ALink* link;
		ANodeBase* targetNode;
	};

	struct Information final
	{
		std::vector<ANodeBase*> vec_net_id;
		std::vector<short> vec_roots;
		int key_info_count = 0;
	};

	FProtection* sProtection;
	Information* sInformation;
	std::vector<FNodeLink*> nodeLinks = {};


	void SendPacket(APacket* packet, std::vector<AActor*>::iterator it);
	
	virtual void CheckPacket(APacket* packet, std::vector<AActor*>::iterator it);

	virtual void AcceptPacket(APacket* packet);

	UFUNCTION(BlueprintCallable, Category = "Link")
	static void AddLink(ALink* _link, ANodeBase* sourceNode, ANodeBase* targetNode);

	UFUNCTION(BlueprintCallable, Category = "Information")
	FString GetInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Information")
	FText GetTypeInfo() const;
	FString GetStateInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Information")
	TArray<FText> GetKeyParameters() const;

	UFUNCTION(BlueprintCallable, Category = "GameRules")
	void SetVLAN(int num);

	UFUNCTION(BlueprintCallable, Category = "GameRules")
	void AddInformation(ANodeBase* node_ptr);

	UFUNCTION(BlueprintCallable, Category = "GameRules")
	void AddKeyInfo();

	UFUNCTION(BlueprintCallable, Category = "GameRules")
	void AddRoots(int root_id);

	UFUNCTION(BlueprintCallable)
	bool ContainInfo();

	static TSubclassOf<APacket> packetTemp;

	UFUNCTION(BlueprintCallable, Category = "Graphics")
	static void FillPacketTemp(TSubclassOf<APacket> temp);

	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	UMaterialInterface* main_mat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Graphics")
	UStaticMeshComponent* Mesh;
};
