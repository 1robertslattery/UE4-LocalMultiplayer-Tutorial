// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LocalMultiplayerDemoHUD.generated.h"

UCLASS()
class LOCALMULTIPLAYERDEMO_API ALocalMultiplayerDemoHUD : public AHUD
{
	GENERATED_BODY()
		
public:

	ALocalMultiplayerDemoHUD();
	
protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	// Widget Blueprint Class
	TSubclassOf<class UUserWidget> PlayerWidgetClass;

	// Point to Widget Class
	class UUserWidget* PlayerUI;

	// Widget Method
	void CreateTwoPlayerUI();

};
