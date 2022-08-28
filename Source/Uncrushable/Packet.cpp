
#pragma once
#include "Packet.h"

UMaterialInterface* APacket::simpleMat = nullptr;
UMaterialInterface* APacket::spamMat = nullptr;
UMaterialInterface* APacket::attackMat = nullptr;
UMaterialInterface* APacket::infoMat = nullptr;
UMaterialInterface* APacket::helpMat = nullptr;


void APacket::BeginPlay()
{
	Super::BeginPlay();
	sPacketMove = new FPacketMove();
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

APacket::APacket()
{
	PrimaryActorTick.bCanEverTick = true;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Packet_charge");
	RootComponent = Mesh;
}

void APacket::InitPacket(EPacketType _packetType, short _sourceId, short _targetId, std::vector<AActor*> _path)
{
	packetType = _packetType;
	switch (_packetType)
	{
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
		sHelper = new Helper();
		break;
	default:
		Mesh->SetMaterial(0, attackMat);
		size = 6;
		sThreat = new Threat();
		break;
	}
	source_id = _sourceId;
	target_id = _targetId;
	path = _path;
}


APacket::Information::Information(bool _isDSRequest, uint8 _key_info_count, std::vector<uint8> _roots_for_id, AActor* _for_spy_ref) :
	isDSRequest(_isDSRequest), key_info_count(_key_info_count), roots_for_id(_roots_for_id), for_spy_ref(_for_spy_ref)
{
}


//////////////////// Visual //////////////////////////

void APacket::FillMaterials(UMaterialInterface* simple, UMaterialInterface* spam, UMaterialInterface* attack, UMaterialInterface* info, UMaterialInterface* help)
{
	simpleMat = simple;
	spamMat = spam;
	attackMat = attack;
	infoMat = info;
	helpMat = help;
}

float APacket::FPacketMove::ComputeNodePath(const AActor& source, const AActor& target, const ALink* _link)
{
	float speed = 4 * _link->speed_coef;
	link = _link;
	path.clear();
	FVector range = target.GetActorLocation() - source.GetActorLocation();
	int max_i = (int)(range.Size() / speed);
	FVector _vec = range / max_i;
	FVector vec = source.GetActorLocation();
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

