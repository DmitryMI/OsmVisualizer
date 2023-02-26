#include "OsmUtilsLibrary.h"
#include "Earth.h"
#include "JsonBlueprintFunctionLibrary.h"

inline TArray<FString> UOsmUtilsLibrary::SplitFilePath(const FString& filePath, char separator)
{
	TArray<FString> segments;
	FString currentToken = "";

	for (int i = 0; i < filePath.Len(); i++)
	{
		if (filePath[i] != separator)
		{
			currentToken += filePath[i];
		}
		else
		{
			segments.Add(currentToken);
			currentToken = "";
		}
	}

	if (!currentToken.IsEmpty())
	{
		segments.Add(currentToken);
	}

	return segments;
}

inline FString UOsmUtilsLibrary::ReconstructPath(const TArray<FString>& pathSegments, char pathSeparator)
{
	FString path;
	for (int segIndex = 0; segIndex < pathSegments.Num(); segIndex++)
	{
		const FString& seg = pathSegments[segIndex];
		path += seg;
		if (segIndex < pathSegments.Num() - 1)
		{
			path += pathSeparator;
		}
	}

	return path;
}

inline bool UOsmUtilsLibrary::DoesMatchPattern(const FString& str, const FString& pattern, char patternMatchingChar)
{
	int patternPos = 0;
	for (int i = 0; i < str.Len(); i++)
	{
		if (pattern[patternPos] == patternMatchingChar)
		{
			while (patternPos < pattern.Len() && pattern[patternPos] == patternMatchingChar)
			{
				patternPos++;
			}
			if (patternPos == pattern.Len())
			{
				return true;
			}

			while (i < str.Len() && str[i] != pattern[patternPos])
			{
				i++;
			}
			if (i == str.Len())
			{
				return false;
			}
		}
		else if (str[i] != pattern[patternPos])
		{
			return false;
		}
		patternPos++;
	}

	return patternPos == pattern.Len();
}

class FFindDirectoriesVisitor : public IPlatformFile::FDirectoryVisitor
{
public:
	IPlatformFile& PlatformFile;
	FRWLock          FoundFilesLock;
	TArray<FString>& FoundFiles;
	FFindDirectoriesVisitor(IPlatformFile& InPlatformFile, TArray<FString>& InFoundFiles)
		: IPlatformFile::FDirectoryVisitor(EDirectoryVisitorFlags::ThreadSafe)
		, PlatformFile(InPlatformFile)
		, FoundFiles(InFoundFiles)
	{
	}
	virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
	{
		if (bIsDirectory)
		{
			FString FileName(FilenameOrDirectory);
			FRWScopeLock ScopeLock(FoundFilesLock, SLT_Write);
			FoundFiles.Emplace(MoveTemp(FileName));
		}
		return true;
	}
};

TArray<FString> UOsmUtilsLibrary::GetFilesMatchingPattern(const FString& patternStr, const FString& patternMatchingCharStr)
{
	check(patternMatchingCharStr.Len() == 1);
	char pathSeparator = '/';
	char patternMatchingChar = patternMatchingCharStr[0];

	FString pattern = patternStr.Replace(TEXT("\\"), TEXT("/"));

	FString patternMatchingStr(1, &patternMatchingChar);

	TArray<FString> pathSegments = SplitFilePath(pattern, pathSeparator);

	TArray<FString> result;
	TArray<TArray<FString>> variantsStack;

	TArray<FString> initialPath;
	initialPath.Add(pathSegments[0]);
	variantsStack.Add(initialPath);

	IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();

	while (!variantsStack.IsEmpty())
	{
		TArray<FString> variantPathSegments = variantsStack.Pop();
		FString patternSegment = pathSegments[variantPathSegments.Num()];

		FString variantPath = ReconstructPath(variantPathSegments, pathSeparator);

		if (variantPathSegments.Num() + 1 == pathSegments.Num())
		{
			TArray<FString> foundFiles;

			platformFile.FindFiles(foundFiles, *variantPath, nullptr);

			for (const FString& file : foundFiles)
			{
				TArray<FString> filePathSegments = SplitFilePath(file, pathSeparator);
				FString lastSegment = filePathSegments.Last();
				if (!DoesMatchPattern(lastSegment, patternSegment, patternMatchingChar))
				{
					continue;
				}

				result.Add(file);
			}
		}
		else
		{
			TArray<FString> foundDirs;			

			FFindDirectoriesVisitor FindDirectoriesVisitor(platformFile, foundDirs);
			platformFile.IterateDirectory(*variantPath, FindDirectoriesVisitor);

			for (const FString& dir : foundDirs)
			{
				TArray<FString> dirPathSegments = SplitFilePath(dir, pathSeparator);
				FString lastSegment = dirPathSegments.Last();
				if (!DoesMatchPattern(lastSegment, patternSegment, patternMatchingChar))
				{
					continue;
				}

				variantsStack.Add(dirPathSegments);
			}
		}

	}

	return result;
}

void UOsmUtilsLibrary::BuildEarthFromJsonFile(const UObject* WorldContextObject, AEarth* earth, const FString& jsonFilePath)
{
	check(earth);

	FJsonObjectWrapper wrapper;
	FFilePath path;
	path.FilePath = jsonFilePath;
	if (!UJsonBlueprintFunctionLibrary::FromFile(const_cast<UObject*>(WorldContextObject), path, wrapper))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load JSON from file %s"), *jsonFilePath);
		return;
	}

	if (!earth->LoadFromJsonObject(wrapper))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to build earth from JSON file %s"), *jsonFilePath);
		return;
	}
}

void UOsmUtilsLibrary::BuildEarthFromJsonFilesPattern(const UObject* WorldContextObject, AEarth* earth, const FString& jsonFilesPattern, const FString& patternMatcher)
{
	TArray<FString> matchingFiles = GetFilesMatchingPattern(jsonFilesPattern, patternMatcher);

	for (const FString& file : matchingFiles)
	{
		BuildEarthFromJsonFile(WorldContextObject, earth, file);
	}
}
