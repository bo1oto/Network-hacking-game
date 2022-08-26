
#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"

#include <vector>
#include "Link.h"

#include "Packet.generated.h"


enum Signature
{
	Spy_1 = 0,
	Spy_2,
	RootKit_1,
	RootKit_2,
	Crash_1,
	Crash_2,
	NotSign
};
enum PacketType
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

	void InitPacket(PacketType _packetType, short _sourceId, short _targetId);
	
	int size;
	short source_id = -2, target_id = -2;
	PacketType packetType;
	struct Helper final
	{
		enum HelpState
		{
			Killer = 0,
			Healer,
			SuccessReport,
			DefeatReport,
			Prevention
		};
		HelpState helpState;
		std::vector<Signature> sign;
		bool isAlarm = false;
	};
	Helper* sHelper;
	struct Information final
	{
		bool isDSRequest = false;
		short key_info_count = 0;
		std::vector<short> roots_for_id;
		AActor* for_spy_ref = nullptr;
	};
	Information* sInformation;
	struct Threat final
	{
		Signature sign = Signature::NotSign;
		bool have_root = false;
		int spy_id = -2;
	};
	Threat* sThreat;

	UFUNCTION(BlueprintCallable)
	static void FillMaterials(UMaterialInterface* simple, UMaterialInterface* spam, UMaterialInterface* attack, UMaterialInterface* info, UMaterialInterface* help);
	static UMaterialInterface* simpleMat;
	static UMaterialInterface* spamMat;
	static UMaterialInterface* attackMat;
	static UMaterialInterface* infoMat;
	static UMaterialInterface* helpMat;

	struct PacketMove final
	{
		float ComputeNodePath(const AActor* source, const AActor* target, const ALink* _link);
		bool iden = false;
		std::vector<FVector> path;
		std::vector<FVector>::iterator it_path;
		const ALink* link = nullptr;
	};
	PacketMove* sPacketMove;

};
