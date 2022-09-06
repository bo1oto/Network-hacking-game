#include "NodeBase.h"

#include <functional>
#include <algorithm>

#include "Uncrushable/Widget_Manager.h"


bool ANodeBase::bIsAlarm = false;
EPolitic ANodeBase::ePolitic = EPolitic::NotForbidden;
int ANodeBase::sameSignChance = 85;
int ANodeBase::upSignChance = 10;
int ANodeBase::behaviorChance = 15;
int ANodeBase::healChance = 60;
int ANodeBase::killChance = 80;
TSubclassOf<APacket> ANodeBase::packetTemp = nullptr;


ANodeBase::ANodeBase()
{
	PrimaryActorTick.bCanEverTick = true;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	RootComponent = Mesh;
}

void ANodeBase::BeginPlay()
{
	Super::BeginPlay();
	eNodeState = ENodeState::Working;
	sProtection = new FProtection();
	sProtection->threatSigns = { ESignature::Crash_1, ESignature::RootKit_1, ESignature::Spy_1 };
	ANodeBase::ePolitic = EPolitic::NotForbidden;
}


void ANodeBase::FillPacketTemp(TSubclassOf<APacket> temp)
{
	packetTemp = temp;
}

void ANodeBase::AddLink(ALink* _link, ANodeBase* sourceNode, ANodeBase* targetNode)
{
	if (sourceNode->eNodeType == ENodeType::TechnicalInfrastructure) {
		if (targetNode->eNodeType != ENodeType::TechnicalInfrastructure) {
			targetNode->VLAN = sourceNode->VLAN;
		}
	}
	else {
		if (targetNode->eNodeType != ENodeType::TechnicalInfrastructure) {
			sourceNode->VLAN == 0 ? sourceNode->VLAN = targetNode->VLAN : targetNode->VLAN = sourceNode->VLAN;
		}
		else {
			sourceNode->VLAN = targetNode->VLAN;
		}
	}

	sourceNode->nodeLinks.push_back(new FNodeLink{ _link, targetNode });
	targetNode->nodeLinks.push_back(new FNodeLink{ _link, sourceNode });
}


void ANodeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

int ANodeBase::FindRouter(int _vlan, int counter) const
{
	for (const auto& node : nodeLinks) {
		if (node->link->bIsAlive && node->targetNode->VLAN == _vlan) {
			if (node->targetNode->eNodeType == ENodeType::TechnicalInfrastructure) {
				return node->targetNode->NodeID;
			}
			else if (counter > 20) {// avoid deathloops
				return -1;
			}
			else {
				return node->targetNode->FindRouter(_vlan, (counter + 1));
			}
		}
	}
	return -1;
}

APacket* ANodeBase::CreatePacket(int target_id, EPacketType packet_type, int spoof_source)
{
	std::stack<AActor*> nodes_stack{};
	DeterminePath(target_id, nodes_stack);
	if (nodes_stack.empty()) {
		return nullptr;
	}
	APacket* packet = GetWorld()->SpawnActor<APacket>(packetTemp, this->GetActorLocation(), FRotator(0, 0, 0), FActorSpawnParameters());
	packet->InitPacket(packet_type, spoof_source != -2 ? spoof_source : NodeID, target_id, nodes_stack);
	return packet;
}

ANodeBase* ANodeBase::CheckNeighbour(int NodeId) const
{
	for (const auto& nodeLink : nodeLinks) {
		if (nodeLink->targetNode->NodeID == NodeId) {
			return nodeLink->targetNode;
		}
	}
	return nullptr;
}
ANodeBase* ANodeBase::CheckNeighbour(ENodeType _nodeType) const
{
	for (const auto& nodeLink : nodeLinks) {
		if (nodeLink->targetNode->eNodeType == _nodeType) {
			return nodeLink->targetNode;
		}
	}
	return nullptr;
}
void ANodeBase::DeterminePath(int target_id, std::stack<AActor*>& path_out)
{
	//Check neighbour, if they is not target we need to find router (NodeType::TechnicalInfrastructure)
	if (eNodeType == ENodeType::TechnicalInfrastructure || this->NodeID == target_id) {
		path_out.push(this);
		return;
	}
	ANodeBase* neighbour = nullptr;
	switch (target_id) {
	default: {
		neighbour = CheckNeighbour(target_id);
		break;
	}
	case -1: {
		neighbour = CheckNeighbour(ENodeType::Security);
		break;
	}
	case -2: {
		return;
	}
	}
	if (neighbour) {
		path_out.push(neighbour);
		path_out.push(this);
		return;
	}

	std::vector<ANodeBase*> bolv;
	int router_id = FindRouter(VLAN);
	if (router_id != -1) {
		ComputeNodePath(this, router_id, path_out);
	}
}
void ANodeBase::ComputeNodePath(const ANodeBase* const sender, int _id, std::stack<AActor*>& path_out, int counter)
{
	if (this->NodeID == _id) {
		path_out.push(this);
		return;
	}
	for (const auto& node : nodeLinks) {
		if (node->link->bIsAlive && node->targetNode != sender) {
			if (counter > 50) {// avoid deathloops
				return;
			}
			node->targetNode->ComputeNodePath(this, _id, path_out, counter + 1);
			if (!path_out.empty()) {
				path_out.push(this);
				return;
			}
		}
	}
}

