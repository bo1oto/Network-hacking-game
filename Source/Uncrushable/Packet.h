
#pragma once

#include "CoreMinimal.h"

#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"

#include<queue>
#include <stack>
#include <functional>

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

private:
	struct FPacketMove final
	{
		FPacketMove(ALink* const& _link, const std::function<void(APacket* packet)>& f);
		bool bIsMoving = false;
		ALink* link = nullptr;
		std::function<void(APacket* packet)> f_checkPacket;
		std::queue<FVector> vector_path;
	};

public:
	APacket();
	//copy constructor
	//= operator
	~APacket();

protected:
	virtual void BeginPlay() final;

public:
	UFUNCTION(BlueprintCallable)
	static void FillMaterials(TArray<UMaterialInterface*>& _materials);

	virtual void Tick(float DeltaTime) final;

	void InitPacket(EPacketType _packetType, int _source_id, int _target_id, std::stack<AActor*> _path);

	void InitPacketMove(AActor* const& source, AActor* const& target, ALink* const& _link, const std::function<void(APacket* packet)>& f);

public:
	int size;
	int source_id, target_id;
	EPacketType packetType;
	std::stack<AActor*> path;

	FHelper* sHelper;
	FInformation* sInformation;
	FThreat* sThreat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* Mesh;

private:
	static TArray<UMaterialInterface*> materials;

	FPacketMove* sPacketMove;

};
