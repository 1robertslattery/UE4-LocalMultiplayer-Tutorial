// Fill out your copyright notice in the Description page of Project Settings.

#include "P1_Character.h"
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
#include "UObject/ConstructorHelpers.h"

const FName AP1_Character::HorizontalAnimName("Horizontal");
const FName AP1_Character::VerticalAnimName("Vertical");
const FString AP1_Character::MyLevelName("Minimal_Default");
const FName AP1_Character::MyTagName("PlayerOne");

// Sets default values
AP1_Character::AP1_Character(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
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
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> ObjMesh(TEXT("/Game/AnimStarterPack/UE4_Mannequin/Mesh/SK_Mannequin"));
	PlayerMesh->SetSkeletalMesh(ObjMesh.Object);
	PlayerMesh->SetRelativeLocation(FVector(0.f, 0.f, -95.f));
	PlayerMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	PlayerMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	// Set Skeletal Mesh Animation Blueprint
	static ConstructorHelpers::FClassFinder<UObject> AnimBPClass(TEXT("/Game/Blueprints/P1_AnimBP"));
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
	animInstance = NULL;
	myPlayerState = NULL;
	horizontal = 0.f;
	vertical = 0.f;
	TotalScore = 0;
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
	isTwoPlayerGame = false;
	
	// Actor Tag
	this->Tags.Add(MyTagName);

}

#pragma region Setup Logic
// Called when the game starts or when spawned
void AP1_Character::BeginPlay()
{
	Super::BeginPlay();
	FindPlayerState();

	// Animation instance that allows us to change variables in animation blueprint
	if (PlayerMesh)
		animInstance = Cast<UAnimInstance>(PlayerMesh->GetAnimInstance());

}

void AP1_Character::FindPlayerState()
{
	class UWorld* const world = GetWorld();

	if (world != nullptr)
	{
		class ALevelScriptActor* LevelActorInstance = Cast<ALevelScriptActor>(world->GetLevelScriptActor());
		class APlayerController* PlConOne = Cast<APlayerController>(UGameplayStatics::GetPlayerController(world, 0));

		if (PlConOne && LevelActorInstance)
		{
			if (LevelActorInstance->GetName().Contains(MyLevelName))
			{
				myPlayerState = Cast<ALocalMultiplayerDemoPlayerState>(PlConOne->PlayerState);
			}
		}
	}
}
#pragma endregion

// Called every frame
void AP1_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

#pragma region Movement
// Called to bind functionality to input
void AP1_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AP1_Character::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AP1_Character::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AP1_Character::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AP1_Character::LookUpAtRate);

}

void AP1_Character::MoveForward(float v)
{
	// Variable to track movement in editor
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

void AP1_Character::MoveRight(float h)
{
	// Variable to track movement in editor
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

void AP1_Character::TurnAtRate(float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AP1_Character::LookUpAtRate(float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}
#pragma endregion

#pragma region Animations
void AP1_Character::RunForwardAnimation(float amount)
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

void AP1_Character::RunRightAnimation(float amount)
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