//Physical dispatch
void ANodeBase::SendPacket(APacket* packet)
{
	if (eNodeState == ENodeState::Offline || eNodeState == ENodeState::Overloaded) {
		packet->Destroy();
		return;
	}
	if (packet->path.size() == 1) {
		AcceptPacket(packet);
		return;
	}

	ALink* link = nullptr;
	AActor*& start_point = packet->path.top();
	packet->path.pop();
	AActor*& end_point = packet->path.top();

	for (const auto& nodeLink : static_cast<ANodeBase*>(start_point)->nodeLinks) {
		if (nodeLink->targetNode == end_point) {
			link = nodeLink->link;
			if ((!start_point->IsHidden() || !end_point->IsHidden()) && !link->IsHidden()) {
				packet->SetActorHiddenInGame(false);
			}
			else {
				packet->SetActorHiddenInGame(true);
			}
			break;
		}
	}

	// Packet will compute path and start moving
	packet->InitPacketMove(start_point, end_point, link, std::bind(&ANodeBase::CheckPacket, static_cast<ANodeBase*>(end_point), std::placeholders::_1));
}

void ANodeBase::SendAlarmPacket()
{
	if (eNodeType == ENodeType::Security) {
		APacket* packet = CreatePacket(this->NodeID, EPacketType::Helpful);
		packet->sHelper = new APacket::FHelper(APacket::FHelper::EHelpState::Alarm, true);
		packet->path.pop();
		AcceptPacket(packet);
	}
	APacket* packet = CreatePacket(-1, EPacketType::Helpful);
	if (!packet) {
		return;
	}

	packet->sHelper = new APacket::FHelper(APacket::FHelper::EHelpState::Alarm, true);

	SendPacket(packet);
}

void ANodeBase::AddWorkload(int quantity)
{
	workload += quantity;
	if (workload > 100) {
		eNodeState = ENodeState::Overloaded;
		FTimerHandle full_unload_timer = FTimerHandle();
		GetWorldTimerManager().SetTimer(full_unload_timer, [this]() -> void {
			eNodeState = ENodeState::Working;
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::FromInt(NodeID) + " unloaded");
		}, 
		1.0f, false, 20.0f);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "I'm (" + FString::FromInt(this->NodeID) + ") dead");
	}
}
void ANodeBase::AddTemporaryWorkload(int _add_work, float delay_time)
{
	AddWorkload(_add_work);
	FTimerHandle unloading_timer = FTimerHandle();
	GetWorldTimerManager().SetTimer(unloading_timer, [_add_work, this] () -> void {
		AddWorkload(-_add_work);
	},
	1.0f, false, delay_time);
}

//Intermediate check (all nodes on the packet path)
void ANodeBase::CheckPacket(APacket* packet)
{
	if (eNodeState == ENodeState::Offline || eNodeState == ENodeState::Overloaded) {
		packet->Destroy();
		return;
	}

	int add_work = 0;
	float delay_time = 0.0f;

	if (sProtection && sProtection->bIsOn) {
		/*
		* 0 - Deny
		* 1 - Allow
		* 2 - Additional verification
		*/
		switch (sProtection->SourceTargetCheck(*packet)) {
		case 0: {
			packet->Destroy();
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Bad politic");
			return;
		}
		case 1: {
			delay_time += 0.2f;
			add_work += 2;
			break;
		}
		case 2: {
			delay_time += 1.0f;
			add_work += 5;
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Add_check");
			//Additional verification...
			break;
		}
		default: {
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Something wrong in politic check");
			break;
		}
		}
	}

	if (sSpyInfo) {
		add_work += 5;
		delay_time += 1.0f;
		if (packet->packetType == EPacketType::Informative) {
			if (packet->sInformation->key_info_count != 0) {
				sSpyInfo->stolen_key_info += packet->sInformation->key_info_count;
			}
			if (!packet->sInformation->roots_for_id.empty()) {
				sSpyInfo->stolen_roots.insert(sSpyInfo->stolen_roots.end(),
					packet->sInformation->roots_for_id.begin(), packet->sInformation->roots_for_id.end());
			}
		}
	}
	if (packet->path.size() <= 1) {
		AddTemporaryWorkload(add_work, delay_time);
		FTimerHandle timer = FTimerHandle();
		GetWorldTimerManager().SetTimer(timer, [packet]() -> void {
			static_cast<ANodeBase*>(packet->path.top())->SendPacket(packet);
		}, 
		1.0f, false, delay_time);
	}
	else {
		packet->path.pop();
		AcceptPacket(packet);
	}
}

