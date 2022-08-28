
#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"

#include <vector>
#include "Link.h"

#include "Packet.generated.h"


enum class Signature : uint8
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
	virtual void Tick(float DeltaTime) final;
	APacket();
protected:
	virtual void BeginPlay() final;


public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* Mesh;

	void InitPacket(EPacketType _packetType, short _sourceId, short _targetId, std::vector<AActor*> _path);
	
	int size;
	short source_id = -2, target_id = -2;
	EPacketType packetType;
	std::vector<AActor*> path;

	struct Helper final
	{
		enum class EHelpState : uint8
		{
			Killer = 0,
			Healer,
			SuccessReport,
			DefeatReport,
			Prevention
		};
		EHelpState helpState;
		std::vector<Signature> sign;
		bool isAlarm = false;
	};
	struct Information final
	{
		Information() = delete;
		Information(bool _isDSRequest, uint8 _key_info_count, std::vector<uint8> _roots_for_id, AActor* _for_spy_ref);
		bool isDSRequest = false;
		uint8 key_info_count = 0;
		std::vector<uint8> roots_for_id;
		AActor* for_spy_ref = nullptr;
	};
	struct Threat final
	{
		Signature sign = Signature::NotSign;
		bool have_root = false;
		int spy_id = -2;
	};

	Helper* sHelper;
	Information* sInformation;
	Threat* sThreat;

	UFUNCTION(BlueprintCallable)
	static void FillMaterials(UMaterialInterface* simple, UMaterialInterface* spam, UMaterialInterface* attack, UMaterialInterface* info, UMaterialInterface* help);
	static UMaterialInterface* simpleMat;
	static UMaterialInterface* spamMat;
	static UMaterialInterface* attackMat;
	static UMaterialInterface* infoMat;
	static UMaterialInterface* helpMat;

	struct FPacketMove final
	{
		float ComputeNodePath(const AActor& source, const AActor& target, const ALink* _link);
		bool iden = false;
		std::vector<FVector> path;
		std::vector<FVector>::iterator it_path;
		const ALink* link = nullptr;
	};
	FPacketMove* sPacketMove;

};
