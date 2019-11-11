//
//  FakeResources.c
//  ReClassicfication
//
//  Created by Uli Kusterer on 20.02.13.
//  Copyright (c) 2013 Uli Kusterer. All rights reserved.
//

#include <stdint.h>
#include <string.h>	// for memmove().
#include <unistd.h>
#include "FakeResources.h"
#include "EndianStuff.h"


// Turn this on if you want to read actual Mac resource forks on a Mac or Darwin
//	but using the ANSI file APIs (uses Apple's "/..namedfork/rsrc" trick).
#define READ_REAL_RESOURCE_FORKS		1


/*
	resource data offset									  4 bytes
	resource map offset										  4 bytes
	resource data length									  4 bytes
	resource map length										  4 bytes
	Reserved for system use									112 bytes
	Application data										128 bytes
	resource data											...
	resource map											...


	Resource data is 4-byte-long-counted, followed by actual data.
	
	
	Resource map:
	
	copy of resource header	in RAM							 16 bytes
	next resource map in RAM								  4 bytes
	file reference number in RAM							  2 bytes
	Resource file attributes								  2 bytes
	type list offset (resource map-relative)				  2 bytes
	name list offset (resource map-relative)				  2 bytes
*/

struct FakeResourceMap
{
	struct FakeResourceMap*			nextResourceMap;
	bool							dirty;				// per-file tracking of whether FakeUpdateResFile() needs to write
	FILE*							fileDescriptor;
	int16_t							fileRefNum;
	uint16_t						resFileAttributes;
	uint16_t						numTypes;
	struct FakeTypeListEntry*		typeList;
};

/*
	Type list:
	
	number of types (-1)									  2 bytes
		resource type										  4 bytes
		number of resources (-1)							  2 bytes
		offset to reference list (type list relative)		  2 bytes
*/

struct FakeTypeListEntry
{
	uint32_t						resourceType;
	uint16_t						numberOfResourcesOfType;	// -1
	struct FakeReferenceListEntry*	resourceList;
};

/*
	Reference list:
		
	resource ID												  2 bytes
	resource name offset (relative to resource name list)	  2 bytes
	resource attributes										  1 byte
	resource data offset (resource data relative)			  3 bytes
	handle to resource in RAM								  4 bytes
*/

struct FakeReferenceListEntry
{
	int16_t				resourceID;
	uint8_t				resourceAttributes;
	Handle				resourceHandle;
	char				resourceName[257];	// 257 = 1 Pascal length byte, 255 characters for actual string, 1 byte for C terminator \0.
};

/*
	Resource names are stored as byte-counted strings. (I.e. packed P-Strings)
	Look-up by name is case-insensitive but case-preserving and diacritic-sensitive.
*/


struct FakeResourceMap	*	gResourceMap = NULL;		// Linked list.
struct FakeResourceMap	*	gCurrResourceMap = NULL;	// Start search of map here.
int16_t						gFileRefNumSeed = 0;
int16_t						gFakeResError = noErr;
struct FakeTypeCountEntry*	gLoadedTypes = NULL;
int16_t						gNumLoadedTypes = 0;


struct FakeTypeCountEntry
{
	uint32_t		type;
	int16_t			retainCount;
};


size_t	FakeFWriteUInt32BE( uint32_t inInt, FILE* theFile )
{
	inInt = BIG_ENDIAN_32(inInt);
	return fwrite( &inInt, sizeof(inInt), 1, theFile );
}


size_t	FakeFWriteInt16BE( int16_t inInt, FILE* theFile )
{
	inInt = BIG_ENDIAN_16(inInt);
	return fwrite( &inInt, sizeof(inInt), 1, theFile );
}


size_t	FakeFWriteUInt16BE( uint16_t inInt, FILE* theFile )
{
	inInt = BIG_ENDIAN_16(inInt);
	return fwrite( &inInt, sizeof(inInt), 1, theFile );
}


void	FakeFSeek( FILE* inFile, long inOffset, int inMode )
{
	int theResult = fseek( inFile,  inOffset, inMode );
}


int16_t	FakeResError()
{
	return gFakeResError;
}


// To be able to iterate types across files without duplicates, we build a list
//	of all types in open files and keep track of how many files contain each type
//	by "retaining" each type and "releasing" it when a file closes.
void	FakeRetainType( uint32_t resType )
{
	if( gLoadedTypes == NULL )
	{
		gLoadedTypes = malloc( sizeof(struct FakeTypeCountEntry) );
		gLoadedTypes[0].type = resType;
		gLoadedTypes[0].retainCount = 1;
	}
	
	for( int x = 0; x < gNumLoadedTypes; x++ )
	{
		if( gLoadedTypes[x].type == resType )
		{
			gLoadedTypes[x].retainCount++;
			return;
		}
	}
	
	gNumLoadedTypes++;
	gLoadedTypes = realloc( gLoadedTypes, gNumLoadedTypes * sizeof(struct FakeTypeCountEntry) );
	gLoadedTypes[gNumLoadedTypes -1].type = resType;
	gLoadedTypes[gNumLoadedTypes -1].retainCount = 1;
}


