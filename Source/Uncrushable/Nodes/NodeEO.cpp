
#include "NodeEO.h"
#include "Uncrushable/Widget_Manager.h"

int ANodeEO::id_counter = 0;

ANodeEO::ANodeEO() : ANodeBase()
{
}

void ANodeEO::BeginPlay()
{
	ANodeBase::BeginPlay();
	nodeType = NodeType::ExternalOutput;
	nodeState = NodeState::Captured;
	delete sProtection;
	sProtection = nullptr;
	id = id_counter;
	id_counter++;
}

void ANodeEO::AcceptPacket(APacket* packet)
{
	if (nodeState == NodeState::Captured && packet->packetType == EPacketType::Simple && packet->sInformation && packet->sInformation->for_spy_ref)
	{
		UWidget_Manager::self_ref->AddNodeInfo((ANodeBase*)(packet->sInformation->for_spy_ref), false);
		
		if (packet->sInformation->key_info_count) UWidget_Manager::self_ref->AddKeyInfo(packet->sInformation->key_info_count);
		if (!packet->sInformation->roots_for_id.empty())
		{
			for (auto root : packet->sInformation->roots_for_id)
			{
				if (!UWidget_Manager::self_ref->known_nodes.ContainsByPredicate([root](FNodeInfo nodeInfo) -> bool
				{
					return nodeInfo.node_id == root && nodeInfo.node_ptr && nodeInfo.node_ptr->nodeState == NodeState::Captured;
				}))
				{
					if (!UWidget_Manager::self_ref->roots.Contains(root)) 
						UWidget_Manager::self_ref->roots.Add(root);
				}
			}
		}
		ANodeBase::Information* fast_ptr = ((ANodeBase*)packet->sInformation->for_spy_ref)->sInformation;
		if (fast_ptr)
		{
			for (auto elem : fast_ptr->vec_net_id)
			{
				UWidget_Manager::self_ref->AddNodeInfo(elem, true);
			}
		}
	}
	if (nodeState == NodeState::Captured && packet->packetType == EPacketType::Informative && packet->sInformation->key_info_count)
	{
		UWidget_Manager::self_ref->AddKeyInfo(packet->sInformation->key_info_count);
	}
	ANodeBase::AcceptPacket(packet);
}
