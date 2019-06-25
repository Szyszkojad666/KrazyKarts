// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Kart.generated.h"

class UBoxComponent;

USTRUCT()
struct FKartMove
{
	GENERATED_USTRUCT_BODY()
		
	UPROPERTY()
	float SteeringThrow;

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float DeltaTime;
	
	UPROPERTY()
	float TimeStamp;
};

USTRUCT()
struct FKartState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FKartMove LastMove;

	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FTransform Transform;
};
UCLASS()
class KRAZYKARTS_API AKart : public APawn
{
	GENERATED_BODY()

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	AKart();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBoxComponent* BoxCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	UPROPERTY(EditAnywhere)
	float Mass = 1000;

	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 10000;

	UPROPERTY(EditAnywhere)
	float MaxRotation = 90;

	UPROPERTY(EditAnywhere)
	float TurningCircleRadius = 11;
	
	//How aerodynamic the car is
	UPROPERTY(EditAnywhere)
	float DragCoefficient = 16;

	UPROPERTY(EditAnywhere)
	float RollingResistanceCoefficient = 0.02;

private:
	
	FVector Velocity;

	float SteeringThrow;

	float Throttle;

	TArray<FKartMove> UnacknowledgedMoves;

	void UpdateLocationFromVelocity(float DeltaTime);
	void ApplyRotation(float DeltaTime, float SteeringThrow);
	
	FVector CalculateAirResistance();
	FVector CalculateRollingResistance();
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FKartState ServerState;

	UFUNCTION()
	void OnRep_ServerState();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FKartMove InMove);

	FKartMove CreateMove(float DeltaTime);

	void MoveRight(float Value);

	void MoveForward(float Value);

	void SimulateMove(const FKartMove& Move);

	void ClearAcknowledgedMoves(FKartMove LastMove);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};