
// Fill out your copyright notice in the Description page of Project Settings.

#include "Kart.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"
#include "Runtime/Engine/Classes/Components/BoxComponent.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "KrazyKarts/Components/KartMovementComponent.h"
#include "KrazyKarts/Components/KartReplicationComponent.h"

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

	KartMovementComponent = CreateDefaultSubobject<UKartMovementComponent>(TEXT("KartMovementComponent"));
	KartReplicationComponent = CreateDefaultSubobject<UKartReplicationComponent>(TEXT("KartReplicationComponent"));
	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = true; // Camera does not rotate relative to arm

	bReplicates = true;
	bReplicateMovement = false;
}

// Called when the game starts or when spawned
void AKart::BeginPlay()
{
	Super::BeginPlay();
	if (Role == ROLE_Authority)
	{
		NetUpdateFrequency = 1;
	}
}

// Called to bind functionality to input
void AKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AKart::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AKart::MoveRight);
}

void AKart::MoveRight(float Value)
{
	KartMovementComponent->MoveRight(Value);
}

void AKart::MoveForward(float Value)
{
	KartMovementComponent->MoveForward(Value);
}





