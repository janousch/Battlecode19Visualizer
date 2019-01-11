// Fill out your copyright notice in the Description page of Project Settings.

#include "Replay.h"

#include <math.h>


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

double UReplay::Random() {
	double X = sin(Seed++) * 10000;
	double Random = X - FMath::FloorToInt(X);
	return Random;
}

FBCMap UReplay::InitializeMap(FBCMap Map) {
	float ChanceToStartAlive = Random()*0.07 + 0.38;

	for (int x = 0; x < Map.MapWidth; x++) {
		for (int y = 0; y < Map.MapHeight; y++) {
			if (Random() < ChanceToStartAlive) {
				Map.SetField(x, y, 1);
			}
			else {
				Map.SetField(x, y, 0);
			}
		}
	}

	return Map;
}

FBCMap UReplay::MakeMap() {
	int32 Width = FMath::FloorToInt(Random() * 33) + 32;
	int32 Height = Width / 2;

	FBCMap CellMap = FBCMap(Width, Height);
	CellMap = InitializeMap(CellMap);
	
	int32 NumberOfSteps = 2;
	for (int i = 0; i < NumberOfSteps; i++) {
		CellMap = DoSimulationStep(CellMap);
	}

	// Invert ?!
	for (int x = 0; x < CellMap.MapWidth; x++) {
		for (int y = 0; y < CellMap.MapHeight; y++) {
			int32 Value = CellMap.GetField(x, y) == 0 ? 1 : 0;
			CellMap.SetField(x, y, Value);
		}
	}

	TArray<TArray<FPair>> Regions;
	FBCMap Visited = FBCMap(Width, Height);
	for (int y = 0; y < Height; y++) {
		for (int x = 0; x < Width; x++) {
			if (CellMap.GetField(x, y) == 1 && Visited.GetField(x, y) == 0) {
				TArray<FPair> Stack;
				Regions.Add(TArray<FPair>());
				Stack.Insert(FPair(x, y), 0);

				while (Stack.Num() != 0)
				{
					FPair P = Stack.Last();
					int32 StackX = P.First, StackY = P.Second;
					Stack.RemoveAt(Stack.Num() - 1);
					Regions[Regions.Num() - 1].Add(P);
					Visited.SetField(StackX, StackY, 1);

					if (StackY > 0 && CellMap.GetField(StackX, StackY - 1) == 1 && Visited.GetField(StackX, StackY - 1) == 0) Stack.Insert(FPair(StackX, StackY - 1), 0);
					if (StackX > 0 && CellMap.GetField(StackX - 1, StackY) == 1 && Visited.GetField(StackX - 1, StackY) == 0) Stack.Insert(FPair(StackX - 1, StackY), 0);
					if (StackY < Height - 1 && CellMap.GetField(StackX, StackY + 1) == 1 && Visited.GetField(StackX, StackY + 1) == 0) Stack.Insert(FPair(StackX, StackY + 1), 0);
					if (StackX < Width - 1 && CellMap.GetField(StackX + 1, StackY) == 1 && Visited.GetField(StackX + 1, StackY) == 0) Stack.Insert(FPair(StackX + 1, StackY), 0);
				}
			}
		}
	}

	Regions.Sort([](const TArray<FPair>& V1, const TArray<FPair> V2) { return V1.Num() > V2.Num(); });

	for (int i = 1; i < Regions.Num(); i++) {
		TArray<FPair> Pairs = Regions[i];
		for (int j = 0; j < Pairs.Num(); j++) {
			CellMap.SetField(Pairs[j].First, Pairs[j].Second, 0);
		}
	}

	CellMap = HorizontalMirroring(CellMap);
	CellMap.MapHeight = CellMap.Columns[0].Rows.Num();
	CellMap.MapWidth = CellMap.Columns.Num();


	return CellMap;
}

FBCMap UReplay::HorizontalMirroring(FBCMap OldMap) {
	FBCMap NewMap = OldMap;

	for (int x = 0; x < OldMap.MapWidth; x++) {
		for (int y = OldMap.MapHeight - 1; y >= 0; y--) {
			NewMap.Columns[x].Rows.Add(OldMap.GetField(x, y));
		}
	}
	
	return NewMap;
}

