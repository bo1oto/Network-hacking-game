#include "Nodes.h"


int ANodeSC::id_counter = 30;

ANodeSC::ANodeSC() : ANodeBase()
{
}

void ANodeSC::BeginPlay()
{
	ANodeBase::BeginPlay();
	AddWorkload(40);
	eNodeType = ENodeType::Security;
	NodeID = id_counter;
	id_counter++;
	helpTimer = FTimerHandle();
	sProtection->behaviorAnalizer = true;
	sProtection->bSpamFilter = true;
	have_recovery_system = true;
}

void ANodeSC::Tick(float DeltaTime)
{
	ANodeBase::Tick(DeltaTime);
}

void ANodeSC::AcceptPacket(APacket* packet)
{
	if (packet->packetType == EPacketType::Helpful)
	{
		if (have_recovery_system)
		{
			int item_num = sApocalypseRescueKit->map_id_pos[packet->source_id];
			auto it = sApocalypseRescueKit->list_apocalypse_timers.begin();
			std::advance(it, item_num);
			switch (packet->sHelper->eHelpState)
			{
			case APacket::FHelper::EHelpState::SuccessReport:
			{
				GetWorldTimerManager().ClearTimer(*(*it).first);
				delete (*it).first;
				(*it).first = nullptr;
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Help for " + FString::FromInt(packet->source_id) + " successful!");

				AddTemporaryWorkload(5, 2.0f);
				packet->Destroy();
				return;
			}
			case APacket::FHelper::EHelpState::DefeatReport:
			{
				(*it).second = true;
				AddTemporaryWorkload(5, 2.0f);
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Help for " + FString::FromInt(packet->source_id) + " failed!");

				AddTemporaryWorkload(5, 2.0f);
				packet->Destroy();
				return;
			}
			default: 
				break;
			}
		}
		if (packet->sHelper->bIsRaiseAlarm)
		{
			if (!ANodeBase::bIsAlarm)
			{
				ANodeBase::bIsAlarm = true;
				ANodeBase::ePolitic = EPolitic::OnlyAllowed;
				ANodeBase::sameSignChance = 100;
				ANodeBase::upSignChance = 50;
				ANodeBase::behaviorChance = 75;
				if (have_recovery_system)
				{
					sApocalypseRescueKit = new ApocalypseRescueKit();
					SaveThisWorld();
				}
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Alarm!!!");
			}
			AddTemporaryWorkload(10, 1.0f);
			packet->Destroy();
			return;
		}
	}
	ANodeBase::AcceptPacket(packet);
}

void ANodeSC::GeneratePacket(int chance)
{
}

void ANodeSC::SaveThisWorld()
{
	auto BlessAndSave = [this](int _id)
	{
		APacket* packet = CreatePacket(_id, EPacketType::Helpful);

		if (!packet) {
			return;
		}

		packet->sHelper = new APacket::FHelper(APacket::FHelper::EHelpState::Healer, false);

		FTimerHandle* timer = new FTimerHandle();
		sApocalypseRescueKit->list_apocalypse_timers.push_back(std::make_pair(timer, false));
		sApocalypseRescueKit->map_id_pos.insert({ _id, sApocalypseRescueKit->list_size });

		//The timer starts, if there is no response, then the killers start spamming
		GetWorldTimerManager().SetTimer(*timer, [this, _id = _id, vec_num = sApocalypseRescueKit->list_size, path = packet->path]
		{
			bool no_response_flag = true;
			//Так как путь статичен, я копирую в стэк вектор узлов, а уже из него создаю копию в куче и указатель на неё
			auto it = sApocalypseRescueKit->list_apocalypse_timers.begin();
			std::advance(it, vec_num);
			if ((*it).second)
			{
				APacket* _packet = CreatePacket(_id, EPacketType::Helpful);

				if (!_packet) {
					return;
				}

				_packet->sHelper = new APacket::FHelper(APacket::FHelper::EHelpState::Killer, false);

				SendPacket(_packet);
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Killer for " + FString::FromInt(_id) + " coming!");
			}
			else if (no_response_flag)
			{
				no_response_flag = false;
				FTimerHandle timer = FTimerHandle();
				GetWorldTimerManager().SetTimer(timer, [it]
				{
					(*it).second = true;
				}, 1.0f, false, 30.0f);
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Not yet for " + FString::FromInt(_id));
			}

		}, 15.0f, true, 15.0f);

		SendPacket(packet);

		sApocalypseRescueKit->list_size++;
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Healer for " + FString::FromInt(_id) + " coming!");
	};
	for (int i = 10; i < ANodeTI::id_counter; ++i)
	{
		BlessAndSave(i);
	}
	for (int i = 50; i < ANodeDS::id_counter; ++i)
	{
		BlessAndSave(i);
	}
	for (int i = 70; i < ANodePC::id_counter; ++i)
	{
		BlessAndSave(i);
	}
	for (int i = 30; i < ANodeSC::id_counter; ++i)
	{
		if (i != NodeID) BlessAndSave(i);
	}
}
