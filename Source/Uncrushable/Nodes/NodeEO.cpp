
#pragma once

#include "NodeEO.h"

#include "Uncrushable/Widget_Manager.h"


int ANodeEO::id_counter = 0;


ANodeEO::ANodeEO() : ANodeBase()
{
}

void ANodeEO::BeginPlay()
{
	ANodeBase::BeginPlay();
	eNodeType = ENodeType::ExternalOutput;
	eNodeState = ENodeState::Captured;
	delete sProtection;
	sProtection = nullptr;
	id = id_counter;
	id_counter++;
}

void ANodeEO::AcceptPacket(APacket* packet)
{
	if (eNodeState == ENodeState::Captured && packet->packetType == EPacketType::Simple && packet->sInformation)
	{
		if (packet->sInformation->key_info_count) {
			UWidget_Manager::self_ref->AddKeyInfo(packet->sInformation->key_info_count);
		}

		for (const auto root_id : packet->sInformation->roots_for_id) {
			if ((*UWidget_Manager::all_nodes.Find(root_id))->eNodeState != ENodeState::Captured) {
				UWidget_Manager::self_ref->roots.Add(root_id);
			}
		}

		for (const auto& elem : packet->sInformation->nodes_info)
		{
			UWidget_Manager::self_ref->AddNodeInfo(elem, true);
		}
	}
	if (eNodeState == ENodeState::Captured && packet->packetType == EPacketType::Informative && packet->sInformation->key_info_count) {
		UWidget_Manager::self_ref->AddKeyInfo(packet->sInformation->key_info_count);
	}
	ANodeBase::AcceptPacket(packet);
}