FBCMap UReplay::VerticalMirroring(FBCMap OldMap) {
	FBCMap NewMap = OldMap;

	for (int x = OldMap.MapWidth - 1; x >= 0; x--) {
		FColumn NewColumn;
		for (int y = 0; y < OldMap.MapHeight; y++) {
			NewColumn.Rows.Add(OldMap.GetField(x, y));
		}
		NewMap.Columns.Add(NewColumn);
	}
	
	return NewMap;
}

FBCMap UReplay::DoSimulationStep(FBCMap OldMap) {
	int32 BIRTH_LIMIT = 5;
	int32 DEATH_LIMIT = 4;

	FBCMap NewMap = FBCMap(OldMap.MapWidth, OldMap.MapHeight);
	//Loop over each row and column of the map
	for (int x = 0; x < OldMap.MapWidth; x++) {
		for (int y = 0; y < OldMap.MapHeight; y++) {
			int nbs = OldMap.CountAliveNeighbours(x, y);
			//The new value is based on our simulation rules
			//First, if a cell is alive but has too few neighbours, kill it.
			if (OldMap.GetField(x, y) == 1) {
				if (nbs < DEATH_LIMIT) {
					NewMap.SetField(x, y, 0);
				}
				else {
					NewMap.SetField(x, y, 1);
				}
			} //Otherwise, if the cell is dead now, check if it has the right number of neighbours to be 'born'
			else {
				if (nbs > BIRTH_LIMIT) {
					NewMap.SetField(x, y, 1);
				}
				else {
					NewMap.SetField(x, y, 0);
				}
			}
		}
	}
	return NewMap;
}

