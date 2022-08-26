
#include "NodeDS.h"

int ANodeDS::id_counter = 50;

ANodeDS::ANodeDS() : ANodeBase()
{
}

void ANodeDS::BeginPlay()
{
	ANodeBase::BeginPlay();
	AddWorkload(20);
	nodeType = NodeType::DataStorage;
	id = id_counter;
	id_counter++;
	sInformation = new Information();
	sInformation->key_info_count = 20;
}

void ANodeDS::AcceptPacket(APacket* packet)
{
	if (packet->packetType == PacketType::Informative && packet->sInformation->isDSRequest)
	{
		std::vector<ANodeBase*>* nodes = DeterminePath(packet->source_id);
		if (nodes && !(*nodes).empty())
		{
			APacket* _packet = GetWorld()->SpawnActor<APacket>(packetTemp, this->GetActorLocation(), FRotator(0, 0, 0), FActorSpawnParameters());
			_packet->InitPacket(PacketType::Informative, this->id, packet->source_id);
			_packet->sInformation->key_info_count = 1;
			SendPacket(_packet, nodes, (*nodes).begin());
		}
	}
	ANodeBase::AcceptPacket(packet);
}
