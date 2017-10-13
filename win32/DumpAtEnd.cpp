// uncomment to find memory leaks at end
// #define CHECK_MEMORY_LEAKS

// to also display file locations in some cases define _CRTDBG_MAP_ALLOC in the project properties

#ifdef CHECK_MEMORY_LEAKS
// hack to make sure this object is destructed last
// msvc merges .CRT$XC in alphabetic order
// might not work in other versions of MSVC
#pragma init_seg(".CRT$XCB")
struct CallDumpMemLeaksLast {
	~CallDumpMemLeaksLast() {
		_CrtDumpMemoryLeaks();
	}
};
static CallDumpMemLeaksLast dump_last;
#endif