// Fill out your copyright notice in the Description page of Project Settings.

#include "Kart.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"
#include "Runtime/Engine/Classes/Components/BoxComponent.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

// Sets default values
AKart::AKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	RootComponent = BoxCollision;
	Mesh->SetupAttachment(BoxCollision);
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = true; // Camera does not rotate relative to arm

}

// Called when the game starts or when spawned
void AKart::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector Force = GetActorForwardVector() * Throttle * MaxDrivingForce;
	Force += CalculateAirResistance();
	Force += CalculateRollingResistance();
	FVector Acceleration = Force / Mass;
	Velocity = Velocity + Acceleration * DeltaTime;
	

	ApplyRotation(DeltaTime);
	UpdateLocationFromVelocity(DeltaTime);
}

void AKart::ApplyRotation(float DeltaTime)
{
	float DeltaLocation = FVector::DotProduct (GetActorForwardVector(), Velocity) * DeltaTime; // Dot Products gives you the proportion of how close one Vector is to another
	float DTheta = (DeltaLocation / TurningCircleRadius) * SteeringThrow;
	FQuat DeltaRotation(GetActorUpVector(), DTheta);
	AddActorWorldRotation(DeltaRotation);
	Velocity = DeltaRotation.RotateVector(Velocity);
}

void AKart::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector Translation = Velocity * 100 * DeltaTime;
	FHitResult HitResult;
	AddActorWorldOffset(Velocity, true, &HitResult);
	if (HitResult.IsValidBlockingHit())
	{
		Velocity = FVector(0, 0, 0);
	}
}

FVector AKart::CalculateAirResistance()
{
	float AirResistanceMagnitude = -pow(Velocity.Size(), 2) * DragCoefficient;
	FVector AirResistanceVector = Velocity.GetSafeNormal() * AirResistanceMagnitude;
	return AirResistanceVector;
}

FVector AKart::CalculateRollingResistance()
{
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100;
	float NormalForce =  Mass * AccelerationDueToGravity;
	FVector RollingResistance = -Velocity.GetSafeNormal() *NormalForce * RollingResistanceCoefficient;
	return RollingResistance;
}

// Called to bind functionality to input
void AKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AKart::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AKart::MoveRight);
}

void AKart::MoveForward(float Value)
{
	Throttle = Value;
}

void AKart::MoveRight(float Value)
{
	SteeringThrow = Value;
}


