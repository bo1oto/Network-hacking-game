
#pragma once

#include "CoreMinimal.h"

#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"

#include <vector>
#include <stack>

#include "Uncrushable/Link.h"
#include "Uncrushable/Packet.h"

#include "NodeBase.generated.h"


enum class ENodeType : uint8
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
enum class ENodeState : uint8
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
		std::vector<ESignature> threatSigns;
		bool SignatureCheck(const APacket& packet) const;
		int SourceTargetCheck(const APacket& packet) const;
	};

	struct FNodeLink final
	{
		ALink* link;
		ANodeBase* targetNode;
	};

	struct FInformation final
	{
		std::vector<ANodeBase*> known_ids;
		std::vector<int> node_roots;
		int key_info_count = 0;
	};

private:
	struct FSpyInfo final
	{
		FTimerHandle spyTimer;
		int stolen_key_info = 0;
		std::vector<short> stolen_roots;
		int spy_id = -2;
	};

public:
	ANodeBase();

	virtual void Tick(float DeltaTime) override;


	int FindRouter(int _vlan, int counter = 0) const;

	APacket* CreatePacket(int target_id, EPacketType packet_type);

	ANodeBase* CheckNeighbour(int NodeId) const;
	ANodeBase* CheckNeighbour(ENodeType _nodeType) const;
	void DeterminePath(int NodeId, std::stack<AActor*>& path_out);
	void ComputeNodePath(const ANodeBase* sender, int _id, std::stack<AActor*>& path_out, int counter = 0);

	void SendPacket(APacket* packet);

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Graphics")
	static void FillPacketTemp(TSubclassOf<APacket> temp);

	UFUNCTION(BlueprintCallable, Category = "Link")
	static void AddLink(ALink* _link, ANodeBase* sourceNode, ANodeBase* targetNode);


	void SendAlarmPacket();

	inline void AddWorkload(int quantity);
	void AddWorkloadWithDelay(short _add_work, float delay_time);

	virtual void CheckPacket(APacket* packet);

	virtual void AcceptPacket(APacket* packet);

	virtual void GeneratePacket(int chance);


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


public:
	static TSubclassOf<APacket> packetTemp;


	UPROPERTY(BlueprintReadOnly)
	ENodeState eNodeState;

	UPROPERTY(BlueprintReadOnly)
	int vlan = 0;

	UPROPERTY(BlueprintReadOnly)
	int id;

	ENodeType eNodeType;

	FProtection* sProtection;
	FInformation* sInformation;
	std::vector<FNodeLink*> nodeLinks = {};


	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	UMaterialInterface* main_mat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Graphics")
	UStaticMeshComponent* Mesh;


protected:
	static bool bIsAlarm;
	static EPolitic ePolitic;
	static int sameSignChance;
	static int upSignChance;
	static int behaviorChance;
	static int healChance;
	static int killChance;

private:
	int workload = 0;

	FSpyInfo* sSpyInfo;

};
