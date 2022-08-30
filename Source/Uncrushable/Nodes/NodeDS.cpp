
#pragma once

#include "NodeDS.h"


int ANodeDS::id_counter = 50;


ANodeDS::ANodeDS() : ANodeBase() {
}

void ANodeDS::BeginPlay()
{
	ANodeBase::BeginPlay();
	AddWorkload(20);
	eNodeType = ENodeType::DataStorage;
	id = id_counter;
	id_counter++;
	sInformation = new FInformation();
	sInformation->key_info_count = 20;
}

void ANodeDS::AcceptPacket(APacket* packet)
{
	if (packet->packetType == EPacketType::Informative && packet->sInformation->isDSRequest)
	{
		packet->sInformation = new APacket::FInformation(false, 1, {}, nullptr);
		SendPacket(packet);
	}
	ANodeBase::AcceptPacket(packet);
}
