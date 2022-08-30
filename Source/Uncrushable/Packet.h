
#pragma once

#include "CoreMinimal.h"

#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"

#include <array>
#include <vector>
#include <stack>

#include "Link.h"

#include "Packet.generated.h"


enum class ESignature : uint8
{
	Spy_1 = 0,
	Spy_2,
	RootKit_1,
	RootKit_2,
	Crash_1,
	Crash_2,
	NotSign
};

enum class EPacketType : uint8
{
	AttackSpam = 0,
	AttackCapture,
	AttackCrash,
	AttackSpy,
	Simple,
	Informative,
	Helpful
};

UCLASS()
class UNCRUSHABLE_API APacket : public AActor
{
	GENERATED_BODY()

public:
	struct FHelper final
	{
		enum class EHelpState : uint8
		{
			Killer = 0,
			Healer,
			SuccessReport,
			DefeatReport,
			Prevention
		};
		EHelpState eHelpState;
		std::vector<ESignature> sign;
		bool isAlarm = false;
	};
	struct FInformation final
	{
		FInformation() = delete;
		FInformation(bool _isDSRequest, uint8 _key_info_count, std::vector<uint8> _roots_for_id, AActor* _for_spy_ref);
		bool isDSRequest = false;
		uint8 key_info_count = 0;
		std::vector<uint8> roots_for_id;
		AActor* for_spy_ref = nullptr;
	};
	struct FThreat final
	{
		ESignature sign = ESignature::NotSign;
		bool have_root = false;
		int spy_id = -2;
	};
	struct FPacketMove final
	{
		float ComputeNodePath( AActor* const & source,  AActor* const& target, const ALink* _link);
		bool bIsMoving = false;
		std::vector<FVector> vector_path;
		std::vector<FVector>::iterator it_path;
		const ALink* link = nullptr;
	};

public:
	APacket();
	//copy constructor
	//= operator
	//destructor
	void InitPacket(EPacketType _packetType, short _sourceId, short _targetId, std::stack<AActor*> _path);

protected:
	virtual void BeginPlay() final;

public:
	virtual void Tick(float DeltaTime) final;

	UFUNCTION(BlueprintCallable)
	static void FillMaterials(TArray<UMaterialInterface*>& _materials);

public:
	static TArray<UMaterialInterface*> materials;

	int size;
	short source_id = -2, target_id = -2;
	EPacketType packetType;
	std::stack<AActor*> path;

	FHelper* sHelper;
	FInformation* sInformation;
	FThreat* sThreat;
	FPacketMove* sPacketMove;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* Mesh;

};
