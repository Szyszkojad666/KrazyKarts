// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Kart.generated.h"

class UBoxComponent;

UCLASS()
class KRAZYKARTS_API AKart : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AKart();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ApplyRotation(float DeltaTime);

	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

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

	UPROPERTY(ReplicatedUsing = OnRep_ActorTransform)
	FTransform ActorTransform;

	UPROPERTY(Replicated)
	FVector Velocity;
	
	UPROPERTY(Replicated)
	float SteeringThrow;
	
	UPROPERTY(Replicated)
	float Throttle;

	UFUNCTION()
	void OnRep_ActorTransform();

	UFUNCTION()
	void OnRep_Velocity();

private:

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MoveForward(float Value);

	void MoveForward(float Value);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MoveRight(float Value);

	void MoveRight(float Value);

	

	void UpdateLocationFromVelocity(float DeltaTime);
	FVector CalculateAirResistance();
	FVector CalculateRollingResistance();
};