void ANodeBase::AcceptPacket(APacket* packet)
{
	int add_work = 0;
	float processing_time = 0.0f;

	if (sProtection && sProtection->bIsOn) {
		add_work += 5;
		processing_time += 0.5f;
		if (sProtection->SignatureCheck(*packet)) {
			add_work += 3;
			processing_time += 0.5f;
			SendAlarmPacket();
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Detected sign!");

			AddTemporaryWorkload(add_work, processing_time);
			packet->Destroy();
			return;
		}
	}

	switch (packet->packetType) {
	case EPacketType::AttackSpam: {
		//greatly increases the load
		add_work += 100;
		processing_time += 3.0f;
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Spamed!");
		break;
	} 
	case EPacketType::Simple:
	case EPacketType::Informative: {
		//just a packet, just a load
		add_work += 4;
		processing_time += 1.5f;
		break;
	} 
	case EPacketType::AttackSpy: {
		//infects the node with a spy who collects information and sends it to a hacker
		sSpyInfo = new FSpyInfo(packet->sThreat->spy_master_id);
		AddWorkload(5);
		GetWorldTimerManager().SetTimer(sSpyInfo->spyTimer, [this] () -> void {
			if (sProtection && sProtection->bIsOn && sProtection->behaviorAnalizer && (rand() % 101 < behaviorChance)) {
				GetWorldTimerManager().ClearTimer(sSpyInfo->spyTimer);
				delete sSpyInfo;
				AddWorkload(-5);
				SendAlarmPacket();
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Spy detected!");
				return;
			}

			if (sInformation) {
				sSpyInfo->stolen_key_info += sInformation->key_info_count;
				sInformation->key_info_count = 0;
				sSpyInfo->stolen_roots.insert(sSpyInfo->stolen_roots.end(), sInformation->node_roots.begin(),sInformation->node_roots.end());
				sSpyInfo->stolen_node_info.insert(sSpyInfo->stolen_node_info.end(), sInformation->known_ids.begin(), sInformation->known_ids.end());
				sSpyInfo->stolen_node_info.push_back(NodeID);
			}

			APacket* packet = CreatePacket(sSpyInfo->spy_id, EPacketType::Simple);
			if (!packet) {
				return;
			}

			//можно добавить спуф id из известных узлов
			packet->sInformation = new APacket::FInformation(false, sSpyInfo->stolen_key_info, {}, sSpyInfo->stolen_node_info);
			sSpyInfo->stolen_key_info = 0;
			SendPacket(packet);

		}, 
		20.0f, true, 20.0f);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Spy attack succesful!");
		break;
	} 
	case EPacketType::AttackCapture: {
		//puts the node under the control of a hacker, instantly with roots and after many time without
		if (packet->sThreat->have_root) {
			add_work += 10;
			processing_time += 2.0f;
			eNodeState = ENodeState::Captured;
			if (sProtection) {
				sProtection->bIsOn = false;
			}
			/* ƒолжно быть так, тип он должен сообщить о захвате, но пока пусть так, мб это какое-то PtP
			APacket* packet = CreatePacket(sSpyInfo->spy_id, EPacketType::Simple);

			if (!packet) {
				return;
			}

			packet->sInformation = new APacket::FInformation(false, 0, {}, { id });
			SendPacket(packet);*/

			UWidget_Manager::self_ref->AddNodeInfo(NodeID, false);
			UWidget_Manager::self_ref->NodeRoots.Remove(this->NodeID);
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Captured by roots!");
			break;
		}

		AddWorkload(15);
		float brutforse_time;
		int* counter = new int(0);
		FTimerHandle* brutforseTimer = new FTimerHandle();

		switch (eNodeType) {
		case ENodeType::PersonalComputer:			brutforse_time = 100.0f; break;
		case ENodeType::TechnicalInfrastructure:	brutforse_time = 1000.0f; break;
		case ENodeType::DataStorage:				brutforse_time = 1000.0f; break;
		case ENodeType::Security:					brutforse_time = 2000.0f; break;
		default:									brutforse_time = 0.0f; break;
		}

		GetWorldTimerManager().SetTimer(*brutforseTimer, [this, brutforseTimer, counter, brutforse_time] () -> void {
			(*counter)++;
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Left until the capture: " + FString::FromInt(brutforse_time / 10 * (*counter)));
			if (*counter < 10) {
				if (sProtection && rand() % 101 > 50) {
					GetWorldTimerManager().ClearTimer(*brutforseTimer);
					delete counter;
					delete brutforseTimer;
					AddWorkload(-15);
					SendAlarmPacket();
				}
			}
			else {
				delete counter;
				GetWorldTimerManager().ClearTimer(*brutforseTimer);
				delete brutforseTimer;
				AddWorkload(-15);
				AddTemporaryWorkload(10, 2.0f);
				eNodeState = ENodeState::Captured;
				if (sProtection) {
					sProtection->bIsOn = false;
				}
				/* јналогично захвату через руты
				APacket* packet = CreatePacket(sSpyInfo->spy_id, EPacketType::Simple);

				if (!packet) {
					return;
				}

				packet->sInformation = new APacket::FInformation(false, 0, {}, { id });
				SendPacket(packet);*/
				UWidget_Manager::self_ref->AddNodeInfo(NodeID, false);
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Captured!");
			}
		}, 
		brutforse_time / 10.0f, true, 0.0f);
		break;
	} 
	case EPacketType::AttackCrash: {
		//Shuts down the node completely
		eNodeState = ENodeState::Offline;
		FTimerHandle timer = FTimerHandle();
		GetWorldTimerManager().SetTimer(timer, [this]() -> void {
			eNodeState = ENodeState::Working;
		}, 
		1.0f, false, 30.0f);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Crushed!");
		break;
	} 
	case EPacketType::Helpful: {
		//Heal or kill node
		add_work += 5;
		processing_time += 3.0f;
		bool operationState = false;
		if (eNodeState == ENodeState::Captured || sSpyInfo) {
			switch (packet->sHelper->eHelpState) {
			case APacket::FHelper::EHelpState::Killer: {
				if (rand() % 101 < killChance) {
					add_work += 10;
					processing_time += 5;
					operationState = true;
					if (sSpyInfo) {
						delete sSpyInfo;
						AddWorkload(-5);
					}
					eNodeState = ENodeState::Offline;
				}
				break;
			} 
			case APacket::FHelper::EHelpState::Healer: {
				if (rand() % 101 < healChance) {
					add_work += 30;
					processing_time += 10;
					operationState = true;
					if (sSpyInfo) {
						delete sSpyInfo;
						AddWorkload(-5);
					}
					eNodeState = ENodeState::Working;
				}
				break;
			} 
			default: {
				break;
			}
			}
		}
		else {
			operationState = true;
		}

		APacket* response_packet = CreatePacket(packet->source_id, EPacketType::Helpful);
		if (!packet) {
			break;
		}

		response_packet->sHelper = new APacket::FHelper(operationState ? APacket::FHelper::EHelpState::SuccessReport : APacket::FHelper::EHelpState::DefeatReport, false);

		SendPacket(response_packet);
		break;
	} 
	default: {
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Can't accept this packet!");
		break;
	}
	}

	AddTemporaryWorkload(add_work, processing_time);
	packet->Destroy();
}

