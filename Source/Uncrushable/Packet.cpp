
#pragma once

#include "Packet.h"

enum EPacketMaterials : uint8
{
	SIMPLE = 0,
	SPAM = 1,
	ATTACK = 2,
	INFORMATIVE = 3,
	HELPFUL = 4
};

TArray<UMaterialInterface*> APacket::materials = {};


APacket::APacket()
	: size(0),
	source_id(-2), target_id(-2),
	packetType(EPacketType::Simple),
	path(std::stack<AActor*>()),
	sHelper(nullptr), sInformation(nullptr), sThreat(nullptr), 
	Mesh(CreateDefaultSubobject<UStaticMeshComponent>("Packet_charge")),
	sPacketMove(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = Mesh;
}

APacket::~APacket()
{
	delete sHelper;
	delete sInformation;
	delete sThreat;
	delete sHelper;
	Mesh->DestroyComponent();
	delete Mesh;
}



void APacket::BeginPlay()
{
	Super::BeginPlay();
}

void APacket::FillMaterials(TArray<UMaterialInterface*>& _materials)
{
	materials = _materials;
}

void APacket::Tick(float DeltaTime)
{
	if (sPacketMove->bIsMoving) {
		if (!sPacketMove->link || !sPacketMove->link->bIsAlive) {
			this->Destroy();
		} 
		else {
			this->SetActorLocation(sPacketMove->vector_path.back());
			sPacketMove->vector_path.pop();
			if (sPacketMove->vector_path.size() <= 0) {
				sPacketMove->bIsMoving = false;
				sPacketMove->f_checkPacket(this);
			}
		}
	}
	Super::Tick(DeltaTime);
}

void APacket::InitPacket(EPacketType _packetType, int _source_id, int _target_id, std::stack<AActor*> _path)
{
	packetType = _packetType;
	switch (_packetType) {
	case EPacketType::Simple:
		Mesh->SetMaterial(0, materials[EPacketMaterials::SIMPLE]);
		size = 4;
		break;
	case EPacketType::Informative:
		Mesh->SetMaterial(0, materials[EPacketMaterials::INFORMATIVE]);
		size = 12;
		break;
	case EPacketType::AttackSpam:
		Mesh->SetMaterial(0, materials[EPacketMaterials::SPAM]);
		size = 4;
		break;
	case EPacketType::Helpful:
		Mesh->SetMaterial(0, materials[EPacketMaterials::HELPFUL]);
		size = 8;
		sHelper = new FHelper();
		break;
	case EPacketType::AttackCapture:
	case EPacketType::AttackCrash:
		Mesh->SetMaterial(0, materials[EPacketMaterials::ATTACK]);
		size = 6;
		sThreat = new FThreat();
		break;
	default:
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Can't init packet!");
		break;
	}
	source_id = _source_id;
	target_id = _target_id;
	path = _path;
}

APacket::FPacketMove::FPacketMove(ALink* const& _link, const std::function<void(APacket* packet)>& f)
	: bIsMoving(false),
	link(_link),
	f_checkPacket(f),
	vector_path(std::queue<FVector>())
{
}

void APacket::InitPacketMove(AActor* const& source,  AActor* const& target, ALink* const& _link, const std::function<void(APacket* packet)>& f)
{
	if (!sPacketMove) {
		sPacketMove = new FPacketMove(_link, f);
	} 
	else {
		sPacketMove->link = _link;
		sPacketMove->f_checkPacket = f;
	}

	float speed = 4 * _link->speed_coef;
	FVector range = target->GetActorLocation() - source->GetActorLocation();
	FVector start_pos = source->GetActorLocation();

	int max_i = static_cast<int>(range.Size() / speed);
	FVector step_size = range / max_i;

	for (int i = 0; i < max_i; ++i) {
		sPacketMove->vector_path.push(start_pos);
		start_pos += step_size;
	}
	float time = max_i / 60.0f;//60 FPS ?
	sPacketMove->link->AddWorkloadWithDelay(size, time);

	sPacketMove->bIsMoving = true;//Packet start moveing in Tick()
}

APacket::FInformation::FInformation(bool _isDSRequest, uint8 _key_info_count, std::vector<uint8> _roots_for_id, AActor* _for_spy_ref)
	: isDSRequest(_isDSRequest),
	key_info_count(_key_info_count),
	roots_for_id(_roots_for_id),
	for_spy_ref(_for_spy_ref)
{
}
