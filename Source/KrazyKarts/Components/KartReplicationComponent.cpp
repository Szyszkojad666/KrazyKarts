// Fill out your copyright notice in the Description page of Project Settings.

#include "KartReplicationComponent.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "UnrealNetwork.h"
#include "KrazyKarts.h"
#include "Engine/GameEngine.h"


// Sets default values for this component's properties
UKartReplicationComponent::UKartReplicationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	bReplicates = true;

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
			ClientTick(DeltaTime);
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
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Im running on rep"));
	switch (GetOwnerRole())
	{
	case ROLE_AutonomousProxy:
		AutonomousProxy_OnRep_ServerState();
		break;
	case ROLE_SimulatedProxy:
		SimuluatedProxy_OnRep_ServerState();
		break;
	default:
		break;
	}
}

void UKartReplicationComponent::SimuluatedProxy_OnRep_ServerState()
{
	TimeBetweenUpdates = TimeSinceUpdate;
	TimeSinceUpdate = 0;
	StartingLocation = GetOwner()->GetActorLocation();
}

void UKartReplicationComponent::AutonomousProxy_OnRep_ServerState()
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
		/*
		UE_LOG(LogTemp, Warning, TEXT("Owner role is %s: "), *GETENUMSTRING("ENetRole", GetOwnerRole()));
		UE_LOG(LogTemp, Warning, TEXT("Owner role is %s: "), *GETENUMSTRING("ENetRole", GetOwner()->GetRemoteRole()));
		UE_LOG(LogTemp, Warning, TEXT("MyName is: %s"), *GetOwner()->GetName())
		*/

		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Im running"));
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
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Im running on update server state"));
	if (!KartMovementComponent) return;
	ServerState.LastMove = InMove;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = KartMovementComponent->GetVelocity();
	//UE_LOG(LogTemp, Warning, TEXT("Owner role is %s: "), *GETENUMSTRING("ENetRole", GetOwnerRole()));
}

void UKartReplicationComponent::ClientTick(float DeltaTime)
{
	TimeSinceUpdate += DeltaTime;
	if (TimeBetweenUpdates < KINDA_SMALL_NUMBER) // comparing floats to 0 is bad
	{
		return;
	}
	TargetLocation = ServerState.Transform.GetLocation();
	float LerpRatio = TimeSinceUpdate / TimeBetweenUpdates;
	LerpRatio = FMath::Clamp(LerpRatio, 0.0f, 1.0f);
	FVector NewLocation = FMath::LerpStable(StartingLocation, TargetLocation, LerpRatio);
	GetOwner()->SetActorLocation(NewLocation);
}
