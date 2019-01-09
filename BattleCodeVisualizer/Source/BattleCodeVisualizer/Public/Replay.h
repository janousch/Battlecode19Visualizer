// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Replay.generated.h"

USTRUCT(BlueprintType)
struct FRow {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
		TArray<int32> Row;

	FRow(int32 Width) {
		Row.SetNumUninitialized(Width, false);
	}

	FRow() {}
};


USTRUCT(BlueprintType)
struct FBCMap {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
		TArray<FRow> Column;

	void SetField(int32 X, int32 Y, int32 Value) {
		Column[Y].Row[X] = Value;
	}

	int32 GetField(int32 X, int32 Y) {
		return Column[Y].Row[X];
	}

	void SetFields(int32 XStart, int32 XEnd, int32 YStart, int32 YEnd, int32 Value) {
		for (int y = YStart; y < YEnd; y++) {
			for (int x = XStart; x < XEnd; x++) {
				SetField(x, y, Value);
			}
		}
	}

	int32 CountNumberOfTrueSquares(int32 X, int32 Y) {
		int32 Counter = 0;
		
		for (auto& i : { -1, 0, 1 }) {
			int32 YCoord = Y + i;
			for (auto j : { -1, 0, 1 }) {
				int32 XCoord = X + j;
				if (YCoord < MapHeight && XCoord < MapWidth && YCoord >= 0 && XCoord >= 0) {
					Counter += GetField(XCoord, YCoord);
				}
			}
		}

		return Counter;
	}

	FBCMap(int32 Width, int32 Height) {
		MapWidth = Width;
		MapHeight = Height;

		Column.Init(FRow(Width), Height);
	}
	
	FBCMap() {}

private:

	int32 MapWidth;
	int32 MapHeight;
};

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

	UFUNCTION(BlueprintPure, Category = "Replay")
		float GetRandom();

	UFUNCTION(BlueprintCallable, Category = "Replay")
		FBCMap MakeMap();

	void Initialize();

private:

	int32 Seed;

	FActionRecord DeserializeTurn(TArray<uint8> Turn);
	
};
