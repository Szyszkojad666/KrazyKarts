// Fill out your copyright notice in the Description page of Project Settings.

#include "KartReplicationComponent.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "UnrealNetwork.h"
#include "KrazyKarts.h"


// Sets default values for this component's properties
UKartReplicationComponent::UKartReplicationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UKartReplicationComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	KartMovementComponent = GetOwner()->FindComponentByClass<UKartMovementComponent>();
}


// Called every frame
void UKartReplicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (KartMovementComponent)
	{
		ENetRole Role = GetOwnerRole();
		FKartMove MoveToSimulate = KartMovementComponent->GetLastMove();
		if (GetOwner()->Role == ROLE_AutonomousProxy)
		{
			UnacknowledgedMoves.Add(MoveToSimulate);
			Server_SendMove(MoveToSimulate);
		}
		if (GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
		{
			UpdateServerState(MoveToSimulate);
		}
		if (GetOwnerRole() == ROLE_SimulatedProxy)
		{
			KartMovementComponent->SimulateMove(ServerState.LastMove);
		}
	}
}

void UKartReplicationComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UKartReplicationComponent, ServerState);
}

void UKartReplicationComponent::OnRep_ServerState()
{
	if (KartMovementComponent)
	{
		GetOwner()->SetActorTransform(ServerState.Transform);
		KartMovementComponent->SetVelocity(ServerState.Velocity);
		ClearAcknowledgedMoves(ServerState.LastMove);
		for (const FKartMove& Move : UnacknowledgedMoves)
		{
			KartMovementComponent->SimulateMove(Move);
		}
	}
}

void UKartReplicationComponent::Server_SendMove_Implementation(FKartMove InMove)
{
	if (KartMovementComponent)
	{
		KartMovementComponent->SimulateMove(InMove);
		UpdateServerState(InMove);
	}
}
	

bool  UKartReplicationComponent::Server_SendMove_Validate(FKartMove InMove)
{
	return true;
}

void UKartReplicationComponent::ClearAcknowledgedMoves(FKartMove LastMove)
{
	TArray<FKartMove> NewMoves;
	for (const FKartMove& Move : UnacknowledgedMoves)
	{
		if (Move.TimeStamp > LastMove.TimeStamp)
		{
			NewMoves.Add(Move);
		}
	}
	UnacknowledgedMoves = NewMoves;
}

void UKartReplicationComponent::UpdateServerState(const FKartMove& InMove)
{
	if (!KartMovementComponent) return;
	ServerState.LastMove = InMove;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = KartMovementComponent->GetVelocity();
}
