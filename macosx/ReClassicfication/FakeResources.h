//
//  FakeResources.h
//  ReClassicfication
//
//  Created by Uli Kusterer on 21.02.13.
//  Copyright (c) 2013 Uli Kusterer. All rights reserved.
//

#ifndef ReClassicfication_FakeResources_h
#define ReClassicfication_FakeResources_h


#include "FakeHandles.h"


// Possible return values of FakeResError():
#ifndef __MACERRORS__
enum
{
	resNotFound		= -192,
	resFNotFound	= -193,
	addResFailed	= -194,
	rmvResFailed	= -196,
	resAttrErr		= -198,
	eofErr			= -39,
	fnfErr			= -43
};
#endif /* __MACERRORS__ */


#ifndef __RESOURCES__
// Resource attribute bit flags:
enum
{
	resReserved		= (1 << 0),	// Apparently not yet used.
	resChanged		= (1 << 1),
	resPreload		= (1 << 2),
	resProtected	= (1 << 3),
	resLocked		= (1 << 4),
	resPurgeable	= (1 << 5),
	resSysHeap		= (1 << 6),
	resReserved2	= (1 << 7)	// Apparently not yet used.
};
#endif

typedef unsigned char FakeStr255[256];


int16_t	FakeOpenResFile( const unsigned char* inPath );
void	FakeCloseResFile( int16_t resRefNum );
Handle	FakeGet1Resource( uint32_t resType, int16_t resID );
Handle	FakeGetResource( uint32_t resType, int16_t resID );
int16_t FakeCurResFile();
void	FakeUseResFile( int16_t resRefNum );
void	FakeUpdateResFile( int16_t inFileRefNum );
int16_t	FakeHomeResFile( Handle theResource );
int16_t	FakeCount1Types();
int16_t	FakeCount1Resources( uint32_t resType );
int16_t	FakeCountTypes();
int16_t	FakeCountResources( uint32_t resType );

void FakeGet1IndType( uint32_t * resType, int16_t index );
Handle FakeGet1IndResource( uint32_t resType, int16_t index );
void FakeGetResInfo( Handle theResource, int16_t * theID, uint32_t * theType, FakeStr255 * name );
void FakeSetResInfo( Handle theResource, int16_t theID, FakeStr255 name );
void FakeAddResource( Handle theData, uint32_t theType, int16_t theID, FakeStr255 name );
void FakeChangedResource( Handle theResource );
void FakeRemoveResource( Handle theResource );
void FakeWriteResource( Handle theResource );
void FakeLoadResource( Handle theResource );
void FakeReleaseResource( Handle theResource );
void FakeSetResLoad(bool load);

int16_t	FakeResError();


// Private calls for internal use/tests:
void					FakeRedirectResFileToPath( int16_t inFileRefNum, const char* cPath );
struct FakeResourceMap*	FakeResFileOpen( const char* inPath, const char* inMode );
struct FakeResourceMap*	FakeFindResourceMap( int16_t inFileRefNum, struct FakeResourceMap*** outPrevMapPtr );
int16_t					FakeCount1ResourcesInMap( uint32_t resType, struct FakeResourceMap* inMap );
int16_t					FakeCount1TypesInMap( struct FakeResourceMap* inMap );

#endif