FString ANodeBase::GetInfo() const
{
	FString str = TEXT("id: ") + FString::FromInt(NodeID) + "\n" + TEXT("VLAN: ") + FString::FromInt(VLAN) + "\n" + TEXT("workload: ") + FString::FromInt(workload);
	return str;
}
FText ANodeBase::GetTypeInfo() const
{
	switch (eNodeType) {
	case ENodeType::ExternalOutput:				return FText::FromString(TEXT("EO"));
	case ENodeType::TechnicalInfrastructure:	return FText::FromString(TEXT("TI"));
	case ENodeType::Security:					return FText::FromString(TEXT("SC"));
	case ENodeType::PersonalComputer:			return FText::FromString(TEXT("PC"));
	case ENodeType::DataStorage:				return FText::FromString(TEXT("DS"));
	default:									return FText::FromString(TEXT("WUT!?"));
	}
}
FString ANodeBase::GetStateInfo() const
{
	switch (eNodeState) {
	case ENodeState::Working:		return TEXT("Working");
	case ENodeState::Captured:		return TEXT("Captured");
	case ENodeState::Overloaded:	return TEXT("Overloaded");
	case ENodeState::Offline:		return TEXT("Offline");
	default:						return TEXT("WUT!?");
	}
}

TArray<FText> ANodeBase::GetKeyParameters() const
{
	// node type, id + VLAN, workload, state
	TArray<FText> parameters {
		FText::FromString(TEXT("Node") + GetTypeInfo().ToString()),
		FText::FromString(TEXT("ID: ") + FString::FromInt(NodeID) + TEXT(" (") + FString::FromInt(VLAN) + TEXT(")")),
		FText::FromString(TEXT("Workload: ") + FString::FromInt(workload)),
		FText::FromString(GetStateInfo())
	};
	return parameters;
}

