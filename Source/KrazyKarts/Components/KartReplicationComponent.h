// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KrazyKarts/Components/KartMovementComponent.h"
#include "KartReplicationComponent.generated.h"

USTRUCT()
struct FKartState
{
	GENERATED_BODY()

	UPROPERTY()
	FKartMove LastMove;

	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FTransform Transform;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UKartReplicationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UKartReplicationComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


private:
	class UKartMovementComponent* KartMovementComponent;
	TArray<FKartMove> UnacknowledgedMoves;

	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FKartState ServerState;

	UFUNCTION()
	void OnRep_ServerState();
	
	void SimuluatedProxy_OnRep_ServerState();
	void AutonomousProxy_OnRep_ServerState();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FKartMove InMove);

	void ClearAcknowledgedMoves(FKartMove LastMove);

	void UpdateServerState(const FKartMove& InMove);

	void ClientTick(float DeltaTime);

	FVector StartingLocation;
	FVector TargetLocation;
	float TimeSinceUpdate;
	float TimeBetweenUpdates;
};
