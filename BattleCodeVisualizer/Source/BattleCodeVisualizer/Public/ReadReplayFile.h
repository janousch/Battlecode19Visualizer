// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Replay.h"

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ReadReplayFile.generated.h"

/**
 * 
 */
UCLASS()
class BATTLECODEVISUALIZER_API UReadReplayFile : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, Category = "Replay")
		static TArray<FString> GetReplayFileNames();
	
	UFUNCTION(BlueprintCallable, Category = "Replay")
		static UReplay* ReadReplay(FString File);
};