// The converse of FakeRetainType (see for more info):
void	FakeReleaseType( uint32_t resType )
{
	for( int x = 0; x < gNumLoadedTypes; x++ )
	{
		if( gLoadedTypes[x].type == resType )
		{
			gLoadedTypes[x].retainCount--;
			if( gLoadedTypes[x].retainCount == 0 )
			{
				gNumLoadedTypes--;
				if( gNumLoadedTypes > 0 )
					gLoadedTypes[x] = gLoadedTypes[gNumLoadedTypes];
			}
			break;
		}
	}
}


struct FakeResourceMap*	FakeFindResourceMap( int16_t inFileRefNum, struct FakeResourceMap*** outPrevMapPtr )
{
	struct FakeResourceMap*	currMap = gResourceMap;
	if( outPrevMapPtr )
		*outPrevMapPtr = &gResourceMap;
	
	while( currMap != NULL && currMap->fileRefNum != inFileRefNum )
	{
		if( outPrevMapPtr )
			*outPrevMapPtr = &currMap->nextResourceMap;
		currMap = currMap->nextResourceMap;
	}
	return currMap;
}


struct FakeResourceMap*	FakeResFileOpen( const char* inPath, const char* inMode )
{
	FILE		*			theFile = fopen( inPath, inMode );
	if( !theFile )
	{
		gFakeResError = fnfErr;
		return NULL;
	}
	
	uint32_t			resourceDataOffset = 0;
	uint32_t			resourceMapOffset = 0;
	uint32_t			lengthOfResourceData = 0;
	uint32_t			lengthOfResourceMap = 0;
	
	struct FakeResourceMap	*	newMap = calloc( 1, sizeof(struct FakeResourceMap) );
	newMap->fileDescriptor = theFile;
	newMap->fileRefNum = gFileRefNumSeed++;
	
	if( fread( &resourceDataOffset, 1, sizeof(resourceDataOffset), theFile ) != sizeof(resourceDataOffset) )
	{
		gFakeResError = eofErr;
		free( newMap );
		newMap = NULL;
		return NULL;
	}
	resourceDataOffset = BIG_ENDIAN_32(resourceDataOffset);

	if( fread( &resourceMapOffset, 1, sizeof(resourceMapOffset), theFile ) != sizeof(resourceMapOffset) )
	{
		gFakeResError = eofErr;
		free( newMap );
		newMap = NULL;
		return NULL;
	}
	resourceMapOffset = BIG_ENDIAN_32(resourceMapOffset);

	if( fread( &lengthOfResourceData, 1, sizeof(lengthOfResourceData), theFile ) != sizeof(lengthOfResourceData) )
	{
		gFakeResError = eofErr;
		free( newMap );
		newMap = NULL;
		return NULL;
	}
	lengthOfResourceData = BIG_ENDIAN_32(lengthOfResourceData);

	if( fread( &lengthOfResourceMap, 1, sizeof(lengthOfResourceMap), theFile ) != sizeof(lengthOfResourceMap) )
	{
		gFakeResError = eofErr;
		free( newMap );
		newMap = NULL;
		return NULL;
	}
	lengthOfResourceMap = BIG_ENDIAN_32(lengthOfResourceMap);
	
	FakeFSeek( theFile, 112, SEEK_CUR );	// Skip system data.
	FakeFSeek( theFile, 128, SEEK_CUR );	// Skip application data.
	
	FakeFSeek( theFile, resourceMapOffset, SEEK_SET );
	FakeFSeek( theFile, 16, SEEK_CUR );		// Skip resource file header copy.
	FakeFSeek( theFile, 4, SEEK_CUR );		// Skip next resource map placeholder.
	FakeFSeek( theFile, 2, SEEK_CUR );		// Skip file ref num placeholder.
	fread( &newMap->resFileAttributes, 1, sizeof(uint16_t), theFile );		// Read file attributes.
	newMap->resFileAttributes = BIG_ENDIAN_16(newMap->resFileAttributes);

	uint16_t		typeListOffset = 0;
	uint16_t		nameListOffset = 0;
	
	fread( &typeListOffset, 1, sizeof(typeListOffset), theFile );
	typeListOffset = BIG_ENDIAN_16(typeListOffset);
	fread( &nameListOffset, 1, sizeof(nameListOffset), theFile );
	nameListOffset = BIG_ENDIAN_16(nameListOffset);
	
	long		typeListSeekPos = resourceMapOffset +(long)typeListOffset;
	FakeFSeek( theFile, typeListSeekPos, SEEK_SET );
	
	uint16_t		numTypes = 0;
	fread( &numTypes, 1, sizeof(numTypes), theFile );
	numTypes = BIG_ENDIAN_16(numTypes) +1;

