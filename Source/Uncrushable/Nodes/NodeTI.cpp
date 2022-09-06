#include "NodeTI.h"

#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>

#include <functional>

#include "NodeSC.h"


int ANodeTI::id_counter = 10;
int ANodeTI::vlan_counter = 1;


ANodeTI::ANodeTI() : ANodeBase()
{
}

void ANodeTI::BeginPlay()
{
	ANodeBase::BeginPlay();
	AddWorkload(20);
	eNodeType = ENodeType::TechnicalInfrastructure;
	routingTable = {};
	NodeID = id_counter;
	id_counter++;
	VLAN = vlan_counter;
	vlan_counter++;
	sInformation = new FInformation();
}
		

bool ANodeTI::CheckIDInTable(int _id) const
{
	auto contains = [](const std::vector<int>& id_vec, int node_id) -> bool
	{
		for (const auto& elem : id_vec)
		{
			if (elem == node_id) 
				return true;
		}
		return false;
	};
	for (const auto& routes : routingTable)
	{
		if (contains(routes->id_numbers, _id)) 
			return true;
	}
	return false;
}

void ANodeTI::AcceptPacket(APacket* packet)
{
	if (packet->target_id != NodeID) {
		if (packet->packetType == EPacketType::Helpful && packet->sHelper && packet->sHelper->bIsRaiseAlarm && eNodeState != ENodeState::Captured) {
			// Then look for the closest known security node
			std::stack<AActor*> main{}, bolv{};
			int _id = -2;
			for (int i = ANodeSC::id_counter - 1; i >= 30; --i) {
				if (CheckIDInTable(i)) {
					ComputeNodePath(this, i, main);
					if (bolv.empty() || main.size() < bolv.size())
					{
						_id = i;
						bolv = main;
					}
				}
			}
			if (!bolv.empty()) {
				packet->target_id = _id;
				packet->path.swap(bolv);
				SendPacket(packet);
			}
			goto add_work;

		}
		if (CheckIDInTable(packet->target_id))
		{
			std::stack<AActor*> nodes_stack{};
			ComputeNodePath(this, packet->target_id, nodes_stack);
			if (!nodes_stack.empty())
			{
				packet->path.swap(nodes_stack);
				SendPacket(packet);
				goto add_work;
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Not find in Table");
			packet->Destroy();
		}
	add_work:
		AddTemporaryWorkload(3, 0.2f);
	}
	else
	{
		ANodeBase::AcceptPacket(packet);
	}

}

void ANodeTI::CreateVLAN(ANodeTI* node)
{
	Routing* route = new Routing(node->VLAN);
	routingTable.push_back(route);
	FillVLAN(route->id_numbers, route->VLAN);
}

void ANodeTI::FillVLAN(std::vector<int>& id_vec, int vlan_num)
{
	auto contains = [](const std::vector<int>& id_vec, int node_id) -> bool
	{
		for (auto elem : id_vec)
		{
			if (elem == node_id) return true;
		}
		return false;
	};

	std::function<void(std::vector<int>& id_vec, ANodeBase* target)> f = 
		[&f, vlan_num, &contains](std::vector<int>& id_vec, ANodeBase* target) -> void
	{
		for (auto nodeLink : target->nodeLinks)
		{
			if (nodeLink->targetNode->VLAN == vlan_num && !contains(id_vec, nodeLink->targetNode->NodeID))
			{
				id_vec.push_back(nodeLink->targetNode->NodeID);
				f(id_vec, nodeLink->targetNode);
			}
		}
	};
	f(id_vec, this);
	if (VLAN == vlan_num && !contains(id_vec, NodeID)) id_vec.push_back(NodeID);
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, (TEXT("VLAN ") + FString::FromInt(VLAN) + TEXT(" members: ") + FString::FromInt(id_vec.size())));

	TArray<AActor*> nodes_arr = TArray<AActor*>();
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANodeBase::StaticClass(), nodes_arr);
	nodes_arr = nodes_arr.FilterByPredicate( [id_vec, contains](AActor* const &NodePtr)
	{
		return contains(id_vec, ((ANodeBase*)NodePtr)->NodeID);
	});
	for (auto elem : nodes_arr)
	{
		AddInformation((ANodeBase*)elem);
	}
}


ANodeTI::Routing::Routing(int vlan_num)
{
	VLAN = vlan_num;
	id_numbers = {};
}
