// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Replay.generated.h"

USTRUCT()
struct FPair {
	GENERATED_BODY()

	int32 First;
	int32 Second;

	FPair(int32 FirstValue, int32 SecondValue) {
		First = FirstValue;
		Second = SecondValue;
	}

	FPair() {}
};

USTRUCT(BlueprintType)
struct FColumn {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
		TArray<int32> Rows;

	FColumn(int32 Height) {
		Rows.SetNumUninitialized(Height, false);
	}

	FColumn() {}
};


USTRUCT(BlueprintType)
struct FBCMap {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
		TArray<FColumn> Columns;

	int32 MapWidth;
	int32 MapHeight;

	void SetField(int32 X, int32 Y, int32 Value) {
		Columns[X].Rows[Y] = Value;
	}

	int32 GetField(int32 X, int32 Y) {
		return Columns[X].Rows[Y];
	}

	void SetFields(int32 XStart, int32 XEnd, int32 YStart, int32 YEnd, int32 Value) {
		for (int y = YStart; y < YEnd; y++) {
			for (int x = XStart; x < XEnd; x++) {
				SetField(x, y, Value);
			}
		}
	}

	int32 CountAliveNeighbours(int32 X, int32 Y) {		
		int32 Count = 0;
		for (int i = -1; i < 2; i++) {
			for (int j = -1; j < 2; j++) {
				int32 NeighbourX = X + i;
				int32 NeighbourY = Y + j;
				//If we're looking at the middle point
				if (i == 0 && j == 0) {
					//Do nothing, we don't want to add ourselves in!
				}
				//In case the index we're looking at it off the edge of the map
				else if (NeighbourX < 0 || NeighbourY < 0 || NeighbourX >= MapWidth || NeighbourY >= MapHeight) {
					Count++;
				}
				//Otherwise, a normal check of the neighbour
				else if (GetField(NeighbourX, NeighbourY) == 1) {
					Count++;
				}
			}
		}

		return Count;
	}

	FBCMap(int32 Width, int32 Height) {
		MapWidth = Width;
		MapHeight = Height;

		Columns.Init(FColumn(Height), Width);
	}
	
	FBCMap() {}
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

	UFUNCTION(BlueprintCallable, Category = "Replay")
		FBCMap MakeMap();

	void InitializeSeed();

private:

	int32 Seed;

	FActionRecord DeserializeTurn(TArray<uint8> Turn);
	
	FBCMap InitializeMap(FBCMap Map);

	FBCMap DoSimulationStep(FBCMap OldMap);

	double Random();

	FBCMap HorizontalMirroring(FBCMap OldMap);

	FBCMap VerticalMirroring(FBCMap OldMap);
};