	newMap->typeList = calloc( ((int)numTypes), sizeof(struct FakeTypeListEntry) );
	newMap->numTypes = numTypes;
	for( int x = 0; x < ((int)numTypes); x++ )
	{
		uint32_t	currType = 0;
		fread( &currType, 1, sizeof(uint32_t), theFile );	// Read type code (4CC).
		char		typeStr[5] = {0};
		memmove( typeStr, &currType, 4 );
		currType = BIG_ENDIAN_32( currType );
		newMap->typeList[x].resourceType = currType;
		
		FakeRetainType( currType );
				
		uint16_t		numResources = 0;
		fread( &numResources, 1, sizeof(numResources), theFile );
		numResources = BIG_ENDIAN_16(numResources);

		uint16_t		refListOffset = 0;
		fread( &refListOffset, 1, sizeof(refListOffset), theFile );
		refListOffset = BIG_ENDIAN_16(refListOffset);
		
		long		oldOffset = ftell(theFile);
		
		long		refListSeekPos = typeListSeekPos +(long)refListOffset;
		FakeFSeek( theFile, refListSeekPos, SEEK_SET );
				
		newMap->typeList[x].resourceList = calloc( ((int)numResources) +1, sizeof(struct FakeReferenceListEntry) );
		newMap->typeList[x].numberOfResourcesOfType = ((int)numResources) +1;
		for( int y = 0; y < ((int)numResources) +1; y++ )
		{
			fread( &newMap->typeList[x].resourceList[y].resourceID, 1, sizeof(uint16_t), theFile );
			newMap->typeList[x].resourceList[y].resourceID = BIG_ENDIAN_16(newMap->typeList[x].resourceList[y].resourceID);
			
			uint16_t	nameOffset = 0;
			fread( &nameOffset, 1, sizeof(nameOffset), theFile );
			nameOffset = BIG_ENDIAN_16(nameOffset);

			unsigned char	attributesAndDataOffset[4];
			uint32_t		dataOffset = 0;
			fread( &attributesAndDataOffset, 1, sizeof(attributesAndDataOffset), theFile );
			newMap->typeList[x].resourceList[y].resourceAttributes = attributesAndDataOffset[0];
			memmove( ((char*)&dataOffset) +1, attributesAndDataOffset +1, 3 );
			dataOffset = BIG_ENDIAN_32(dataOffset);
			FakeFSeek( theFile, 4, SEEK_CUR );		// Skip resource Handle placeholder.
		
			long		innerOldOffset = ftell(theFile);
			long		dataSeekPos = resourceDataOffset +(long)dataOffset;
			FakeFSeek( theFile, dataSeekPos, SEEK_SET );
			uint32_t	dataLength = 0;
			fread( &dataLength, 1, sizeof(dataLength), theFile );
			dataLength = BIG_ENDIAN_32(dataLength);
			newMap->typeList[x].resourceList[y].resourceHandle = FakeNewHandle(dataLength);
			fread( (*newMap->typeList[x].resourceList[y].resourceHandle), 1, dataLength, theFile );
			
			if( -1 != (long)nameOffset )
			{
				long		nameSeekPos = resourceMapOffset +(long)nameListOffset +(long)nameOffset;
				FakeFSeek( theFile, nameSeekPos, SEEK_SET );
				uint8_t	nameLength = 0;
				fread( &nameLength, 1, sizeof(nameLength), theFile );
				newMap->typeList[x].resourceList[y].resourceName[0] = nameLength;
				if( nameLength > 0 )
					fread( newMap->typeList[x].resourceList[y].resourceName +1, 1, nameLength, theFile );
			}
						
			FakeFSeek( theFile, innerOldOffset, SEEK_SET );
		}
		
		FakeFSeek( theFile, oldOffset, SEEK_SET );
	}
	
	newMap->nextResourceMap = gResourceMap;
	gResourceMap = newMap;
	gFakeResError = noErr;
	
	gCurrResourceMap = gResourceMap;
	
	return newMap;
}


int16_t	FakeOpenResFile( const unsigned char* inPath )
{
#if READ_REAL_RESOURCE_FORKS
	const char*	resForkSuffix = "/..namedfork/rsrc";
#endif // READ_REAL_RESOURCE_FORKS
	char		thePath[256 +17] = {0};
	memmove(thePath,inPath +1,inPath[0]);
#if READ_REAL_RESOURCE_FORKS
	memmove(thePath +inPath[0],resForkSuffix,17);
#endif // READ_REAL_RESOURCE_FORKS
	struct FakeResourceMap*	theMap = FakeResFileOpen( thePath, "r+" );
	if( !theMap )
		theMap = FakeResFileOpen( thePath, "r" );
	if( theMap )
		return theMap->fileRefNum;
	else
		return gFakeResError;
}


static bool FakeFindResourceHandleInMap( Handle theResource, struct FakeTypeListEntry** outTypeEntry, struct FakeReferenceListEntry** outRefEntry, struct FakeResourceMap* inMap )
{
	if( (theResource != NULL) && (inMap != NULL) )
	{
		for( int x = 0; x < inMap->numTypes; x++ )
		{
			for( int y = 0; y < inMap->typeList[x].numberOfResourcesOfType; y++ )
			{
				if( inMap->typeList[x].resourceList[y].resourceHandle == theResource )
				{
					if (outTypeEntry)
					{
						*outTypeEntry = &inMap->typeList[x];
					}
					
					if (outRefEntry)
					{
						*outRefEntry = &inMap->typeList[x].resourceList[y];
					}

					return true;
				}
			}
		}
	}

	if (outTypeEntry)
	{
		*outTypeEntry = NULL;
	}
	
	if (outRefEntry)
	{
		*outRefEntry = NULL;
	}

	return false;
}


