
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

void APacket::BeginPlay()
{
	Super::BeginPlay();
	sPacketMove = new PacketMove();
}
void APacket::Tick(float DeltaTime)
{
	if (sPacketMove->iden && sPacketMove->link->isAlive)
	{
		this->SetActorLocation(*(sPacketMove->it_path));
		sPacketMove->it_path++;
		if (sPacketMove->it_path == sPacketMove->path.end())
		{
			sPacketMove->iden = false;
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

float APacket::PacketMove::ComputeNodePath(const AActor* source, const AActor* target, const ALink* _link)
{
	float speed = 4 * _link->speed_coef;
	link = _link;
	path.clear();
	FVector range = target->GetActorLocation() - source->GetActorLocation();
	int max_i = (int)(range.Size() / speed);
	FVector _vec = range / max_i;
	FVector vec = source->GetActorLocation();
	for (int i = 0; i < max_i; i++)
	{
		path.push_back(vec);
		vec += _vec;
	}
	it_path = path.begin();
	iden = true;
	float time = max_i / 60.0f;//60 FPS
	return time;
}
void APacket::InitPacket(PacketType _packetType, short _sourceId, short _targetId)
{
	packetType = _packetType;
	switch (_packetType)
	{
	case PacketType::Simple:
	{	
		Mesh->SetMaterial(0, simpleMat);
		size = 4;
	} break;
	case PacketType::Informative: 
	{
		Mesh->SetMaterial(0, infoMat);
		size = 12;
		sInformation = new Information();
	} break;
	case PacketType::AttackSpam: 
	{
		Mesh->SetMaterial(0, spamMat);
		size = 4;
	} break;
	case PacketType::Helpful: 
	{
		Mesh->SetMaterial(0, helpMat);
		size = 8;
		sHelper = new Helper();
	} break;
	default: 
	{
		Mesh->SetMaterial(0, attackMat);
		size = 6;
		sThreat = new Threat();
	} break;
	}
	source_id = _sourceId;
	target_id = _targetId;
}

