
#pragma once

#include "Packet.h"


UMaterialInterface* APacket::simpleMat = nullptr;
UMaterialInterface* APacket::spamMat = nullptr;
UMaterialInterface* APacket::attackMat = nullptr;
UMaterialInterface* APacket::infoMat = nullptr;
UMaterialInterface* APacket::helpMat = nullptr;


APacket::APacket()
{
	PrimaryActorTick.bCanEverTick = true;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Packet_charge");
	RootComponent = Mesh;
}

void APacket::InitPacket(EPacketType _packetType, short _sourceId, short _targetId, std::vector<AActor*> _path)
{
	packetType = _packetType;
	switch (_packetType) {
	case EPacketType::Simple:
		Mesh->SetMaterial(0, simpleMat);
		size = 4;
		break;
	case EPacketType::Informative:
		Mesh->SetMaterial(0, infoMat);
		size = 12;
		break;
	case EPacketType::AttackSpam:
		Mesh->SetMaterial(0, spamMat);
		size = 4;
		break;
	case EPacketType::Helpful:
		Mesh->SetMaterial(0, helpMat);
		size = 8;
		sHelper = new FHelper();
		break;
	default:
		Mesh->SetMaterial(0, attackMat);
		size = 6;
		sThreat = new FThreat();
		break;
	}
	source_id = _sourceId;
	target_id = _targetId;
	path = _path;
}

void APacket::BeginPlay()
{
	Super::BeginPlay();
	sPacketMove = new FPacketMove();
}

void APacket::Tick(float DeltaTime)
{
	if (sPacketMove->bIsMoving && sPacketMove->link->bIsAlive) {
		this->SetActorLocation(sPacketMove->it_path[0]);
		++sPacketMove->it_path;
		if (sPacketMove->it_path == sPacketMove->vector_path.end()) {
			sPacketMove->bIsMoving = false;
		}
	}
	Super::Tick(DeltaTime);
}

void APacket::FillMaterials(UMaterialInterface* simple, UMaterialInterface* spam, UMaterialInterface* attack, UMaterialInterface* info, UMaterialInterface* help)
{
	simpleMat = simple;
	spamMat = spam;
	attackMat = attack;
	infoMat = info;
	helpMat = help;
}

APacket::FInformation::FInformation(bool _isDSRequest, uint8 _key_info_count, std::vector<uint8> _roots_for_id, AActor* _for_spy_ref) 
	: isDSRequest(_isDSRequest), 
	key_info_count(_key_info_count), 
	roots_for_id(_roots_for_id), 
	for_spy_ref(_for_spy_ref) {
}

float APacket::FPacketMove::ComputeNodePath(const AActor& source, const AActor& target, const ALink* _link)
{
	float speed = 4 * _link->speed_coef;
	link = _link;
	vector_path.clear();
	FVector range = target.GetActorLocation() - source.GetActorLocation();
	int max_i = static_cast<int>(range.Size() / speed);
	FVector step_size = range / max_i;
	FVector start_pos = source.GetActorLocation();
	for (int i = 0; i < max_i; i++) {
		vector_path.push_back(start_pos);
		start_pos += step_size;
	}
	it_path = vector_path.begin();
	bIsMoving = true;
	float time = max_i / 60.0f;//60 FPS ?
	return time;
}