static bool FakeFindResourceHandle( Handle theResource, struct FakeResourceMap** outMap, struct FakeTypeListEntry** outTypeEntry, struct FakeReferenceListEntry** outRefEntry )
{
	struct FakeResourceMap*		currMap = gResourceMap;
	while( currMap != NULL )
	{
		if( FakeFindResourceHandleInMap(theResource, outTypeEntry, outRefEntry, currMap) )
		{
			if( outMap )
			{
				*outMap = currMap;
			}
			return true;
		}

		currMap = currMap->nextResourceMap;
	}

	if ( outMap )
	{
		*outMap = NULL;
	}
	
	if (outTypeEntry)
	{
		*outTypeEntry = NULL;
	}
	
	if (outRefEntry)
	{
		*outRefEntry = NULL;
	}

	return false;
}


static struct FakeTypeListEntry* FakeFindTypeListEntry(struct FakeResourceMap* inMap, uint32_t theType)
{
	if( inMap != NULL )
	{
		for( int x = 0; x < inMap->numTypes; x++ )
		{
			if( inMap->typeList[x].resourceType == theType )
			{
				return &inMap->typeList[x];
			}
		}
	}

	return NULL;
}

int16_t	FakeHomeResFile( Handle theResource )
{
	struct FakeResourceMap*		currMap = NULL;

	if( FakeFindResourceHandle( theResource, &currMap, NULL, NULL) )
	{
		gFakeResError = noErr;
		return currMap->fileRefNum;
	}
	
	gFakeResError = resNotFound;
	return -1;
}


