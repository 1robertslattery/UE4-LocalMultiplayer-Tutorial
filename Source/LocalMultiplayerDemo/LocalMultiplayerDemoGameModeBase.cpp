// Fill out your copyright notice in the Description page of Project Settings.

#include "LocalMultiplayerDemoGameModeBase.h"
#include "LocalMultiplayerDemo.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"
#include "LocalMultiplayerDemoHUD.h"
#include "GameFramework/PlayerState.h"
#include "LocalMultiplayerDemoPlayerState.h"
#include "Runtime/Engine/Classes/Engine/LevelScriptActor.h"
#include "Runtime/Engine/Public/EngineUtils.h"
#include "Runtime/Engine/Classes/Engine/TargetPoint.h"
#include "UObject/ConstructorHelpers.h"
#include "P1_Character.h"
#include "P2_Character.h"

const FString ALocalMultiplayerDemoGameModeBase::MyLevelName("Minimal_Default");

// Sets default values
ALocalMultiplayerDemoGameModeBase::ALocalMultiplayerDemoGameModeBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Default Spawn Points Settings
	RespawnSetup.RespawnPosition_1 = FVector(480.f, 430.f, 45.f);
	RespawnSetup.RespawnPosition_2 = FVector(-130.f, 350.f, 45.f);
	RespawnSetup.RespawnPosition_3 = FVector(-130.f, -380.f, 45.f);
	RespawnSetup.RespawnPosition_4 = FVector(450.f, -380.f, 45.f);

	// DefaultPawnClass assumes APlayerController at index 0 automatically
	DefaultPawnClass = AP1_Character::StaticClass();
	PlayerStateClass = ALocalMultiplayerDemoPlayerState::StaticClass();
	HUDClass = ALocalMultiplayerDemoHUD::StaticClass();

	// Default Variable Settings
	hasSetSecondPlayer = false;
	canFinishSetup = false;
	canSetWidget = false;
	delayWidgetSetupTimer = 0.f;
	PlayerOneInWorld = NULL;
	LevelActorInstance = NULL;
	isTwoPlayerMode = false;

}

#pragma region Setup Logic
// Called when the game starts or when spawned
void ALocalMultiplayerDemoGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// Find player one
	class UWorld* const world = GetWorld();

	if (world != nullptr)
	{
		for (TActorIterator<AP1_Character> ObstacleItr(world); ObstacleItr; ++ObstacleItr)
		{
			class AP1_Character* FoundPlayer = *ObstacleItr;

			if (FoundPlayer != nullptr)
			{
				if (PlayerOneInWorld != FoundPlayer)
				{
					PlayerOneInWorld = FoundPlayer;

					// Once player one is found, spawn our respawn locations in the world
					CreateRespawnPoints();
				}
			}
		}

		if (PlayerOneInWorld != nullptr)
		{
			// To make sure everything has loaded on BeginPlay, we will do a level check
			LevelActorInstance = Cast<ALevelScriptActor>(world->GetLevelScriptActor());

			if (LevelActorInstance != nullptr)
			{
				if (LevelActorInstance->GetName().Contains(MyLevelName))
				{
					// Set two player variable to true for GameModeBase class
					isTwoPlayerMode = true;

					// Two player game variable for player one. This should be loaded and set from a save game file.
					PlayerOneInWorld->isTwoPlayerGame = isTwoPlayerMode;
				}
			}
		}
	}
}

// Once player one is found and each respawn position has been set,
// spawn as many ATargetPoint classes as you want, giving them a unique tag, so that you can find them later in other actors
void ALocalMultiplayerDemoGameModeBase::CreateRespawnPoints()
{
	class UWorld* const world = GetWorld();

	if (world != nullptr)
	{
		FRotator StartRot = FRotator(0, 0, 0);

		FActorSpawnParameters spawnParams;
		spawnParams.Owner = this;
		spawnParams.Instigator = Instigator;

		class ATargetPoint* FirstTargetPoint = world->SpawnActor<ATargetPoint>(ATargetPoint::StaticClass(), RespawnSetup.RespawnPosition_1, StartRot, spawnParams);

		if (FirstTargetPoint != nullptr)
			FirstTargetPoint->Tags.Add(FName(TEXT("RespawnOne")));

		class ATargetPoint* SecondTargetPoint = world->SpawnActor<ATargetPoint>(ATargetPoint::StaticClass(), RespawnSetup.RespawnPosition_2, StartRot, spawnParams);

		if (SecondTargetPoint != nullptr)
			SecondTargetPoint->Tags.Add(FName(TEXT("RespawnTwo")));

		class ATargetPoint* ThirdTargetPoint = world->SpawnActor<ATargetPoint>(ATargetPoint::StaticClass(), RespawnSetup.RespawnPosition_3, StartRot, spawnParams);

		if (ThirdTargetPoint != nullptr)
			ThirdTargetPoint->Tags.Add(FName(TEXT("RespawnThree")));

		class ATargetPoint* FourthTargetPoint = world->SpawnActor<ATargetPoint>(ATargetPoint::StaticClass(), RespawnSetup.RespawnPosition_4, StartRot, spawnParams);

		if (FourthTargetPoint != nullptr)
			FourthTargetPoint->Tags.Add(FName(TEXT("RespawnFour")));
	}
}
#pragma endregion

