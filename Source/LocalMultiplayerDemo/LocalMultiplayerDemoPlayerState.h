// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "LocalMultiplayerDemoPlayerState.generated.h"

UCLASS()
class LOCALMULTIPLAYERDEMO_API ALocalMultiplayerDemoPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:

	ALocalMultiplayerDemoPlayerState();
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	int32 TotalScore_P1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	int32 TotalScore_P2;
	
};
