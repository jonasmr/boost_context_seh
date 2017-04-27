#include <windows.h>
#include <stdio.h>
#include <conio.h>

typedef void*   FContext;
struct Transfer {
    FContext  fctx;
    void    *   data;
};

extern "C"
Transfer  jump_fcontext( FContext const to, void * vp);
extern "C" 
FContext  make_fcontext( void * sp, size_t size, void (* fn)( Transfer) );



LONG WINAPI XptHandler(struct _EXCEPTION_POINTERS* pInfo)
{
	printf("ExceptionHandler\n");
	return EXCEPTION_EXECUTE_HANDLER;
}

void Crash()
{
	intptr_t* p = 0;
	*p = 32;
}

void TestCrash(Transfer T)
{
	Crash();
	jump_fcontext(T.fctx, (void*)42);
}


void Test(Transfer T)
{
	printf("Test\n");
	jump_fcontext(T.fctx, (void*)42);
}

typedef unsigned long long uint64_t;
int main()
{
	
	SetUnhandledExceptionFilter(XptHandler);


	printf("Start\n");
	void* pStack = _aligned_malloc((32<<10), 128);
	memset(pStack, 0xfb, (32<<10));

#if 1 //Case 0: Verify fcontext usage is sane
	FContext Context = make_fcontext((void*)pStack, (32<<10), Test);
	jump_fcontext(Context, 0);
#endif

#if 0 //Case 1: Verify crash handler works.
	Crash();
#endif 

#if 1 //Case 2: Crash inside function called through fcontext. XptHandler is not called.
	FContext ContextCrash = make_fcontext((void*)pStack, (32<<10), TestCrash);
	jump_fcontext(ContextCrash, 0);
#endif
	printf("End\n");
	_aligned_free(pStack);
	_getch();
}