// Fill out your copyright notice in the Description page of Project Settings.

#include "ReadReplayFile.h"

#include "FileManager.h"
#include "Paths.h"
#include "FileHelper.h"

TArray<FString> UReadReplayFile::GetReplayFileNames() {
	TArray<FString> ReplayFiles;
	FString Directory = FPaths::ProjectDir() + "/Replays";
	FString Filename = "*.bc19";

	IFileManager& FileManager = IFileManager::Get();
	FileManager.FindFilesRecursive(ReplayFiles, *Directory, *Filename, true, false);

	return ReplayFiles;
}

UReplay* UReadReplayFile::ReadReplay(FString File) {
	UReplay* Replay = NewObject<UReplay>(UReplay::StaticClass());
	FFileHelper::LoadFileToArray(Replay->Bytes, *File);
	Replay->InitializeSeed();

	return Replay;
}

