// Fill out your copyright notice in the Description page of Project Settings.

#include "KartReplicationComponent.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "UnrealNetwork.h"
#include "KrazyKarts.h"
#include "Engine/World.h"
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
	MeshOffsetRoot = GetOwner()->FindComponentByClass<USceneComponent>();
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
	if (!KartMovementComponent) return;
	TimeBetweenUpdates = TimeSinceUpdate;
	TimeSinceUpdate = 0;
	StartingVelocity = KartMovementComponent->GetVelocity();

	if (!MeshOffsetRoot) return;
	StartingTransform.SetLocation(MeshOffsetRoot->GetComponentLocation());
	StartingTransform.SetRotation(MeshOffsetRoot->GetComponentQuat());
	
	GetOwner()->SetActorTransform(ServerState.Transform);
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
		ClientSimulatedTime += InMove.DeltaTime;
	}
}

bool  UKartReplicationComponent::Server_SendMove_Validate(FKartMove InMove)
{
	float ProposedTime = ClientSimulatedTime + InMove.DeltaTime;
	bool TimeCheating = ProposedTime > GetWorld()->TimeSeconds;
	if (TimeCheating) return false;
	return InMove.IsValid();
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
	//UE_LOG(LogTemp, Warning, TEXT("Owner role is %s: "), *GETENUMSTRING("ENetRole", GetOwnerRole()));
}

void UKartReplicationComponent::ClientTick(float DeltaTime)
{
	TimeSinceUpdate += DeltaTime;
	if (TimeBetweenUpdates < KINDA_SMALL_NUMBER || !KartMovementComponent ) // comparing floats to 0 is bad
	{
		return;
	}
	
	FHermiteCubicSpline Spline = CreateSpline();
	
	float LerpRatio = TimeSinceUpdate / TimeBetweenUpdates;
	LerpRatio = FMath::Clamp(LerpRatio, 0.0f, 1.0f);
	
	InterpolateLocation(Spline, LerpRatio);

	InterpolateVelocity(Spline, LerpRatio);
	
	InterpolateRotation(LerpRatio);
}

void UKartReplicationComponent::InterpolateRotation(float LerpRatio)
{
	if (!MeshOffsetRoot) return;
	TargetRotation = ServerState.Transform.GetRotation();
	FQuat NewRotation = FQuat::Slerp(StartingTransform.GetRotation(), TargetRotation, LerpRatio);
	MeshOffsetRoot->SetWorldRotation(NewRotation);
}

void UKartReplicationComponent::InterpolateVelocity(FHermiteCubicSpline &Spline, float LerpRatio)
{
	FVector NewDerivative = Spline.InterpolateDerivative(LerpRatio);
	FVector NewVelocity = NewDerivative / VelocityToDerivative();
	KartMovementComponent->SetVelocity(NewVelocity);
}

void UKartReplicationComponent::InterpolateLocation(FHermiteCubicSpline &Spline, float LerpRatio)
{
	if (!MeshOffsetRoot) return;
	FVector NewLocation = Spline.InterpolateLocation(LerpRatio);
	MeshOffsetRoot->SetWorldLocation(NewLocation);
}

FHermiteCubicSpline UKartReplicationComponent::CreateSpline()
{
	FHermiteCubicSpline Spline;
	Spline.TargetLocation = ServerState.Transform.GetLocation();
	Spline.StartLocation = StartingTransform.GetLocation();
	Spline.StartDerivative = StartingVelocity * VelocityToDerivative();
	Spline.TargetDerivative = ServerState.Velocity * VelocityToDerivative();

	return Spline;
}