void	FakeUpdateResFile( int16_t inFileRefNum )
{
	const long kResourceHeaderLength            = 16;
	const long kResourceHeaderMapOffsetPos      = 4;
	const long kResourceHeaderMapLengthPos      = 4 + 4 + 4;
	const long kResourceHeaderReservedLength    = 112;
	const long kResourceHeaderAppReservedLength = 128;
	const long kReservedHeaderLength            = kResourceHeaderReservedLength + kResourceHeaderAppReservedLength;
	const long kResourceDataSizeLength          = 4;
	const long kResourceMapNextHandleLength     = 4;
	const long kResourceMapFileRefLength        = 2;
	const long kResourceMapTypeListOffsetLength = 2;
	const long kResourceMapNameListOffsetLength = 2;
	const long kResourceMapNumTypesLength       = 2;
	const long kResourceRefLength               = 2 + 2 + 1 + 3 + 4;
	const long kResourceTypeLength              = 4 + 2 + 2;

	struct FakeResourceMap*		currMap = FakeFindResourceMap( inFileRefNum, NULL );
	long						headerLength = kResourceHeaderLength + kReservedHeaderLength;
	uint32_t					resMapOffset = 0;
	long						refListSize = 0;
	
	if (!currMap->dirty)
		return;

	// Write header:
	FakeFSeek( currMap->fileDescriptor, 0, SEEK_SET );
	uint32_t    resDataOffset = (uint32_t)headerLength;
	FakeFWriteUInt32BE( resDataOffset, currMap->fileDescriptor );
	FakeFWriteUInt32BE( 0, currMap->fileDescriptor );               // placeholder offset to resource map
	FakeFWriteUInt32BE( 0, currMap->fileDescriptor );               // placeholder resource data length
	FakeFWriteUInt32BE( 0, currMap->fileDescriptor );               // placeholder resource map length

	// reserved
	for( int x = 0; x < (kResourceHeaderReservedLength / sizeof(uint32_t)); x++ )
		FakeFWriteUInt32BE( 0, currMap->fileDescriptor );
	for( int x = 0; x < (kResourceHeaderAppReservedLength / sizeof(uint32_t)); x++ )
		FakeFWriteUInt32BE( 0, currMap->fileDescriptor );
	
	resMapOffset = (uint32_t)headerLength;
	
	// Write out data for each resource and calculate space needed:
	for( int x = 0; x < currMap->numTypes; x++ )
	{
		for( int y = 0; y < currMap->typeList[x].numberOfResourcesOfType; y++ )
		{
			uint32_t	theSize = (uint32_t)FakeGetHandleSize( currMap->typeList[x].resourceList[y].resourceHandle );
			FakeFWriteUInt32BE( theSize, currMap->fileDescriptor );
			resMapOffset += sizeof(theSize);
			fwrite( *currMap->typeList[x].resourceList[y].resourceHandle, 1, theSize, currMap->fileDescriptor );
			resMapOffset += theSize;
		}

		refListSize += currMap->typeList[x].numberOfResourcesOfType * kResourceRefLength;
	}
	
	// Write out what we know into the header now:
	FakeFSeek( currMap->fileDescriptor, kResourceHeaderMapOffsetPos, SEEK_SET );
	FakeFWriteUInt32BE( resMapOffset, currMap->fileDescriptor );
	uint32_t	resDataLength = resMapOffset -(uint32_t)headerLength;
	FakeFWriteUInt32BE( resDataLength, currMap->fileDescriptor );
	
	// Start writing resource map after data:
	uint32_t		resMapLength = 0;
	FakeFSeek( currMap->fileDescriptor, resMapOffset, SEEK_SET );

    // Copy what we know from the resource header
    FakeFWriteUInt32BE( resDataOffset, currMap->fileDescriptor );
    FakeFWriteUInt32BE( resMapOffset, currMap->fileDescriptor );
    FakeFWriteUInt32BE( resDataLength, currMap->fileDescriptor );
    FakeFWriteUInt32BE( 0, currMap->fileDescriptor ); // Placeholder

    // Fake a next handle
    FakeFWriteUInt32BE( 0, currMap->fileDescriptor );
	
	resMapLength += kResourceHeaderLength + kResourceMapNextHandleLength + kResourceMapFileRefLength;   // reserved: copy of resource header, next resource handle, file ref
    FakeFWriteInt16BE( inFileRefNum, currMap->fileDescriptor );
	FakeFWriteUInt16BE( currMap->resFileAttributes, currMap->fileDescriptor );
	resMapLength += sizeof(uint16_t);
	
	uint16_t		typeListOffset = ftell(currMap->fileDescriptor)
										+kResourceMapTypeListOffsetLength +kResourceMapNameListOffsetLength -resMapOffset;
	FakeFWriteUInt16BE( typeListOffset, currMap->fileDescriptor );	// Res map relative, points to the type count
	long			refListStartPosition = typeListOffset + kResourceMapNumTypesLength + currMap->numTypes * kResourceTypeLength;	// Calc where we'll start to put resource lists (right after types).

	uint16_t		nameListOffset = refListStartPosition +refListSize;		// Calc where we'll start to put name lists (right after resource lists).
	FakeFWriteUInt16BE( nameListOffset, currMap->fileDescriptor );	// Res map relative.

	// Now write type list and ref lists:
	uint32_t		nameListStartOffset = 0;
	FakeFWriteUInt16BE( currMap->numTypes -1, currMap->fileDescriptor );
	uint32_t		resDataCurrOffset = 0;		// Keep track of where we wrote the associated data for each resource, relative to the start of resource data
	
	refListStartPosition = kResourceMapNumTypesLength + currMap->numTypes * kResourceTypeLength; // relative to beginning of resource type list

	for( int x = 0; x < currMap->numTypes; x++ )
	{
		// Write entry for this type:
		uint32_t	currType = currMap->typeList[x].resourceType;
		FakeFWriteUInt32BE( currType, currMap->fileDescriptor );
		
		uint16_t	numResources = currMap->typeList[x].numberOfResourcesOfType -1;
		FakeFWriteUInt16BE( numResources, currMap->fileDescriptor );
		
		uint16_t	refListOffset = refListStartPosition;
		FakeFWriteUInt16BE( refListOffset, currMap->fileDescriptor );
		
		// Jump to ref list location and write ref list out:
		long		oldOffsetAfterPrevType = ftell(currMap->fileDescriptor);
		
		FakeFSeek( currMap->fileDescriptor, resMapOffset + typeListOffset + refListStartPosition, SEEK_SET );

		for( int y = 0; y < currMap->typeList[x].numberOfResourcesOfType; y++ )
		{
			FakeFWriteInt16BE( currMap->typeList[x].resourceList[y].resourceID, currMap->fileDescriptor );
			
			// Write name to name table:
			if( currMap->typeList[x].resourceList[y].resourceName[0] == 0 )
				FakeFWriteInt16BE( -1, currMap->fileDescriptor );	// Don't have a name, mark as -1.
			else
			{
				FakeFWriteUInt16BE( nameListStartOffset, currMap->fileDescriptor );	// Associate name in name table with this.
				
				long oldOffsetAfterNameOffset = ftell( currMap->fileDescriptor );
				FakeFSeek( currMap->fileDescriptor, resMapOffset +nameListOffset +nameListStartOffset, SEEK_SET );
				fwrite( currMap->typeList[x].resourceList[y].resourceName, currMap->typeList[x].resourceList[y].resourceName[0] +1, sizeof(uint8_t), currMap->fileDescriptor );

				long	currMapLen = (ftell(currMap->fileDescriptor) -resMapOffset);
				if( currMapLen > resMapLength )
					resMapLength = (uint32_t)currMapLen;

				FakeFSeek( currMap->fileDescriptor, oldOffsetAfterNameOffset, SEEK_SET );
				nameListStartOffset += currMap->typeList[x].resourceList[y].resourceName[0] +1;	// Make sure we write next name *after* this one.
			}
			
			fwrite( &currMap->typeList[x].resourceList[y].resourceAttributes, 1, sizeof(uint8_t), currMap->fileDescriptor );
			uint32_t	resDataCurrOffsetBE = BIG_ENDIAN_32(resDataCurrOffset);
			fwrite( ((uint8_t*)&resDataCurrOffsetBE) +1, 1, 3, currMap->fileDescriptor );
			resDataCurrOffset += kResourceDataSizeLength + FakeGetHandleSize(currMap->typeList[x].resourceList[y].resourceHandle);
			FakeFWriteUInt32BE( 0, currMap->fileDescriptor );	// Handle placeholder.
			
			long	currMapLen = (ftell(currMap->fileDescriptor) -resMapOffset);
			if( currMapLen > resMapLength )
				resMapLength = (uint32_t)currMapLen;
		}
		
		refListStartPosition += currMap->typeList[x].numberOfResourcesOfType * kResourceRefLength;
		
		// Jump back to after our type entry so we can write the next one:
		FakeFSeek( currMap->fileDescriptor, oldOffsetAfterPrevType, SEEK_SET );
	}

	// Write res map length:
	FakeFSeek( currMap->fileDescriptor, kResourceHeaderMapLengthPos, SEEK_SET );
	FakeFWriteUInt32BE( resMapLength, currMap->fileDescriptor );
    FakeFSeek( currMap->fileDescriptor, resMapOffset + kResourceHeaderMapLengthPos, SEEK_SET );
    FakeFWriteUInt32BE( resMapLength, currMap->fileDescriptor );
	
	ftruncate(fileno(currMap->fileDescriptor), resMapOffset + resMapLength);
	
	currMap->dirty = false;
}