/*
FBCMap UReplay::MakeMap() {
	int32 Width = FMath::FloorToInt(GetRandom() * 33) + 32;
	int32 Height = Width;

	FBCMap PassMap = FBCMap(Width, Height);

	//Figure out chunk Width and Height accordingly.
	// Here, we have just two players and are assuming horizontal orientation to start, so this is easy
	int32 ch = FMath::CeilToInt(Height / 2);
	int32 cw = Width;

	// Determine passability using ideas from 
	// https://gamedevelopment.tutsplus.com/tutorials/generate-random-cave-levels-using-cellular-automata--gamedev-9664
	// Note: It's really sensitive to all three of these parameters.
	float StartAlive = GetRandom()*0.07 + 0.38;
	int32 Birth = 5;
	int32 Death = 4;

	PassMap.SetFields(0, cw, 0, ch, -1);

	for (int32 w = 0; w < cw; w++) for (int32 h = 0; h < ch; h++) {
		int32 Value = GetRandom() < StartAlive ? 1 : 0;
		PassMap.SetField(w, h, Value);
	}

	for (int32 i = 0; i < 2; i++) {
		FBCMap NewPassMap = FBCMap(cw, ch);
		for (int32 n = 0; n < cw; n++) {
			for (int32 m = 0; m < ch; m++) {
				int32 TrueSquares = PassMap.CountNumberOfTrueSquares(m, n);
				int32 Value = (PassMap.GetField(n, m) && TrueSquares >= Death) || (!PassMap.GetField(n, m) && TrueSquares >= Birth);
				NewPassMap.SetField(n, m, Value);
			}
		}
		PassMap = NewPassMap;
	}

	// Invert the PassMap
	for (int32 n = 0; n <cw; n++) {
		for (int32 m = 0; m < ch; m++) {
			PassMap.SetField(n, m, PassMap.GetField(n, m) > 0 ? 0 : 1);
		}
	}

	// Flood fill to find all of the different sections of the map
	TArray<int32> Regions;
	FBCMap Visited = FBCMap(cw, ch);

	for (int32 m = 0; m < ch; m++) {
		for (int32 n = 0; n < cw; n++) {
			if (PassMap.GetField(n, m) > 0 && Visited.GetField(n, m) < 1) {
				FBCMap Stack = FBCMap(n, m); // Stack-based DFS to flood-fill
				while (Stack.Column.Num() > 0) {
					int32 coords = Stack.pop();
					int32 x = coords[0];
					int32 y = coords[1];

					Regions[Regions.length - 1].push(coords);
					Visited[y][x] = true;

					if (y > 0 && PassMap[y - 1][x] && !Visited[y - 1][x]) Stack.push([x, y - 1]);
					if (x > 0 && PassMap[y][x - 1] && !Visited[y][x - 1]) Stack.push([x - 1, y]);
					if (y < ch - 1 && PassMap[y + 1][x] && !Visited[y + 1][x]) Stack.push([x, y + 1]);
					if (x < cw - 1 && PassMap[y][x + 1] && !Visited[y][x + 1]) Stack.push([x + 1, y]);
				}
			}
		}
	}
	/*

	Regions.sort(x = > -1 * x.length);
	for (int32 region = 1; region < Regions.length; region++) {
		for (int32 i = 0; i<Regions[region].length; i++) {
			int32 coord = Regions[region][i];
			PassMap[coord[1]][coord[0]] = false;
		}
	}

	// Generate other features: castles, karbonite and fuel depots

	// Select number of castles:
	int32 num_castles = Math.min(Math.max(Math.floor(2.5*GetRandom() + ((Height + Width) / 64) - 0.5), 1), 3);

	// Choose their locations. We prefer for opposing castles to be far away, so we'll weight the distribution towards the bottom of the map depending on prefer_horizontal and ignore castles which are too close to the midline of the map. Additionally, we want castles on the same team to be at least a certain distance from each other, so we'll re-roll if that's not met.
	int32 roll_castle = function() {
		int32 triangle_ch = ch*(ch + 1) / 2;
		int32 y = 0;
		while (y<3 || y > ch - 8) y = ch - Math.floor(0.5*(Math.sqrt(1 + 8 * GetRandom()*triangle_ch) - 1));
		int32 x = Math.floor(GetRandom()*cw);

		return[x, y]
	}.bind(this);

	int32 castles = [];
	int32 counter = 0; // This is for the rare case that a solution doesn't actually exist, to ensure that we terminate.
	for (int32 i = 0; i<num_castles; i++) {
		int32 coord = roll_castle();

		// forgive me christ
		while (!PassMap[coord[1]][coord[0]] || (castles.length > 0 && Math.min.apply(NULL, castles.map(c = > Math.abs(c[0] - coord[0]) + Math.abs(c[1] - coord[1]))) < 16) && counter < 1000) {
			coord = roll_castle();
			counter += 1;
		}

		if (counter < 1000) castles.push(coord);
	}

	int32 roll_resource_seed = _ = >[Math.floor(GetRandom()*cw), Math.floor(GetRandom()*ch)];

	int32 resource_density = GetRandom() * (1 / 200 - 1 / 400) + 1 / 400;
	int32 num_resource_clusters = Math.round(cw*ch*resource_density);

	int32 resources_cluster_seeds = insulate(castles); // Castles must be seeds of resources
	counter = 0; // This is for the rare case that a solution doesn't actually exist, to ensure that we terminate.
	for (int32 n = resources_cluster_seeds.length; n<num_resource_clusters; n++) {
		int32 coord = roll_resource_seed();

		// oops i did it again
		while (!PassMap[coord[1]][coord[0]] || Math.min.apply(NULL, resources_cluster_seeds.map(c = > Math.abs(c[0] - coord[0]) + Math.abs(c[1] - coord[1]))) < 12 && counter < 1000) {
			coord = roll_resource_seed();
			counter += 1;
		}

		if (counter < 1000) resources_cluster_seeds.push(coord);
	}

	// Now that we have the seed locations for the clusters, we'll roll for how many resources we put in (karbonite and fuel separately)
	// Locations closer to the midline will tend to have more resources, to discourage turtling.
	int32 karbonite_depots = [];
	int32 fuel_depots = [];
	Visited = MakeMapHelper(NULL, cw, ch);

	// helper to check if coordinate is in a list
	function c_in(c, l) {
		for (int32 i = 0; i<l.length; i++) {
			if (l[i][0] == = c[0] && l[i][1] == = c[1]) return true;
		} return false;
	}

	for (int32 i = 0; i<resources_cluster_seeds.length; i++) {
		int32 x;
		int32 y;

		[x, y] = resources_cluster_seeds[i];

		// Choose amount of karbonite and fuel in the cluster.
		int32 num_karbonite = Math.max(1, Math.round(GetRandom() * 4 * (y / ch / 2 + .5)));
		int32 num_fuel = Math.max(1, Math.round(GetRandom() * 4 * (y / ch / 2 + .5)));
		int32 total_depot = num_karbonite + num_fuel;

		// We now run a BFS to choose
		int32 region = [];
		int32 queue = []; // Queue-based BFS for finding the region:
		queue.push([x, y]);

		for (int32 z = 0; z<5 * total_depot; z++) { // Choose an area 5x larger than necessary for the resource cluster.
			[x, y] = queue.pop(0);
			region.push([x, y]);
			Visited[y][x] = true;

			if (y > 0 && PassMap[y - 1][x] && !Visited[y - 1][x]) queue.push([x, y - 1]);
			if (x > 0 && PassMap[y][x - 1] && !Visited[y][x - 1]) queue.push([x - 1, y]);
			if (y < ch - 1 && PassMap[y + 1][x] && !Visited[y + 1][x]) queue.push([x, y + 1]);
			if (x < cw - 1 && PassMap[y][x + 1] && !Visited[y][x + 1]) queue.push([x + 1, y]);

		}

		// Choose the actual karbonite and fuel locations

		counter = 0; // This is for the rare case that a solution doesn't actually exist, to ensure that we terminate.
		for (int32 k = 0; k<num_karbonite; k++) {
			[x, y] = region[Math.floor(GetRandom()*region.length)];

			while ((c_in([x, y], castles) || c_in([x, y], karbonite_depots) || c_in([x, y], fuel_depots)) && counter < 10000) {
				[x, y] = region[Math.floor(GetRandom()*region.length)];
				counter++;
			}

			if (counter < 10000) karbonite_depots.push([x, y]);
		}
		counter = 0; // This is for the rare case that a solution doesn't actually exist, to ensure that we terminate.
		for (int32 k = 0; k<num_fuel; k++) {
			[x, y] = region[Math.floor(GetRandom()*region.length)];

			while ((c_in([x, y], castles) || c_in([x, y], karbonite_depots) || c_in([x, y], fuel_depots)) && counter < 10000) {
				[x, y] = region[Math.floor(GetRandom()*region.length)];
				counter++;
			}

			if (counter < 10000) fuel_depots.push([x, y]);
		}


	}

	// Convert lists into bool maps
	int32 karb_map = MakeMapHelper(false, cw, ch);

	int32 fuel_map = MakeMapHelper(false, cw, ch);

	for (int32 i = 0; i<karbonite_depots.length; i++) {
		karb_map[karbonite_depots[i][1]][karbonite_depots[i][0]] = true;
	}

	for (int32 i = 0; i<fuel_depots.length; i++) {
		fuel_map[fuel_depots[i][1]][fuel_depots[i][0]] = true;
	}

	// mirror the map
	int32 full_passmap = MakeMapHelper(false, Width, Height);

	int32 full_karbmap = MakeMapHelper(false, Width, Height);

	int32 full_fuelmap = MakeMapHelper(false, Width, Height);

	int32 transpose = GetRandom() < 0.5;
	for (int32 n = 0; n<Height; n++) {
		for (int32 m = 0; m<Width; m++) {
			full_passmap[transpose ? m : n][transpose ? n : m] = n<ch ? PassMap[n][m] : PassMap[Height - n - 1][m];
			full_fuelmap[transpose ? m : n][transpose ? n : m] = n<ch ? fuel_map[n][m] : fuel_map[Height - n - 1][m];
			full_karbmap[transpose ? m : n][transpose ? n : m] = n<ch ? karb_map[n][m] : karb_map[Height - n - 1][m];
		}
	}

	int32 all_castles = castles.concat(castles.map(c = >[c[0], Height - c[1] - 1]));
	if (transpose) all_castles = all_castles.map(c = >[c[1], c[0]]);


	this.shadow = MakeMapHelper(0, Width, Height);

	this.karbonite_map = full_karbmap;
	this.fuel_map = full_fuelmap;
	this.map = full_passmap;

	int32 to_create = [];

	for (int32 i = 0; i<all_castles.length / 2; i++) {
		to_create.push({
		team:0,
				x : all_castles[i][0],
					y : all_castles[i][1]
		});

		to_create.push({
		team:1,
				x : all_castles[(all_castles.length / 2) + i][0],
					y : all_castles[(all_castles.length / 2) + i][1]
		});
	}
	*/
/*
	return PassMap;
}
*/

void UReplay::InitializeSeed() {
	Seed = 0;

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