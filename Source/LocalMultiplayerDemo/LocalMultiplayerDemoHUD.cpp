// Fill out your copyright notice in the Description page of Project Settings.

#include "LocalMultiplayerDemoHUD.h"
#include "LocalMultiplayerDemo.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"
#include "Runtime/UMG/Public/Slate/SObjectWidget.h"
#include "Runtime/UMG/Public/IUMGModule.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"

ALocalMultiplayerDemoHUD::ALocalMultiplayerDemoHUD()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> WidgetAsset(TEXT("/Game/Blueprints/PlayerUI"));

	if (WidgetAsset.Succeeded())
	{
		PlayerWidgetClass = WidgetAsset.Class;
	}
}

// Called when the game starts or when spawned
void ALocalMultiplayerDemoHUD::BeginPlay()
{
	Super::BeginPlay();

	// If one player game, you can create widget here
}

// Create 2P widget and add to viewport
void ALocalMultiplayerDemoHUD::CreateTwoPlayerUI()
{
	class UWorld* const world = GetWorld();

	if (world != NULL)
	{
		if (PlayerWidgetClass != NULL) 
		{
			PlayerUI = CreateWidget<UUserWidget>(world, PlayerWidgetClass);

			if (PlayerUI != NULL)
				PlayerUI->AddToViewport();
		}
	}
}