void	FakeRedirectResFileToPath( int16_t inFileRefNum, const char* cPath )
{
	struct FakeResourceMap**	prevMapPtr = NULL;
	struct FakeResourceMap*		currMap = FakeFindResourceMap( inFileRefNum, &prevMapPtr );
	if( currMap )
	{
		fclose( currMap->fileDescriptor );
		currMap->fileDescriptor = fopen( cPath, "w" );
		currMap->dirty = true;
	}
}


void	FakeCloseResFile( int16_t inFileRefNum )
{
	struct FakeResourceMap**	prevMapPtr = NULL;
	struct FakeResourceMap*		currMap = FakeFindResourceMap( inFileRefNum, &prevMapPtr );
	if( currMap )
	{
		FakeUpdateResFile(inFileRefNum);
		
		*prevMapPtr = currMap->nextResourceMap;	// Remove this from the linked list.
		if( gCurrResourceMap == currMap )
			gCurrResourceMap = currMap->nextResourceMap;
		
		for( int x = 0; x < currMap->numTypes; x++ )
		{
			FakeReleaseType( currMap->typeList[x].resourceType );
			
			for( int y = 0; y < currMap->typeList[x].numberOfResourcesOfType; y++ )
			{
				FakeDisposeHandle( currMap->typeList[x].resourceList[y].resourceHandle );
			}
			free( currMap->typeList[x].resourceList );
		}
		free( currMap->typeList );
		
		fclose( currMap->fileDescriptor );
		free( currMap );
	}
}


Handle	FakeGet1ResourceFromMap( uint32_t resType, int16_t resID, struct FakeResourceMap* inMap )
{
	gFakeResError = noErr;
	
	if( inMap != NULL )
	{
		for( int x = 0; x < inMap->numTypes; x++ )
		{
			uint32_t		currType = inMap->typeList[x].resourceType;
			if( currType == resType )
			{
				for( int y = 0; y < inMap->typeList[x].numberOfResourcesOfType; y++ )
				{
					if( inMap->typeList[x].resourceList[y].resourceID == resID )
					{
						return inMap->typeList[x].resourceList[y].resourceHandle;
					}
				}
				break;
			}
		}
	}
	
	gFakeResError = resNotFound;
	
	return NULL;
}


Handle	FakeGet1Resource( uint32_t resType, int16_t resID )
{
	return FakeGet1ResourceFromMap( resType, resID, gCurrResourceMap );
}


Handle	FakeGetResource( uint32_t resType, int16_t resID )
{
	struct FakeResourceMap *	currMap = gCurrResourceMap;
	Handle						theRes = NULL;
	
	while( theRes == NULL && currMap != NULL )
	{
		theRes = FakeGet1ResourceFromMap( resType, resID, currMap );
		if( theRes != NULL )
		{
			gFakeResError = noErr;
			return theRes;
		}
		
		currMap	= currMap->nextResourceMap;
	}
	
	gFakeResError = resNotFound;
	
	return NULL;
}


int16_t	FakeCount1ResourcesInMap( uint32_t resType, struct FakeResourceMap* inMap )
{
	gFakeResError = noErr;
	
	if( inMap != NULL )
	{
		for( int x = 0; x < inMap->numTypes; x++ )
		{
			uint32_t		currType = inMap->typeList[x].resourceType;
			if( currType == resType )
				return inMap->typeList[x].numberOfResourcesOfType;
		}
	}
	
	return 0;
}