// Called every frame
void ALocalMultiplayerDemoGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Again, to make sure everything has loaded correctly, we will do another level check
	if (LevelActorInstance != nullptr)
	{
		if (LevelActorInstance->GetName().Contains(MyLevelName))
		{
			if (isTwoPlayerMode)
			{
				SetupTwoPlayers();

				// Once players have spawned, load player UI
				LoadTwoPlayerWidget(DeltaTime);
			}
		}
	}
}

#pragma region Player/UI Logic
// Create a new APlayerController, unpossess it, spawn player two, and then have player two possess the created APlayerController
void ALocalMultiplayerDemoGameModeBase::SetupTwoPlayers()
{
	class UWorld* const world = GetWorld();

	if (world != nullptr)
	{
		if (PlayerOneInWorld != nullptr)
		{
			if (!canFinishSetup) 
			{
				// Find location of player one
				FVector PlayerOnePos = PlayerOneInWorld->GetActorLocation();
				PlayerOnePos.X = PlayerOneInWorld->GetActorLocation().X;
				PlayerOnePos.Y = PlayerOneInWorld->GetActorLocation().Y;
				PlayerOnePos.Z = PlayerOneInWorld->GetActorLocation().Z;

				// Create offset for player two spawn, so that it doesn't spawn on top of player one
				FVector PlayerTwoSpawnLocationOffset = FVector(PlayerOnePos.X, -150.f, 40.f);
				FVector FinalSpawnPos = PlayerOnePos + PlayerTwoSpawnLocationOffset;
				FRotator SpawnRot = FRotator(0.f, 0.f, 0.f);

				FActorSpawnParameters spawnParams;
				spawnParams.Owner = this;
				spawnParams.Instigator = Instigator;

				// Create new APlayerController at index 1
				class APlayerController* NewPlayerController = Cast<APlayerController>(UGameplayStatics::CreatePlayer(world, 1));

				if (NewPlayerController != nullptr)
				{
					// Another player one will spawn with the new APlayerController, because it is set as this GameModeBase's DefaultPawnClass
					class AP1_Character* CreatedPlayer = Cast<AP1_Character>(NewPlayerController->GetCharacter());
					class AHUD* CreatedHud = Cast<AHUD>(NewPlayerController->GetHUD());
					class APlayerCameraManager* CreatedCamManager = Cast<APlayerCameraManager>(NewPlayerController->PlayerCameraManager);
					class ALocalMultiplayerDemoPlayerState* CreatedPlayerState = Cast<ALocalMultiplayerDemoPlayerState>(NewPlayerController->PlayerState);

					if (CreatedPlayer && CreatedHud && CreatedCamManager && CreatedPlayerState)
					{
						// DESTROY NEW PLAYER ONE and any new classes you aren't going to use
						// Keep APlayerState classes, because we use them for scoring. Keep APlayerCameraManager, too.
						CreatedPlayer->Destroy();
						CreatedHud->Destroy();
						
						// UnPossess the newly created APlayerController
						NewPlayerController->UnPossess();

						// Spawn player two
						class AP2_Character* PlayerTwoInWorld = world->SpawnActor<AP2_Character>(AP2_Character::StaticClass(), FinalSpawnPos, SpawnRot, spawnParams);

						if (PlayerTwoInWorld != nullptr)
						{
							// Have player two possess APlayerController at index 1
							NewPlayerController->Possess(PlayerTwoInWorld);
							hasSetSecondPlayer = true;
														
							// End method
							canFinishSetup = true;
						}
					}
				}
			}
		}
	}
}

// Load widget method
void ALocalMultiplayerDemoGameModeBase::LoadTwoPlayerWidget(float dTime)
{
	if (hasSetSecondPlayer)
	{
		if (!canSetWidget)
		{
			// Run short delay to make sure that player two has already been spawned and possessed
			delayWidgetSetupTimer += 1 * dTime;

			if (delayWidgetSetupTimer > 0.2f)
			{
				class UWorld* const world = GetWorld();

				if (world != nullptr)
				{
					// Check for Player Controller, Level, and HUD classes
					class ALevelScriptActor* LevelActorInst = Cast<ALevelScriptActor>(world->GetLevelScriptActor());
					class APlayerController* PlConZero = Cast<APlayerController>(UGameplayStatics::GetPlayerController(world, 0));
					class ALocalMultiplayerDemoHUD* HudFromPlConZero = Cast<ALocalMultiplayerDemoHUD>(PlConZero->GetHUD());

					if (PlConZero && HudFromPlConZero && LevelActorInst)
					{
						// Create widget
						HudFromPlConZero->CreateTwoPlayerUI();
						canSetWidget = true;
					}
				}
			}
		}
	}
}
#pragma endregion



