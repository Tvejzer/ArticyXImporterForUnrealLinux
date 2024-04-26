//  
// Copyright (c) 2023 articy Software GmbH & Co. KG. All rights reserved.  
//

#include "ArticyLocalizerGenerator.h"
#include "ArticyImportData.h"
#include "ArticyGlobalVariables.h"
#include "ArticyImporterHelpers.h"
#include "ArticyTypeSystem.h"
#include "CodeFileGenerator.h"
#include "CodeGenerator.h"
#include "Internationalization/StringTableRegistry.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"

void ArticyLocalizerGenerator::GenerateCode(const UArticyImportData* Data, FString& OutFile)
{
	if(!ensure(Data))
		return;

	OutFile = CodeGenerator::GetArticyLocalizerClassname(Data, true);
	CodeFileGenerator(OutFile + ".h", true, [&](CodeFileGenerator* header)
	{
		header->Line("#include \"CoreUObject.h\"");
		header->Line("#include \"ArticyRuntime/Public/ArticyLocalizerSystem.h\"");
		header->Line("#include \"" + OutFile + ".generated.h\"");

		header->Line();

		// Generate the UArticyLocalizerSystem class
		const auto type = CodeGenerator::GetArticyLocalizerClassname(Data, false);
		header->Class(type + " : public UArticyLocalizerSystem", TEXT("Articy Localizer System"), true, [&]
		{
			header->AccessModifier("public");

			header->Method("void", "Reload", "", [&]
			{
				header->Line(TEXT("const FString& LangName = FInternationalization::Get().GetCurrentCulture()->GetTwoLetterISOLanguageName()"), true);
				header->Line(TEXT("if (!bListenerSet) {"));
				header->Line(FString::Printf(TEXT("FInternationalization::Get().OnCultureChanged().AddUObject(this, &%s::Reload)"), *type), true, true, 1);
				header->Line(TEXT("bListenerSet = true"), true, true, 1);
				header->Line(TEXT("}"));

				FString L10NDir = FPaths::ProjectContentDir() / "L10N";
				if (FPaths::DirectoryExists(L10NDir))
				{
					for (const auto Language : Data->Languages.Languages)
					{
						FString LangPath = L10NDir / Language.Key / "ArticyContent/Generated";
						header->Line(FString::Printf(TEXT("if (LangName.Equals(\"%s\")) {"), *Language.Key));
						IterateStringTables(header, LangPath);
						header->Line(TEXT("}"));
					}
				}
				IterateStringTables(header, FPaths::ProjectContentDir() / "ArticyContent/Generated");
			});
		});
	});
}

void ArticyLocalizerGenerator::IterateStringTables(CodeFileGenerator* Header, const FString& DirectoryPath)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (PlatformFile.DirectoryExists(*DirectoryPath))
	{
		TArray<FString> FoundFiles;
		PlatformFile.FindFiles(FoundFiles, *DirectoryPath, TEXT(".csv"));

		FString RelPath = DirectoryPath.Replace(*FPaths::ProjectContentDir(), TEXT(""), ESearchCase::IgnoreCase);

		for (const FString& FilePath : FoundFiles)
		{
			FString StringTable = FPaths::GetBaseFilename(*FilePath, true);
			Header->Line(FString::Printf(TEXT("FStringTableRegistry::Get().UnregisterStringTable(FName(\"%s\"))"), *StringTable), true);
			Header->Line(FString::Printf(TEXT("LOCTABLE_FROMFILE_GAME(\"%s\", \"%s\", \"%s/%s.csv\")"), *StringTable, *StringTable, *RelPath, *StringTable), true);
		}
	}
}