int16_t	FakeCount1TypesInMap( struct FakeResourceMap* inMap )
{
	if( inMap == NULL )
		return 0;
	
	return inMap->numTypes;
}


int16_t	FakeCount1Types()
{
	return FakeCount1TypesInMap( gCurrResourceMap );
}


int16_t	FakeCount1Resources( uint32_t resType )
{
	return FakeCount1ResourcesInMap( resType, gCurrResourceMap );
}


int16_t	FakeCountResources( uint32_t resType )
{
	int16_t						numRes = 0;
	struct FakeResourceMap* 	theMap = gCurrResourceMap;
	
	while( theMap )
	{
		numRes += FakeCount1ResourcesInMap( resType, theMap );
		
		theMap = theMap->nextResourceMap;
	}
	
	return numRes;
}


int16_t	FakeCountTypes()
{
	return gNumLoadedTypes;
}


int16_t FakeCurResFile()
{
	struct FakeResourceMap* currMap = gCurrResourceMap;

	if( !currMap )
		return 0;
	
	return currMap->fileRefNum;
}

void	FakeUseResFile( int16_t resRefNum )
{
	struct FakeResourceMap*	currMap = FakeFindResourceMap( resRefNum, NULL );
	if( !currMap )
		currMap = gResourceMap;
	
	gCurrResourceMap = currMap;
}


void FakeGet1IndType( uint32_t * resType, int16_t index )
{
	if( resType == NULL )
		return;

	*resType = 0;
	
	struct FakeResourceMap* currMap = gCurrResourceMap;
	if( (index <= 0) || (index > FakeCount1Types()) || !currMap )
	{
		gFakeResError = resNotFound;
		return;
	}

	*resType = currMap->typeList[index-1].resourceType;
	
	gFakeResError = noErr;
}

Handle FakeGet1IndResource( uint32_t resType, int16_t index )
{
	struct FakeResourceMap* currMap = gCurrResourceMap;

	if( !currMap || (index <= 0) || (index > FakeCount1Resources(resType)))
	{
		gFakeResError = resNotFound;
	}

	for( int x = 0; x < currMap->numTypes; x++ )
	{
		uint32_t		currType = currMap->typeList[x].resourceType;
		if( currType == resType )
		{
			gFakeResError = noErr;
			return currMap->typeList[x].resourceList[index-1].resourceHandle;
		}
	}
	
	gFakeResError = resNotFound;
	
	return NULL;
}

void FakeGetResInfo( Handle theResource, int16_t * theID, uint32_t * theType, FakeStr255 * name )
{
	struct FakeTypeListEntry*   typeEntry = NULL;
	struct FakeReferenceListEntry* refEntry = NULL;


	if( FakeFindResourceHandle(theResource, NULL, &typeEntry, &refEntry) )
	{
		gFakeResError = noErr;
		if( theID )
		{
			*theID = refEntry->resourceID;
		}
		
		if( theType )
		{
			*theType = typeEntry->resourceType;
		}
		
		if( name )
		{
			memcpy(name, refEntry->resourceName, sizeof(FakeStr255));
		}
		return;
	}
	
	gFakeResError = resNotFound;
}


void FakeSetResInfo( Handle theResource, int16_t theID, FakeStr255 name )
{
	struct FakeReferenceListEntry* refEntry = NULL;

	if( !theResource || !FakeFindResourceHandle( theResource, NULL, NULL, &refEntry) )
	{
		gFakeResError = resNotFound;
		return;
	}

	if( (refEntry->resourceAttributes & resProtected) != 0 )
	{
		gFakeResError = resAttrErr;
		return;
	}

	refEntry->resourceID = theID;
	memcpy(refEntry->resourceName, name, sizeof(FakeStr255));

	gFakeResError = noErr;
}


void FakeAddResource( Handle theData, uint32_t theType, int16_t theID, FakeStr255 name )
{
	struct FakeResourceMap* currMap = gCurrResourceMap;
	struct FakeTypeListEntry* typeEntry = NULL;
	struct FakeReferenceListEntry* resourceEntry = NULL;

	// AddResource() only ensures that the handle is not a resource, but doesn't check whether the type/ID are already in use
	if( !theData || FakeFindResourceHandleInMap( theData, &typeEntry, &resourceEntry, currMap ) )
	{
		gFakeResError = addResFailed;
		return;
	}

	typeEntry = FakeFindTypeListEntry( currMap, theType );
	if( !typeEntry )
	{
		currMap->numTypes++;
		currMap->typeList = realloc(currMap->typeList, currMap->numTypes * sizeof(struct FakeTypeListEntry));

		typeEntry = currMap->typeList + (currMap->numTypes - 1);
		typeEntry->resourceType = theType;
		typeEntry->numberOfResourcesOfType = 0;
		typeEntry->resourceList = NULL;
		
		FakeRetainType(theType);
	}

	typeEntry->numberOfResourcesOfType++;

	if( typeEntry->resourceList )
	{
		typeEntry->resourceList = realloc( typeEntry->resourceList, typeEntry->numberOfResourcesOfType * sizeof(struct FakeReferenceListEntry) );
	}
	else
	{
		typeEntry->resourceList = calloc(typeEntry->numberOfResourcesOfType, sizeof(struct FakeReferenceListEntry));
	}
	
	resourceEntry = typeEntry->resourceList + ( typeEntry->numberOfResourcesOfType - 1 );
	resourceEntry->resourceAttributes = 0;
	resourceEntry->resourceID = theID;
	memcpy(resourceEntry->resourceName, name, sizeof(FakeStr255));
	resourceEntry->resourceHandle = theData;

	currMap->dirty = true;

	gFakeResError = noErr;
}

