
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
	if (sPacketMove->iden)
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

float APacket::PacketMove::ComputeNodePath(AActor* source, AActor* target)
{
	path.clear();
	float _x, _y, _z;
	float speed = 5;
	FVector range = target->GetActorLocation() - source->GetActorLocation();
	int max_i = (int)(range.Size() / speed);
	
	_x = range.X / max_i;
	_y = range.Y / max_i;
	_z = range.Z / max_i;
	FVector vec = source->GetActorLocation();
	for (int i = 0; i < max_i; i++)
	{
		path.push_back(vec);
		vec += FVector(_x, _y, _z);
	}
	path.push_back(target->GetActorLocation());
	it_path = path.begin();
	iden = true;
	return (max_i + 1.0f) / 60.0f;
}
void APacket::InitPacket(PacketType _packetType, short _sourceId, short _targetId)
{
	packetType = _packetType;
	switch (_packetType)
	{
	case PacketType::Simple: Mesh->SetMaterial(0, simpleMat); break;
	case PacketType::Informative: 
	{
		Mesh->SetMaterial(0, infoMat);
		sInformation = new Information();
	} break;
	case PacketType::AttackSpam: Mesh->SetMaterial(0, spamMat); break;
	case PacketType::Helpful: 
	{
		Mesh->SetMaterial(0, helpMat);
		sHelper = new Helper();
	} break;
	default: 
	{
		Mesh->SetMaterial(0, attackMat);
		sThreat = new Threat();
	} break;
	}
	source_id = _sourceId;
	target_id = _targetId;
}

