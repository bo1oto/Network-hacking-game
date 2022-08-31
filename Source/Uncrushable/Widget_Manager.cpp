
#pragma once

#include "Widget_Manager.h"

#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>


UWidget_Manager* UWidget_Manager::self_ref = nullptr;
bool UWidget_Manager::isGameStart = false;
TMap<int, ANodeBase*> UWidget_Manager::all_nodes = {};


FNodeInfo::FNodeInfo() :
	node_id(-1), node_ptr(nullptr), 
	characteristic(TEXT("Empty object")), 
	bIsFullInfo(false) {
}
FNodeInfo::FNodeInfo(int _node_id) :
	node_id(_node_id), 
	node_ptr(nullptr), 
	characteristic(TEXT("No information\n available :(")),
	bIsFullInfo(false) {
}
FNodeInfo::FNodeInfo(ANodeBase* _node_ptr) :
	node_id(_node_ptr->id), 
	node_ptr(_node_ptr), 
	characteristic(UWidget_Manager::FillNodeCharacteristic(*_node_ptr)), 
	bIsFullInfo(true) {
	node_ptr->Mesh->SetMaterial(0, node_ptr->main_mat);
}


void UWidget_Manager::SetSelfRef(UWidget_Manager* _self_ref)
{
	UWidget_Manager::self_ref = _self_ref;
}

FString UWidget_Manager::FillNodeCharacteristic(const ANodeBase& node_ptr)
{
	FString str = "Type: ";
	switch (node_ptr.eNodeType)
	{
	case ENodeType::PersonalComputer:		str += TEXT("PC"); break;
	case ENodeType::ExternalOutput:			str += TEXT("EO"); break;
	case ENodeType::TechnicalInfrastructure: str += TEXT("TI"); break;
	case ENodeType::Security:				str += TEXT("SC"); break;
	case ENodeType::DataStorage:				str += TEXT("DS"); break;
	default:								str += TEXT("???"); break;
	}
	if (node_ptr.sProtection != nullptr)
	{
		str += TEXT("\nSignatures: ");
		for (const auto& sign : node_ptr.sProtection->threatSigns)
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
		str += node_ptr.sProtection->bSpamFilter ? TEXT("\nSpam filter: Present") : TEXT("\nSpam filter: Absent");
	}
	else
	{
		str += TEXT("\nNo protection");
	}
	return str;
}

void UWidget_Manager::StartGame()
{
	TArray<AActor*> for_all_nodes = TArray<AActor*>();
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANodeBase::StaticClass(), for_all_nodes);

	TArray<AActor*> nodes_arr = TArray<AActor*>();
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANodePC::StaticClass(), nodes_arr);
	generation_timers = new std::vector<FTimerHandle>();
	for (const auto& node : nodes_arr)
	{
		FTimerHandle timer = FTimerHandle();
		node->GetWorldTimerManager().SetTimer(timer, [nodePC = (ANodePC*)node]
		{
			if (UWidget_Manager::isGameStart && nodePC->eNodeState != ENodeState::Overloaded &&
				nodePC->eNodeState != ENodeState::Offline && !nodePC->nodeLinks.empty())
			{
				nodePC->GeneratePacket(std::rand() % 101);
			}
		}, network_activity_time_tick, true, network_activity_time_tick);
		generation_timers->push_back(timer);
	}
	isGameStart = true;
}

void UWidget_Manager::AddNodeInfo(int node_id, bool bAsID)
{
	auto f = [node_id](const FNodeInfo& node_info) -> bool {
		return node_info.node_id == node_id;
	};

	ANodeBase* node_ptr = *UWidget_Manager::all_nodes.Find(node_id);
	node_ptr->SetActorHiddenInGame(false);
	//node_ptr->SetActorEnableCollision(true);

	if (bAsID) {
		if (known_nodes.ContainsByPredicate(f)) {
			return;
		}
		known_nodes.Add(FNodeInfo(node_ptr->id));
	}
	else {
		node_ptr->Mesh->SetMaterial(0, node_ptr->main_mat);
		FNodeInfo* fast_ptr = known_nodes.FindByPredicate(f);
		if (fast_ptr) {
			fast_ptr->node_ptr = node_ptr;
			fast_ptr->characteristic = UWidget_Manager::FillNodeCharacteristic(*node_ptr);
			fast_ptr->bIsFullInfo = true;
		}
		else {
			known_nodes.Add(FNodeInfo(node_ptr));
		}

		for (const auto& nodeLink : node_ptr->nodeLinks) {
			nodeLink->link->SetActorHiddenInGame(false);
			AddNodeInfo(nodeLink->targetNode->id, true);
		}
	}

}

void UWidget_Manager::AddKeyInfo(int quantity)
{
	key_info_counter += quantity;
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Key info progress: " + FString::FromInt(key_info_counter) + "/20 !");
	if (key_info_counter >= 20)
	{
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
	switch (attack_type)
	{
	case 0:
		_packet_type = EPacketType::AttackCapture;
		_sign = upThreat ? ESignature::RootKit_2 : ESignature::RootKit_1;
		break;
	case 1:
		_packet_type = EPacketType::AttackCrash;
		_sign = upThreat ? ESignature::Crash_2 : ESignature::Crash_1;
		break;
	case 2:
		_packet_type = EPacketType::AttackSpy;
		_sign = upThreat ? ESignature::Spy_2 : ESignature::Spy_1;
		break;
	default:
		_packet_type = EPacketType::Simple;
		_sign = ESignature::NotSign;
		break;
	}

	APacket* packet = source_node->CreatePacket(target_id, _packet_type, spoof_id);

	if (!packet) {
		return;
	}

	int _spy_master_id = attack_type == 2 ? source_node->id : -2;
	packet->sThreat = new APacket::FThreat(_sign, roots.Contains(target_id), _spy_master_id);

	source_node->SendPacket(packet);

}
void UWidget_Manager::InitInformative(int target_id, ANodeBase* source_node, int spoof_id)
{
	APacket* packet = source_node->CreatePacket(target_id, EPacketType::Informative, spoof_id);

	if (!packet) {
		return;
	}

	packet->sInformation = new APacket::FInformation(false, 0, {}, { source_node->id });
	if (source_node->sInformation) {
		packet->sInformation->key_info_count += source_node->sInformation->key_info_count;
	}
	source_node->SendPacket(packet);
}
