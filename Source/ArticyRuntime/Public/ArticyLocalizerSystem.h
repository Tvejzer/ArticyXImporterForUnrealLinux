//  
// Copyright (c) 2023 articy Software GmbH & Co. KG. All rights reserved.  
//

#pragma once

#include "CoreMinimal.h"
#include "ArticyType.h"
#include "ArticyHelpers.h"
#include "Internationalization/StringTableRegistry.h"
#include "ArticyLocalizerSystem.generated.h"

UCLASS(BlueprintType)
class ARTICYRUNTIME_API UArticyLocalizerSystem : public UObject
{
	GENERATED_BODY()

public:
	static UArticyLocalizerSystem* Get();

	virtual void Reload() {};

	inline FText LocalizeString(UObject* Outer, const FText& Key, bool ResolveTextExtension = true, const FText* BackupText = nullptr)
	{
		if (!bDataLoaded)
		{
			Reload();
		}

		const FText MissingEntry = FText::FromString("<MISSING STRING TABLE ENTRY>");

		// Look up entry in specified string table
		TOptional<FString> TableName = FTextInspector::GetNamespace(Key);
		if (!TableName.IsSet())
		{
			TableName = TEXT("ARTICY");
		}
		const FText SourceString = FText::FromStringTable(
			FName(TableName.GetValue()),
			Key.ToString(),
			EStringTableLoadingPolicy::FindOrFullyLoad);
		const FString Decoded = SourceString.ToString();
		if (!SourceString.IsEmpty() && !SourceString.EqualTo(MissingEntry))
		{
			if (ResolveTextExtension)
			{
				return ArticyHelpers::ResolveText(Outer, &SourceString);
			}
			return SourceString;
		}

		if (BackupText)
		{
			return *BackupText;
		}

		// By default, return via the key
		if (ResolveTextExtension)
		{
			return ArticyHelpers::ResolveText(Outer, &Key);
		}
		return Key;
	}
	FArticyType GetArticyType(const FString& TypeName) const;
	TMap<FString, FArticyType> Types;

protected:
	bool bDataLoaded = false;
	bool bListenerSet = false;
};
