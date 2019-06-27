// Fill out your copyright notice in the Description page of Project Settings.

#include "KartMovementComponent.h"
#include "Engine/World.h"
#include "Runtime/Engine/Classes/GameFramework/GameState.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"

// Sets default values for this component's properties
UKartMovementComponent::UKartMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UKartMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


// Called every frame
void UKartMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// ...
}

void UKartMovementComponent::MoveRight(float Value)
{
	SteeringThrow = Value;
}

void UKartMovementComponent::MoveForward(float Value)
{
	Throttle = Value;
}

void UKartMovementComponent::ApplyRotation(float DeltaTime, float SteeringThrow)
{
	float DeltaLocation = FVector::DotProduct(GetOwner()->GetActorForwardVector(), Velocity) * DeltaTime; // Dot Products gives you the proportion of how close one Vector is to another
	float DTheta = (DeltaLocation / TurningCircleRadius) * SteeringThrow;
	FQuat DeltaRotation(GetOwner()->GetActorUpVector(), DTheta);
	GetOwner()->AddActorWorldRotation(DeltaRotation);
	Velocity = DeltaRotation.RotateVector(Velocity);
}

void UKartMovementComponent::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector Translation = Velocity * 100 * DeltaTime;
	FHitResult HitResult;
	GetOwner()->AddActorWorldOffset(Velocity, true, &HitResult);
	if (HitResult.IsValidBlockingHit())
	{
		Velocity = FVector(0, 0, 0);
	}
}

FVector UKartMovementComponent::CalculateAirResistance()
{
	float AirResistanceMagnitude = -pow(Velocity.Size(), 2) * DragCoefficient;
	FVector AirResistanceVector = Velocity.GetSafeNormal() * AirResistanceMagnitude;
	return AirResistanceVector;
}

FVector UKartMovementComponent::CalculateRollingResistance()
{
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100;
	float NormalForce = Mass * AccelerationDueToGravity;
	FVector RollingResistance = -Velocity.GetSafeNormal() *NormalForce * RollingResistanceCoefficient;
	return RollingResistance;
}

void UKartMovementComponent::SimulateMove(const FKartMove& Move)
{
	FVector Force = GetOwner()->GetActorForwardVector() * Move.Throttle * MaxDrivingForce;
	Force += CalculateAirResistance();
	Force += CalculateRollingResistance();
	FVector Acceleration = Force / Mass;
	Velocity = Velocity + Acceleration * Move.DeltaTime;

	ApplyRotation(Move.DeltaTime, Move.SteeringThrow);
	UpdateLocationFromVelocity(Move.DeltaTime);
}

FKartMove UKartMovementComponent::CreateMove(float DeltaTime)
{
	FKartMove Move;
	Move.DeltaTime = DeltaTime;
	Move.SteeringThrow = SteeringThrow;
	Move.Throttle = Throttle;
	Move.TimeStamp = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	return Move;
}