// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Replay.generated.h"

USTRUCT(BlueprintType)
struct FActionRecord {
	GENERATED_BODY()

		UPROPERTY(BlueprintReadOnly)
		int32 Signal;
	UPROPERTY(BlueprintReadOnly)
		int32 SignalRadius;
	UPROPERTY(BlueprintReadOnly)
		int32 CastleTalk;
	UPROPERTY(BlueprintReadOnly)
		int32 Action;
	UPROPERTY(BlueprintReadOnly)
		int32 TradeFuel;
	UPROPERTY(BlueprintReadOnly)
		int32 TradeKarbonite;
	UPROPERTY(BlueprintReadOnly)
		int32 Dx;
	UPROPERTY(BlueprintReadOnly)
		int32 Dy;
	UPROPERTY(BlueprintReadOnly)
		int32 GiveKarbonite;
	UPROPERTY(BlueprintReadOnly)
		int32 GiveFuel;
	UPROPERTY(BlueprintReadOnly)
		int32 BuildUnit;

};

/**
 * 
 */
UCLASS(BlueprintType)
class BATTLECODEVISUALIZER_API UReplay : public UObject
{
	GENERATED_BODY()
	
public:

	TArray<uint8> Bytes;

	UFUNCTION(BlueprintPure, Category = "Replay")
		FActionRecord GetTurn(int32 Turn, int32 Offset);

	UFUNCTION(BlueprintPure, Category = "Replay")
		int32 GetNumberOfTurns();

	void Initialize();

private:

	int32 Seed;

	FActionRecord DeserializeTurn(TArray<uint8> Turn);
	
};
