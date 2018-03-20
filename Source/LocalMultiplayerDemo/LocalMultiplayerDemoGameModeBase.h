// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LocalMultiplayerDemoGameModeBase.generated.h"

USTRUCT(BlueprintType)
struct FRespawnSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVector RespawnPosition_1;
	
	UPROPERTY()
	FVector RespawnPosition_2;
	
	UPROPERTY()
	FVector RespawnPosition_3;
	
	UPROPERTY()
	FVector RespawnPosition_4;

};

UCLASS()
class LOCALMULTIPLAYERDEMO_API ALocalMultiplayerDemoGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
private:

	// Two Player Mode Variables
	bool hasSetSecondPlayer;
	bool canFinishSetup;

	// Load Widget Variables
	bool canSetWidget;
	float delayWidgetSetupTimer;
	
	// Method to Spawn Player Two
	void SetupTwoPlayers();
	
public:

	// Sets default values for this character's properties
	ALocalMultiplayerDemoGameModeBase();

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Player One Reference
	UPROPERTY()
	class AP1_Character* PlayerOneInWorld;

	// Level Reference
	class ALevelScriptActor* LevelActorInstance;

	// Level Name
	static const FString MyLevelName;

public:

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Load UI Method
	void LoadTwoPlayerWidget(float dTime);

	// Two Player Variable
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Play Mode")
	bool isTwoPlayerMode;

public:

	// Struct Reference
	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	FRespawnSettings RespawnSetup;

	// Populate World With Respawn Locations
	void CreateRespawnPoints();
	
};
