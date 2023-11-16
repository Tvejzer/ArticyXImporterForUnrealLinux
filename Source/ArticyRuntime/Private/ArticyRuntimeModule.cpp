//  
// Copyright (c) 2023 articy Software GmbH & Co. KG. All rights reserved.  
 
//

#include "ArticyRuntimeModule.h"
#include "Internationalization/StringTableRegistry.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "GenericPlatform/GenericPlatformFile.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

DEFINE_LOG_CATEGORY(LogArticyRuntime)

void FArticyRuntimeModule::StartupModule()
{
#if WITH_EDITOR
	FEditorDelegates::PostPIEStarted.AddRaw(this, &FArticyRuntimeModule::LoadStringTables);
#endif

	LoadStringTables(false);
}

void FArticyRuntimeModule::ShutdownModule()
{

}

void FArticyRuntimeModule::LoadStringTables(bool)
{
#if WITH_EDITOR
	IterateStringTables(FPaths::ProjectContentDir() / "ArticyContent/Generated", false);
#endif

	IterateStringTables(FPaths::ProjectContentDir() / "ArticyContent/Generated", true);
}

void FArticyRuntimeModule::IterateStringTables(const FString& DirectoryPath, bool Load) const
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	
	if (PlatformFile.DirectoryExists(*DirectoryPath))
	{
		TArray<FString> FoundFiles;
		PlatformFile.FindFiles(FoundFiles, *DirectoryPath, TEXT(".csv"));

		for (const FString& FilePath : FoundFiles)
		{
			FString StringTable = FPaths::GetBaseFilename(*FilePath, true);
			if (Load)
			{
				FStringTableRegistry::Get().Internal_LocTableFromFile(
					FName(StringTable),
					StringTable,
					"ArticyContent/Generated/" + StringTable + ".csv",
					FPaths::ProjectContentDir());
			}
			else
			{
				FStringTableRegistry::Get().UnregisterStringTable(FName(StringTable));
			}
		}
	}
}

IMPLEMENT_MODULE(FArticyRuntimeModule, ArticyRuntime)
