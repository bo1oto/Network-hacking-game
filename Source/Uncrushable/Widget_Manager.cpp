#include "Widget_Manager.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>

UWidget_Manager* UWidget_Manager::self_ref = nullptr;
bool UWidget_Manager::isGameStart = false;

FNodeInfo::FNodeInfo()
{
	node_id = -1;
	isFullInfo = false;
	node_ptr = nullptr;
	characteristic = "Empty object";
}
FNodeInfo::FNodeInfo(int _node_id)
{
	node_id = _node_id;
	isFullInfo = false;
	node_ptr = nullptr;
	characteristic = "No information\n available :(";
}
FNodeInfo::FNodeInfo(ANodeBase* _node_ptr)
{
	node_id = _node_ptr->id;
	node_ptr = _node_ptr; 
	isFullInfo = true;
	node_ptr->Mesh->SetMaterial(0, node_ptr->main_mat);
	characteristic = UWidget_Manager::FillNodeCharacteristic(_node_ptr);
}

void UWidget_Manager::SetSelfRef(UWidget_Manager* _self_ref)
{
	UWidget_Manager::self_ref = _self_ref;
}
void UWidget_Manager::StartGame()
{
	TArray<AActor*> nodes_arr = TArray<AActor*>();
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANodePC::StaticClass(), nodes_arr);
	generation_timers = new std::vector<FTimerHandle>();
	for (auto node : nodes_arr)
	{
		FTimerHandle timer = FTimerHandle();
		node->GetWorldTimerManager().SetTimer(timer, [nodePC = (ANodePC*)node]
		{
			if (UWidget_Manager::isGameStart && nodePC->nodeState != NodeState::Overloaded 
				&& nodePC->nodeState != NodeState::Offline && !nodePC->nodeLinks.empty())
			{
				nodePC->GeneratePacket(std::rand() % (101));
			}
		}, network_activity_time_tick, true, network_activity_time_tick);
		generation_timers->push_back(timer);
	}
	isGameStart = true;
}

FString UWidget_Manager::FillNodeCharacteristic(const ANodeBase* node_ptr)
{
	FString str = "Type: ";
	switch (node_ptr->nodeType)
	{
	case NodeType::PersonalComputer: str += TEXT("PC"); break;
	case NodeType::ExternalOutput: str += TEXT("EO"); break;
	case NodeType::TechnicalInfrastructure: str += TEXT("TI"); break;
	case NodeType::Security: str += TEXT("SC"); break;
	case NodeType::DataStorage: str += TEXT("DS"); break;
	default: str += TEXT("???"); break;
	}
	if (node_ptr->sProtection != nullptr)
	{
		str += TEXT("\nSignatures: ");
		for (auto sign : node_ptr->sProtection->threatSigns)
		{
			switch (sign)
			{
			case Signature::Crash_1: str += TEXT("C, "); break;
			case Signature::Crash_2: str += TEXT("C+, "); break;
			case Signature::RootKit_1: str += TEXT("R, "); break;
			case Signature::RootKit_2: str += TEXT("R+, "); break;
			case Signature::Spy_1: str += TEXT("S, "); break;
			case Signature::Spy_2: str += TEXT("S+, "); break;
			default: str += TEXT("???");
			}
		}
		str += TEXT("\nSpam filter: ");
		if (node_ptr->sProtection->spamFilter) str += TEXT("Present");
		else str += TEXT("Absent");
	}
	else
	{
		str += TEXT("\nNo protection");
	}
	return str;
}