void ANodeBase::SetVLAN(int num)
{
	VLAN = num;
}

bool ANodeBase::ContainInfo()
{
	return sInformation != nullptr;
}

void ANodeBase::AddInformation(ANodeBase* NodePtr)
{
	if (!sInformation) {
		sInformation = new FInformation();
		sInformation->known_ids = { NodePtr->NodeID };
	}
	else if (sInformation->known_ids.empty()) {
		sInformation->known_ids = { NodePtr->NodeID };
	}
	else if (std::find(sInformation->known_ids.begin(), sInformation->known_ids.end(), NodePtr->NodeID)
		== sInformation->known_ids.end()) {
		sInformation->known_ids.push_back(NodePtr->NodeID);
	}
}
void ANodeBase::AddKeyInfo()
{
	if (!sInformation) {
		sInformation = new FInformation();
	}
	sInformation->key_info_count = 2;
}
void ANodeBase::AddRoots(int root_id)
{
	if (!sInformation) {
		sInformation = new FInformation();
		sInformation->node_roots.push_back(root_id);
	}
	else if (sInformation->node_roots.empty()) {
		sInformation->node_roots.push_back(root_id);
	}
	else if (std::find(sInformation->node_roots.begin(), sInformation->node_roots.end(), root_id)
		== sInformation->node_roots.end()) {
		sInformation->node_roots.push_back(root_id);
	}
}


ANodeBase::FSpyInfo::FSpyInfo(int _spy_id)
	: spyTimer(FTimerHandle()),
	stolen_key_info(0),
	stolen_roots(std::vector<int>()),
	stolen_node_info(std::vector<int>()),
	spy_id(_spy_id)
{
}

bool ANodeBase::FProtection::SignatureCheck(const APacket& packet) const
{
	auto signCheck = [](const ESignature sign_1, const ESignature sign_2) -> bool {
		if (sign_1 == sign_2 && (rand() % 101 < sameSignChance)) {
			return true;
		}
		switch (sign_1) {
			case ESignature::Crash_1: {
				if (sign_2 == ESignature::Crash_2 && (rand() % (101) < upSignChance)) {
					return true;
				}
				break;
			}
			case ESignature::RootKit_1: {
				if (sign_2 == ESignature::RootKit_2 && (rand() % (101) < upSignChance)) {
					return true;
				}
				break;
			}
			case ESignature::Spy_1: {
				if (sign_2 == ESignature::Spy_2 && (rand() % (101) < upSignChance)) {
					return true;
				}
				break;
			}
			default: {
				return false;
			}
		}
		return false;
	};

	if (packet.sThreat) {
		for (const auto& _sign : threatSigns) {
			if (signCheck(_sign, packet.sThreat->sign)) {
				return true;
			}
		}
	}
	return false;
}

int ANodeBase::FProtection::SourceTargetCheck(const APacket& packet) const
{
	/*
	* 0 - Deny
	* 1 - Allow
	* 2 - Additional verification 
	*/
	int s_id = packet.source_id, 
		t_id = packet.target_id;

	if (s_id < 5) {//If source is EO
		if (ePolitic == EPolitic::OnlyAllowed || t_id < 70) {//or target is not PC
			return 0;
		}
		else {
			return 1;
		}
	}
	if (t_id < 5) {//If target is EO
		if (ePolitic == EPolitic::OnlyAllowed || s_id >= 70) {//or source is PC
			return 0;
		}
		else {
			return 1;
		}
	}
	return 1;
}
