// Fill out your copyright notice in the Description page of Project Settings.

#include "P2_Character.h"
#include "LocalMultiplayerDemo.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "LocalMultiplayerDemoPlayerState.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Runtime/Engine/Classes/Engine/LevelScriptActor.h"
#include "Runtime/Engine/Classes/Engine/TargetPoint.h"
#include "Runtime/Engine/Public/EngineUtils.h"
#include "Engine.h"
#include "UObject/ConstructorHelpers.h"

const FName AP2_Character::HorizontalAnimName("Horizontal");
const FName AP2_Character::VerticalAnimName("Vertical");
const FString AP2_Character::MyLevelName("Minimal_Default");
const FName AP2_Character::MyTagName("PlayerTwo");

// Sets default values
AP2_Character::AP2_Character(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Set Collision Component
	CollisionComp = GetCapsuleComponent();
	CollisionComp->InitCapsuleSize(42.f, 96.0f);
	CollisionComp->bHiddenInGame = false;
	CollisionComp->bGenerateOverlapEvents = true;
	CollisionComp->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);

	// Set Skeletal Mesh Component
	PlayerMesh = GetMesh();
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> ObjMesh(TEXT("/Game/AnimStarterPack/UE4_Mannequin/Mesh/HumanMale"));
	PlayerMesh->SetSkeletalMesh(ObjMesh.Object);
	PlayerMesh->SetRelativeLocation(FVector(0.f, 0.f, -95.f));
	PlayerMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	PlayerMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	// Set Skeletal Mesh Animation Blueprint
	static ConstructorHelpers::FClassFinder<UObject> AnimBPClass(TEXT("/Game/Blueprints/P2_AnimBP"));
	PlayerMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	PlayerMesh->SetAnimInstanceClass(AnimBPClass.Class);

	// Set Character Movement Component Variables
	CharacterMove = GetCharacterMovement();
	CharacterMove->JumpZVelocity = 600.f;
	CharacterMove->GravityScale = 1.f;
	CharacterMove->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	CharacterMove->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate

	// Create Camera Spring Arm (pulls in towards the player if there is a collision)
	CameraSpringArm = ObjectInitializer.CreateDefaultSubobject<USpringArmComponent>(this, TEXT("CameraSpringArm"));
	CameraSpringArm->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
	CameraSpringArm->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraSpringArm->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraSpringArm->bDoCollisionTest = false;
	CameraSpringArm->bAutoActivate = true;
	CameraSpringArm->SetupAttachment(RootComponent);

	// Create Player Camera
	PlayerCamera = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("PlayerCamera"));
	PlayerCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	PlayerCamera->bAutoActivate = true;
	PlayerCamera->SetupAttachment(CameraSpringArm, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation

	// Default Values for Variables
	respawnCountdown = 3.f;
	canDisable = false;
	canRespawn = false;
	animInstance = NULL;
	FirstRespawnInWorld = NULL;
	SecondRespawnInWorld = NULL;
	ThirdRespawnInWorld = NULL;
	FourthRespawnInWorld = NULL;
	myPlayerState = NULL;
	horizontal = 0.f;
	vertical = 0.f;
	TotalScore = 0;
	isDead = false;
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
	
	// Initialize Array
	RespawnLocation.Empty();

	// Actor Tag
	this->Tags.Add(MyTagName);

}

#pragma region Setup Logic
// Called when the game starts or when spawned
void AP2_Character::BeginPlay()
{
	Super::BeginPlay();
	FindPlayerState();
	FindRespawnLocations();

	// Animation instance that allows us to change variables in animation blueprint
	if (PlayerMesh)
		animInstance = Cast<UAnimInstance>(PlayerMesh->GetAnimInstance());

}

// Get Player State from Player Controller 1
void AP2_Character::FindPlayerState()
{
	class UWorld* const world = GetWorld();

	if (world != nullptr)
	{
		// To make sure everything has loaded correctly, we will also do a level check
		class ALevelScriptActor* LevelActorInstance = Cast<ALevelScriptActor>(world->GetLevelScriptActor());
		class APlayerController* PlConPlTwo = Cast<APlayerController>(UGameplayStatics::GetPlayerController(world, 1));

		if (PlConPlTwo && LevelActorInstance)
		{
			if (LevelActorInstance->GetName().Contains(MyLevelName))
			{
				myPlayerState = Cast<ALocalMultiplayerDemoPlayerState>(PlConPlTwo->PlayerState);
			}
		}
	}
}