void FakeChangedResource( Handle theResource )
{
	struct FakeResourceMap* theMap = NULL;
	struct FakeReferenceListEntry* theEntry = NULL;
	if( !FakeFindResourceHandle( theResource, &theMap, NULL, &theEntry ) )
	{
		gFakeResError = resNotFound;
		return;
	}

	if( (theEntry->resourceAttributes & resProtected) == 0 )
	{
		theMap->dirty = true;
		gFakeResError = noErr;
	}
	else
	{
		gFakeResError = resAttrErr;
	}
}

// NOTE: you must call DisposeHandle(theResource) manually to release the memory.  Normally,
//       the Resource Manager will dispose the handle on update or file close, but this implementation
//       does not track removed resource handles for later disposal.
void FakeRemoveResource( Handle theResource )
{
	struct FakeResourceMap* currMap = gCurrResourceMap;
	struct FakeTypeListEntry* typeEntry = NULL;
	struct FakeReferenceListEntry* resEntry = NULL;
	if( !currMap || !FakeFindResourceHandleInMap( theResource, &typeEntry, &resEntry, currMap ) || ((resEntry->resourceAttributes & resProtected) != 0) )
	{
		gFakeResError = rmvResFailed;
		return;
	}
	
	struct FakeReferenceListEntry* nextResEntry = resEntry + 1;
	int resourcesListSize = typeEntry->numberOfResourcesOfType * sizeof(struct FakeReferenceListEntry);
	long nextResEntryOffset   = (void*)nextResEntry - (void*)typeEntry->resourceList;
	
	typeEntry->numberOfResourcesOfType--;

	if( typeEntry->numberOfResourcesOfType > 0 )
	{
		memcpy( resEntry, nextResEntry, resourcesListSize - nextResEntryOffset );
		typeEntry->resourceList = realloc( typeEntry->resourceList, resourcesListSize - sizeof(struct FakeReferenceListEntry) );
	}
	else
	{
		// got rid of the last resource reference, release the memory
		free(typeEntry->resourceList);
		typeEntry->resourceList = NULL;

		// now remove the type entry
		struct FakeTypeListEntry* nextTypeEntry = typeEntry + 1;
		int typeListSize = currMap->numTypes * sizeof(struct FakeTypeListEntry);
		long nextTypeEntryOffset   = (void*)nextTypeEntry - (void*)currMap->typeList;

		currMap->numTypes--;
		FakeReleaseType(typeEntry->resourceType);

		if( currMap->numTypes > 0 )
		{
			memcpy( typeEntry, nextTypeEntry, typeListSize - nextTypeEntryOffset );
			currMap->typeList = realloc( currMap->typeList, typeListSize - sizeof(struct FakeTypeListEntry) );
		}
		else
		{
			// got rid of the last type entry
			free(currMap->typeList);
			currMap->typeList = NULL;
		}
	}
	

	currMap->dirty = true;
	gFakeResError = noErr;
}


// NOTE: effectively a no-op since we don't have a way to write a resource without writing the whole map
void FakeWriteResource( Handle theResource )
{
	struct FakeReferenceListEntry* resEntry = NULL;
	if( !theResource || !FakeFindResourceHandle( theResource, NULL, NULL, &resEntry ))
	{
		gFakeResError = resNotFound;
	}
	else
	{
		gFakeResError = noErr;
	}
}

// NOTE: effectively a no-op since we don't have a way to load an individual resource from disk right now.
//       All resources are already loaded at file open time.
void FakeLoadResource( Handle theResource )
{
	struct FakeReferenceListEntry* resEntry = NULL;
	if( !theResource || !FakeFindResourceHandle( theResource, NULL, NULL, &resEntry ))
	{
		gFakeResError = resNotFound;
	}
	else
	{
		gFakeResError = noErr;
	}
}

// NOTE: effectively a no-op since we don't have a way to reload a released resource from disk right now
void FakeReleaseResource( Handle theResource )
{
	struct FakeReferenceListEntry* resEntry = NULL;
	if( !theResource || !FakeFindResourceHandle( theResource, NULL, NULL, &resEntry ))
	{
		gFakeResError = resNotFound;
	}
	else
	{
		gFakeResError = noErr;
	}
}


void FakeSetResLoad(bool load)
{
	// NOTE: a no-op since resources are always loaded at file open time
}



