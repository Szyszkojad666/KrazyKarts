// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KartMovementComponent.generated.h"

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


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UKartMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UKartMovementComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void MoveRight(float Value);
	void MoveForward(float Value);

	void SimulateMove(const FKartMove& Move);
	
	void SetVelocity(FVector InVelocity) { Velocity = InVelocity; }
	
	UFUNCTION(BlueprintCallable)
	const FVector GetVelocity() { return Velocity; }

	FORCEINLINE
	FKartMove GetLastMove() { return LastMove; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

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

	USceneComponent* MeshOffsetRoot;

	FKartMove CreateMove(float DeltaTime);

	FKartMove LastMove;

	FVector Velocity;

	float SteeringThrow;

	float Throttle;

	FVector CalculateAirResistance();
	FVector CalculateRollingResistance();

	void UpdateLocationFromVelocity(float DeltaTime);
	void ApplyRotation(float DeltaTime, float SteeringThrow);
};
