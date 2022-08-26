
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

	short workload = 0;

	struct SpyInfo final
	{
		FTimerHandle spyTimer;
		int stolen_key_info = 0;
		std::vector<short> stolen_roots;
		int spy_id = -2;
	};
	SpyInfo* sSpyInfo;

protected:
	virtual void BeginPlay() override;

	static bool IsAlarm;
	static Politic politic;
	static int sameSignChance;
	static int upSignChance;
	static int behaviorChance;
	static int healChance;
	static int killChance;

	void SendAlarmPacket();

	inline void AddWorkload(short quantity);
	void AddWorkloadWithDelay(short _add_work, float delay_time);
	virtual void GeneratePacket(int chance);
public:	
	// Called every frame
	void Tick(float DeltaTime) override;
	ANodeBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Graphics")
	UStaticMeshComponent* Mesh;

	UFUNCTION(BlueprintCallable, Category = "Link")
	static void AddLink(ALink* _link, ANodeBase* sourceNode, ANodeBase* targetNode);

	UFUNCTION(BlueprintCallable, Category = "Information")
	FString GetInfo() const;
	UFUNCTION(BlueprintCallable, Category = "Information")
	FText GetTypeInfo() const;
	FString GetStateInfo() const;
	UFUNCTION(BlueprintCallable, Category = "Information")
	TArray<FText> GetKeyParameters() const;

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<NodeState> nodeState;
	UPROPERTY(BlueprintReadOnly)
	int vlan = 0;
	UPROPERTY(BlueprintReadOnly)
	int id;
	NodeType nodeType;

	std::vector<ANodeBase*> ComputeNodePath(const ANodeBase* sender, int _id, int counter = 0);
	int FindRouter(int _vlan, int counter = 0) const;
	ANodeBase* CheckNeighbour(int node_id) const;
	ANodeBase* CheckNeighbour(NodeType _nodeType) const;
	std::vector<ANodeBase*>* DeterminePath(int node_id);

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
		bool SignatureCheck(const APacket* packet) const;
		int SourceTargetCheck(const APacket* packet) const;
	};
	Protection* sProtection;
	struct NodeLink final
	{
		ALink* link;
		ANodeBase* targetNode;
	};
	std::vector<NodeLink*> nodeLinks = {};
	struct Information final
	{
		std::vector<ANodeBase*> vec_net_id;
		std::vector<short> vec_roots;
		short key_info_count = 0;
	};
	Information* sInformation;

	void SendPacket(APacket* packet, std::vector<ANodeBase*>* vec, std::vector<ANodeBase*>::iterator it);
	virtual void CheckPacket(APacket* packet, std::vector<ANodeBase*>* vec, std::vector<ANodeBase*>::iterator it);
	virtual void AcceptPacket(APacket* packet);


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
};