// Find each ATargetPoint and add them to our array of respawn locations
void AP2_Character::FindRespawnLocations()
{
	class UWorld* const world = GetWorld();

	if (world)
	{
		for (TActorIterator<ATargetPoint> FirstItr(world); FirstItr; ++FirstItr)
		{
			class ATargetPoint* FoundPoint = *FirstItr;

			if (FoundPoint != nullptr)
			{
				if (FoundPoint->ActorHasTag(FName(TEXT("RespawnOne")))) 
				{
					if (FirstRespawnInWorld != FoundPoint)
					{
						FirstRespawnInWorld = FoundPoint;

						// Once the location is found, add it to array
						RespawnLocation.Add(FirstRespawnInWorld);
					}
				}
			}
		}

		for (TActorIterator<ATargetPoint> SecondItr(world); SecondItr; ++SecondItr)
		{
			class ATargetPoint* FoundPoint_2 = *SecondItr;

			if (FoundPoint_2 != nullptr)
			{
				if (FoundPoint_2->ActorHasTag(FName(TEXT("RespawnTwo"))))
				{
					if (SecondRespawnInWorld != FoundPoint_2)
					{
						SecondRespawnInWorld = FoundPoint_2;

						// Once the location is found, add it to array
						RespawnLocation.Add(SecondRespawnInWorld);
					}
				}
			}
		}

		for (TActorIterator<ATargetPoint> ThirdItr(world); ThirdItr; ++ThirdItr)
		{
			class ATargetPoint* FoundPoint_3 = *ThirdItr;

			if (FoundPoint_3 != nullptr)
			{
				if (FoundPoint_3->ActorHasTag(FName(TEXT("RespawnThree"))))
				{
					if (ThirdRespawnInWorld != FoundPoint_3)
					{
						ThirdRespawnInWorld = FoundPoint_3;

						// Once the location is found, add it to array
						RespawnLocation.Add(ThirdRespawnInWorld);
					}
				}
			}
		}

		for (TActorIterator<ATargetPoint> FourthItr(world); FourthItr; ++FourthItr)
		{
			class ATargetPoint* FoundPoint_4 = *FourthItr;

			if (FoundPoint_4 != nullptr)
			{
				if (FoundPoint_4->ActorHasTag(FName(TEXT("RespawnFour"))))
				{
					if (FourthRespawnInWorld != FoundPoint_4)
					{
						FourthRespawnInWorld = FoundPoint_4;

						// Once the location is found, add it to array
						RespawnLocation.Add(FourthRespawnInWorld);
					}
				}
			}
		}
	}
}
#pragma endregion

// Called every frame
void AP2_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (isDead)
	{
		// Timer to determine when to disable/respawn player
		respawnCountdown -= 1 * DeltaTime;

		if (respawnCountdown < 0.f)
		{
			if (!canRespawn)
			{
				Respawn();

				// Reset variables
				respawnCountdown = 3.f;
				canRespawn = true;
				canDisable = false;
			}
		}
		else
		{
			if (!canDisable)
			{
				DisablePlayer();
				canDisable = true;
				canRespawn = false;
			}
		}
	}
}

#pragma region Movement
// Called to bind functionality to input
void AP2_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AP2_Character::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AP2_Character::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AP2_Character::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AP2_Character::LookUpAtRate);

}

void AP2_Character::MoveForward(float v)
{
	if (!isDead)
	{
		// Variable to track vertical movement in editor
		vertical = v;

		// Animate
		RunForwardAnimation(vertical);

		if ((Controller != NULL) && (vertical != 0.0f))
		{
			// Find out which way is forward
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// Get forward vector
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

			// Add movement in that direction
			AddMovementInput(Direction, vertical);
		}
	}
}