void UWidget_Manager::AddKeyInfo(short quantity)
{
	key_info_counter += quantity;
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Key info progress: " + FString::FromInt(key_info_counter) + "/20 !");
	if (key_info_counter >= 20)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Congratulation!");
	}
}
void UWidget_Manager::AddNodeInfo(ANodeBase* node_ptr, bool asID)
{
	auto f = [node_ptr](const FNodeInfo &node_info) -> bool
	{
		return node_info.node_id == node_ptr->id;
	};
	if (asID)
	{
		if (known_nodes.ContainsByPredicate(f)) return;
		known_nodes.Add(FNodeInfo(node_ptr->id));
		node_ptr->SetActorHiddenInGame(false);
		//node_ptr->SetActorEnableCollision(true);
	}
	else
	{
		node_ptr->Mesh->SetMaterial(0, node_ptr->main_mat);
		node_ptr->SetActorHiddenInGame(false);
		//node_ptr->SetActorEnableCollision(true);
		FNodeInfo* fast_ptr = known_nodes.FindByPredicate(f);
		if (fast_ptr != nullptr)
		{
			fast_ptr->node_ptr = node_ptr;
			fast_ptr->characteristic = UWidget_Manager::FillNodeCharacteristic(node_ptr);
			fast_ptr->isFullInfo = true;
		}
		else
		{
			known_nodes.Add(FNodeInfo(node_ptr));
		}
		for (auto nodeLink : node_ptr->nodeLinks)
		{
			AddNodeInfo(nodeLink->targetNode, true);
			nodeLink->targetNode->SetActorHiddenInGame(false);
			//nodeLink->targetNode->SetActorEnableCollision(true);
			nodeLink->link->SetActorHiddenInGame(false);
		}
	}
	
}

void UWidget_Manager::InitSpamAttack(int target_node_id, ANodeBase* source_node, int spoof_id)
{
	std::vector<ANodeBase*>* nodes = source_node->DeterminePath(target_node_id);
	if (nodes != nullptr && !(*nodes).empty())
	{
		APacket* packet = GetWorld()->SpawnActor<APacket>(ANodeBase::packetTemp, source_node->GetActorLocation(), FRotator(0, 0, 0), FActorSpawnParameters());
		packet->Mesh->SetMaterial(0, APacket::spamMat);
		packet->InitPacket(PacketType::AttackSpam, source_node->id, target_node_id);
		if (spoof_id >= 0)
		{
			packet->source_id = spoof_id;
		}
		source_node->SendPacket(packet, nodes, (*nodes).begin());
	}
}
void UWidget_Manager::InitAttack(int target_node_id, ANodeBase* source_node, bool upThreat, int spoof_id, int attack_type)
{
	std::vector<ANodeBase*>* nodes = source_node->DeterminePath(target_node_id);
	PacketType _packetType;
	Signature _sign;
	switch (attack_type)
	{
	case 0:
		_packetType = PacketType::AttackCapture;
		if (upThreat) _sign = Signature::RootKit_2; else _sign = Signature::RootKit_1;
		break;
	case 1:
		_packetType = PacketType::AttackCrash;
		if (upThreat) _sign = Signature::Crash_2; else _sign = Signature::Crash_1;
		break;
	case 2:
		_packetType = PacketType::AttackSpy;
		if (upThreat) _sign = Signature::Spy_2; else _sign = Signature::Spy_1;
		break;
	default:
		_packetType = PacketType::Simple;
		_sign = Signature::NotSign;
		break;
	}
	if (nodes != nullptr && !(*nodes).empty())
	{
		APacket* packet = GetWorld()->SpawnActor<APacket>(ANodeBase::packetTemp, source_node->GetActorLocation(), FRotator(0, 0, 0), FActorSpawnParameters());
		packet->InitPacket(_packetType, source_node->id, target_node_id);
		packet->sThreat->sign = _sign;
		if (spoof_id >= 0)
		{
			packet->source_id = spoof_id;
		}
		if (attack_type == 2) packet->sThreat->spy_id = source_node->id;
		if (roots.Contains(target_node_id)) packet->sThreat->have_root = true;
		source_node->SendPacket(packet, nodes, (*nodes).begin());
	}
}
void UWidget_Manager::InitInformative(int target_node_id, ANodeBase* source_node, int spoof_id)
{
	std::vector<ANodeBase*>* nodes = source_node->DeterminePath(target_node_id);
	if (nodes != nullptr && !(*nodes).empty())
	{
		APacket* packet = GetWorld()->SpawnActor<APacket>(ANodeBase::packetTemp, source_node->GetActorLocation(), FRotator(0, 0, 0), FActorSpawnParameters());
		packet->InitPacket(PacketType::Informative, source_node->id, target_node_id);
		packet->sInformation->for_spy_ref = source_node;
		if (source_node->sInformation)
		{
			packet->sInformation->key_info_count = source_node->sInformation->key_info_count;
		}
		if (spoof_id >= 0)
		{
			packet->source_id = spoof_id;
		}
		source_node->SendPacket(packet, nodes, (*nodes).begin());
	}
}
