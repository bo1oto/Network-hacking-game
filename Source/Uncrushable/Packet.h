
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
			Prevention,
			Alarm
		};

		FHelper() = delete;
		FHelper(EHelpState _eHelpState, bool _bIsRaiseAlarm);
		EHelpState eHelpState;
		bool bIsRaiseAlarm = false;
	};
	struct FInformation final
	{
		FInformation() = delete;
		FInformation(bool _isDSRequest, uint8 _key_info_count, std::vector<uint8> _roots_for_id, std::vector<int> _nodes_info);
		bool isDSRequest;
		uint8 key_info_count;
		std::vector<uint8> roots_for_id;
		std::vector<int> nodes_info;
	};
	struct FThreat final
	{
		FThreat() = delete;
		FThreat(ESignature _sign, bool _have_root, int _spy_master_id = -2);
		ESignature sign = ESignature::NotSign;
		bool have_root = false;
		int spy_master_id;
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
	~APacket();

protected:
	virtual void BeginPlay() final;

public:
	UFUNCTION(BlueprintCallable)
	static void FillMaterials(TArray<UMaterialInterface*>& _materials) noexcept;

	virtual void Tick(float DeltaTime) final;

	void InitPacket(EPacketType _packetType, int _source_id, int _target_id, std::stack<AActor*> _path) noexcept;

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