void AP2_Character::MoveRight(float h)
{
	if (!isDead) 
	{
		// Variable to track horizontal movement in editor
		horizontal = h;

		// Animate
		RunRightAnimation(horizontal);

		if ((Controller != NULL) && (horizontal != 0.0f))
		{
			// Find out which way is right
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// Get right vector 
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			// Add movement in that direction
			AddMovementInput(Direction, horizontal);
		}
	}
}

void AP2_Character::TurnAtRate(float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AP2_Character::LookUpAtRate(float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}
#pragma endregion

#pragma region Animations
void AP2_Character::RunForwardAnimation(float amount)
{
	if (!animInstance)
		return;

	// Create pointer to the horizontal variable inside the animation blueprint
	class UFloatProperty *forwardProp = FindField<UFloatProperty>(animInstance->GetClass(), HorizontalAnimName);

	if (forwardProp != NULL)
	{
		float retVal = forwardProp->GetPropertyValue_InContainer(animInstance);

		// Set value of horizontal variable
		forwardProp->SetPropertyValue_InContainer(animInstance, amount);
		retVal = forwardProp->GetPropertyValue_InContainer(animInstance);
	}
}

void AP2_Character::RunRightAnimation(float amount)
{
	if (!animInstance)
		return;

	// Create pointer to the vertical variable inside the animation blueprint
	class UFloatProperty *rightProp = FindField<UFloatProperty>(animInstance->GetClass(), VerticalAnimName);

	if (rightProp != NULL)
	{
		float retVal = rightProp->GetPropertyValue_InContainer(animInstance);

		// Set value of vertical variable
		rightProp->SetPropertyValue_InContainer(animInstance, amount);
		retVal = rightProp->GetPropertyValue_InContainer(animInstance);
	}
}
#pragma endregion

#pragma region Respawn Logic
// Our disable method, where we disable the collision, mesh, movement, and then hide the actor
void AP2_Character::DisablePlayer()
{
	if (PlayerMesh)
	{
		this->SetActorEnableCollision(false);
		PlayerMesh->SetActive(false);
		CharacterMove->Deactivate();

		// If using splitscreen, I recommend not to hide actor with this.  Possible bug in the behavior there.
		//this->SetActorHiddenInGame(true);

		// If using a follow camera
		CameraSpringArm->SetActive(false);
		PlayerCamera->SetActive(false);

		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("YOU'RE DEAD"));

		// Reset player two score
		if (myPlayerState != NULL)
		{
			TotalScore = 0;
			myPlayerState->TotalScore_P2 = TotalScore;
		}

		// Now find respawn location and put player two there
		ChooseRandomRespawnPoint();
	}
}

void AP2_Character::ChooseRandomRespawnPoint()
{
	// Initialize Point for ATargetPoint
	class ATargetPoint* LocationToRespawnAt = NULL;

	// Find a random value between 0-3, which is the size of our array
	int32 ranVal = FMath::RandRange(0, 3);

	// Check if our array is filled with ATargetPoint
	if (RespawnLocation.Num() > 0) 
	{
		if (RespawnLocation[ranVal] != NULL)
		{
			// Set respawn location
			LocationToRespawnAt = RespawnLocation[ranVal];

			// Get location and rotation of where we are respawning
			if (LocationToRespawnAt != NULL) 
			{
				FVector RespawnPos = LocationToRespawnAt->GetActorLocation();
				FRotator RespawnRot = LocationToRespawnAt->GetActorRotation();

				// Set player two at the found random respawn point
				SetActorLocation(RespawnPos);
				SetActorRotation(RespawnRot);

				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, TEXT("ACTOR POSITION IS SET"));
			}
		}
	}
}

// Our respawn method, where we activate collision, mesh, and movement
void AP2_Character::Respawn()
{
	if (isDead)
	{
		if (PlayerMesh)
		{
			this->SetActorEnableCollision(true);
			PlayerMesh->SetActive(true);
			CharacterMove->Activate();

			// If using splitscreen, I recommend not to hide actor with this.  Possible bug in the behavior.
			//this->SetActorHiddenInGame(false);

			// If using a follow camera
			CameraSpringArm->SetActive(true);
			PlayerCamera->SetActive(true);
		}

		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("RESPAWNED"));

		// End Method
		isDead = false;
	}
}
#pragma endregion

