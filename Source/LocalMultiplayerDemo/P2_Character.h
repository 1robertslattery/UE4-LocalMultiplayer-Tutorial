// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "P2_Character.generated.h"

UCLASS()
class LOCALMULTIPLAYERDEMO_API AP2_Character : public ACharacter
{
	GENERATED_BODY()

private:

	// Respawn Variables
	float respawnCountdown;
	bool canDisable;
	bool canRespawn;

	// Respawn Methods
	void FindRespawnLocations();
	void DisablePlayer();
	void ChooseRandomRespawnPoint();
	void Respawn();

	// Player State Method
	void FindPlayerState();
		
public:

	// Sets default values for this character's properties
	AP2_Character(const FObjectInitializer &ObjectInitializer);

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Animation Instance Reference
	class UAnimInstance* animInstance;

	// Respawn Location References
	class ATargetPoint* FirstRespawnInWorld;
	class ATargetPoint* SecondRespawnInWorld;
	class ATargetPoint* ThirdRespawnInWorld;
	class ATargetPoint* FourthRespawnInWorld;

	// Player State for Player Controller at index 1
	class ALocalMultiplayerDemoPlayerState* myPlayerState;

public:	

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// References to Collision, Mesh, and Character Movement Components
	class UCapsuleComponent* CollisionComp;
	class USkeletalMeshComponent* PlayerMesh;
	class UCharacterMovementComponent* CharacterMove;
	
	// Spring Arm Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USpringArmComponent* CameraSpringArm;

	// Camera Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* PlayerCamera;

	// Actor Movement
	void MoveForward(float v);
	void MoveRight(float h);

	// Actor Animation
	void RunForwardAnimation(float amount);
	void RunRightAnimation(float amount);
		
	// Called via input to turn at a given rate.
	void TurnAtRate(float Rate);

	// Called via input to turn look up/down at a given rate.
	void LookUpAtRate(float Rate);

	// Character Stats Variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Stats")
	float horizontal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Stats")
	float vertical;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
	int32 TotalScore;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Stats")
	bool isDead;

	// Base turn rate, in deg/sec. Other scaling may affect final turn rate
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	float BaseTurnRate;

	// Base look up/down rate, in deg/sec. Other scaling may affect final rate
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	float BaseLookUpRate;
	
protected:

	// Animation Blueprint Variable References
	static const FName HorizontalAnimName;
	static const FName VerticalAnimName;

	// Level Name
	static const FString MyLevelName;

	// Player Tag Name
	static const FName MyTagName;

public:

	// Respawn Locations Array
	UPROPERTY(EditAnywhere, Category = "Spawn Locations")
	TArray<class ATargetPoint*> RespawnLocation;
		
public:

	// Returns CameraSpringArm Subobject
	FORCEINLINE class USpringArmComponent* GetCameraSpringArm() const { return CameraSpringArm; }

	// Returns PlayerCamera Subobject
	FORCEINLINE class UCameraComponent* GetPlayerCamera() const { return PlayerCamera; }

};
