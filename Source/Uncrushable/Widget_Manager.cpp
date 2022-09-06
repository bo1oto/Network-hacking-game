#include "Widget_Manager.h"

#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>


UWidget_Manager* UWidget_Manager::self_ref = nullptr;
bool UWidget_Manager::bIsGameStart = false;
TMap<int, ANodeBase*> UWidget_Manager::all_nodes = {};


FNodeInfo::FNodeInfo() :
	ID(-1), NodePtr(nullptr),
	Characteristic(TEXT("Empty object")), 
	bIsFullInfo(false) {
}
FNodeInfo::FNodeInfo(int _node_id) :
	ID(_node_id),
	NodePtr(nullptr), 
	Characteristic(TEXT("No information\n available :(")),
	bIsFullInfo(false) {
}
FNodeInfo::FNodeInfo(ANodeBase* _node_ptr) :
	ID(_node_ptr->NodeID),
	NodePtr(_node_ptr), 
	Characteristic(UWidget_Manager::FillNodeCharacteristic(*_node_ptr)), 
	bIsFullInfo(true) {
	NodePtr->Mesh->SetMaterial(0, NodePtr->Material);
}


void UWidget_Manager::SetSelfRef(UWidget_Manager* _self_ref) noexcept
{
	UWidget_Manager::self_ref = _self_ref;
}

void UWidget_Manager::AddToAllNodes(int id, ANodeBase* NodePtr) noexcept
{
	UWidget_Manager::all_nodes.Emplace(id, NodePtr);
}

FString UWidget_Manager::FillNodeCharacteristic(const ANodeBase& NodePtr)
{
	FString str = "Type: ";
	switch (NodePtr.eNodeType)
	{
		case ENodeType::PersonalComputer:			str += TEXT("PC"); break;
		case ENodeType::ExternalOutput:				str += TEXT("EO"); break;
		case ENodeType::TechnicalInfrastructure:	str += TEXT("TI"); break;
		case ENodeType::Security:					str += TEXT("SC"); break;
		case ENodeType::DataStorage:				str += TEXT("DS"); break;
		default:									str += TEXT("???"); break;
	}
	if (NodePtr.sProtection != nullptr) {
		str += TEXT("\nSignatures: ");
		for (const auto& sign : NodePtr.sProtection->threatSigns)
		{
			switch (sign)
			{
			case ESignature::Crash_1:	str += TEXT("C, "); break;
			case ESignature::Crash_2:	str += TEXT("C+, "); break;
			case ESignature::RootKit_1:	str += TEXT("R, "); break;
			case ESignature::RootKit_2:	str += TEXT("R+, "); break;
			case ESignature::Spy_1:		str += TEXT("S, "); break;
			case ESignature::Spy_2:		str += TEXT("S+, "); break;
			default:					str += TEXT("???");
			}
		}
		str += NodePtr.sProtection->bSpamFilter ? TEXT("\nSpam filter: Present") : TEXT("\nSpam filter: Absent");
	}
	else {
		str += TEXT("\nNo protection");
	}
	return str;
}

void UWidget_Manager::StartGame()
{
	TArray<AActor*> nodes_arr = TArray<AActor*>();
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANodePC::StaticClass(), nodes_arr);
	generation_timers = new std::vector<FTimerHandle>();
	for (const auto& node : nodes_arr) {
		FTimerHandle timer = FTimerHandle();
		// can't change Tick delta time for NetworkActivityInterval cause it's affect node physics (maybe change tick after game start)
		node->GetWorldTimerManager().SetTimer(timer, [nodePC = static_cast<ANodePC*>(node)]() -> void {
			if (UWidget_Manager::bIsGameStart && nodePC->eNodeState != ENodeState::Overloaded 
				&& nodePC->eNodeState != ENodeState::Offline && !nodePC->nodeLinks.empty()) {
				nodePC->GeneratePacket(std::rand() % 101);
			}
		}, 
		NetworkActivityInterval, true, NetworkActivityInterval);
		generation_timers->push_back(timer);
	}
	bIsGameStart = true;
}

