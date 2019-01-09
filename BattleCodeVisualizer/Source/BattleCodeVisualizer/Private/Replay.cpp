// Fill out your copyright notice in the Description page of Project Settings.

#include "Replay.h"


FActionRecord UReplay::GetTurn(int32 Turn, int32 Offset) {
	TArray<uint8> DifferenceToNextTurn;
	int32 StreamBegin = (6 + 8 * Turn) + Offset;
	int32 StreamEnd = (6 + 8 * (Turn + 1)) + Offset;
	for (int i = StreamBegin; i < StreamEnd; i++) {
		DifferenceToNextTurn.Add(Bytes[i]);
	}

	return DeserializeTurn(DifferenceToNextTurn);
}

int32 UReplay::GetNumberOfTurns() {
	return (Bytes.Num() - 6) / 8;
}

void UReplay::Initialize() {
	for (int i = 0; i < 4; i++) {
		Seed += Bytes[i + 2] << (24 - 8 * i);
	}
}

FActionRecord UReplay::DeserializeTurn(TArray<uint8> Turn) {
	FActionRecord Action = FActionRecord();

	if (Turn.Num() != 8) return Action;

	Action.Signal = (Turn[0] << 8) + Turn[1];

	Action.SignalRadius = Turn[2] << 7;
	Action.SignalRadius += Turn[3] >> 1;

	Action.CastleTalk = (Turn[3] % 2) << 7;
	Action.CastleTalk += Turn[4] >> 1;

	Action.Action = (Turn[4] % 2) << 2;
	Action.Action += Turn[5] >> 6;
	if (Action.Action == 5) {
		Action.TradeFuel = (Turn[5] % (int32)FMath::Pow(2, 6)) << 5;
		Action.TradeFuel += Turn[6] >> 3;
		Action.TradeKarbonite = (Turn[6] % (int32)FMath::Pow(2, 3)) << 8;
		Action.TradeKarbonite += Turn[7];
	}

	else if (Action.Action == 1 || Action.Action == 2) {
		Action.Dx = (Turn[6] >> 7 == 1 ? -1 : 1) * (Turn[6] % (int32)FMath::Pow(2, 7));
		Action.Dy = (Turn[7] >> 7 == 1 ? -1 : 1) * (Turn[7] % (int32)FMath::Pow(2, 7));
	}

	else if (Action.Action == 3 || Action.Action == 6) {
		Action.Dy = Turn[5] % (int32)FMath::Pow(2, 3);
		Action.Dx = (Turn[5] >> 3) % (int32)FMath::Pow(2, 3);

		Action.Dy = (1 - 2 * (Action.Dy >> 2)) * (Action.Dy % (int32)FMath::Pow(2, 2));
		Action.Dx = (1 - 2 * (Action.Dx >> 2)) * (Action.Dx % (int32)FMath::Pow(2, 2));

		if (Action.Action == 3) { // build
			Action.BuildUnit = Turn[6];
		}
		else { // give
			Action.GiveKarbonite = Turn[6];
			Action.GiveFuel = Turn[7];
		}
	}

	return Action;
}