void UWidget_Manager::AddNodeInfo(int node_id, bool bAsID)
{
	auto f = [node_id](const FNodeInfo& node_info) -> bool {
		return node_info.ID == node_id;
	};
	TMap<int, ANodeBase*>& heh = UWidget_Manager::all_nodes;
	if (!UWidget_Manager::all_nodes.Contains(node_id)) {
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Can't find ID in all_nodes: " + FString::FromInt(node_id)));
		return;
	}
	ANodeBase* NodePtr = UWidget_Manager::all_nodes[node_id];

	NodePtr->SetActorHiddenInGame(false);
	//NodePtr->SetActorEnableCollision(true);

	if (bAsID) {
		if (!known_nodes.ContainsByPredicate(f)) {
			known_nodes.Add(FNodeInfo(NodePtr->NodeID));
		}
	}
	else {
		NodePtr->Mesh->SetMaterial(0, NodePtr->Material);
		FNodeInfo* fast_ptr = known_nodes.FindByPredicate(f);
		if (fast_ptr) {
			fast_ptr->NodePtr = NodePtr;
			fast_ptr->Characteristic = UWidget_Manager::FillNodeCharacteristic(*NodePtr);
			fast_ptr->bIsFullInfo = true;
		}
		else {
			known_nodes.Add(FNodeInfo(NodePtr));
		}

		for (const auto& nodeLink : NodePtr->nodeLinks) {
			nodeLink->link->SetActorHiddenInGame(false);
			AddNodeInfo(nodeLink->targetNode->NodeID, true);
		}
	}

}

void UWidget_Manager::AddKeyInfo(int quantity)
{
	nKeyInfo += quantity;
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Key info progress: " + FString::FromInt(nKeyInfo) + "/20 !");
	if (nKeyInfo >= 20) {
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Congratulation! You Win!");
	}
}

void UWidget_Manager::InitSpamAttack(int target_id, ANodeBase* source_node, int spoof_id)
{
	APacket* packet = source_node->CreatePacket(target_id, EPacketType::AttackSpam, spoof_id);
	if (!packet) {
		return;
	}

	source_node->SendPacket(packet);
}
void UWidget_Manager::InitAttack(int target_id, ANodeBase* source_node, bool upThreat, int spoof_id, int attack_type)
{
	EPacketType _packet_type;
	ESignature _sign;
	switch (attack_type) {
	case 0: {
		_packet_type = EPacketType::AttackCapture;
		_sign = upThreat ? ESignature::RootKit_2 : ESignature::RootKit_1;
		break;
	}
	case 1: {
		_packet_type = EPacketType::AttackCrash;
		_sign = upThreat ? ESignature::Crash_2 : ESignature::Crash_1;
		break;
	}
	case 2: {
		_packet_type = EPacketType::AttackSpy;
		_sign = upThreat ? ESignature::Spy_2 : ESignature::Spy_1;
		break;
	}
	default: {
		_packet_type = EPacketType::Simple;
		_sign = ESignature::NotSign;
		break;
	}
	}

	APacket* packet = source_node->CreatePacket(target_id, _packet_type, spoof_id);
	if (!packet) {
		return;
	}

	int _spy_master_id = attack_type == 2 ? source_node->NodeID : -2;
	packet->sThreat = new APacket::FThreat(_sign, NodeRoots.Contains(target_id), _spy_master_id);

	source_node->SendPacket(packet);
}
void UWidget_Manager::InitInformative(int target_id, ANodeBase* source_node, int spoof_id)
{
	APacket* packet = source_node->CreatePacket(target_id, EPacketType::Informative, spoof_id);
	if (!packet) {
		return;
	}

	packet->sInformation = new APacket::FInformation(false, 0, {}, { source_node->NodeID });
	if (source_node->sInformation) {
		packet->sInformation->key_info_count += source_node->sInformation->key_info_count;
	}
	source_node->SendPacket(packet);
}
