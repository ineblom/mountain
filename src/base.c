#define Align(bytes) __attribute__((aligned(bytes)))
#define Expect(x, y) __builtin_expect(x, y)
#define Inline       static inline __attribute__((always_inline))
#define NoInline     static __attribute__((noinline))
#define Internal     static
#define Global       static
#define LocalPersist static
#define Restrict     __restrict
#define Volatile     volatile
#define ThreadLocal  _Thread_local
#define W_           __attribute__((__stdcall__)) __attribute__((__force_align_arg_pointer__))

typedef float     F1;
typedef double    D1;
typedef char      SB1; typedef unsigned char      B1;
typedef short     SW1; typedef unsigned short     W1;
typedef int       SI1; typedef unsigned int       I1;
typedef long long SL1; typedef unsigned long long L1;
typedef float     F3 __attribute__((ext_vector_type(3)));
typedef float     F4 __attribute__((ext_vector_type(4)));

typedef void VoidProc(void);

#define F1_MAX 3.4028234664e+38
#define F1_MIN 1.1754943508e-38
#define I1_MAX 0xFFFFFFFF
#define L1_MAX 0xFFFFFFFFFFFFFFFF

#define F1_(x)  ((F1) (x))
#define D1_(x)  ((D1) (x))
#define SB1_(x) ((SB1)(x))
#define B1_(x)  ((B1) (x))
#define SW1_(x) ((SW1)(x))
#define W1_(x)  ((W1) (x))
#define SI1_(x) ((SI1)(x))
#define I1_(x)  ((I1) (x))
#define SL1_(x) ((SL1)(x))
#define L1_(x)  ((L1) (x))

Inline I1 GeSI1(I1 a, I1 b) { return SI1_(a) >= SI1_(b); }
Inline I1 GeSL1(L1 a, L1 b) { return SL1_(a) >= SL1_(b); }
Inline I1 GtSI1(I1 a, I1 b) { return SI1_(a) >  SI1_(b); }
Inline I1 GtSL1(L1 a, L1 b) { return SL1_(a) >  SL1_(b); }
Inline I1 LeSI1(I1 a, I1 b) { return SI1_(a) <= SI1_(b); }
Inline I1 LeSL1(L1 a, L1 b) { return SL1_(a) <= SL1_(b); }
Inline I1 LtSI1(I1 a, I1 b) { return SI1_(a) <  SI1_(b); }
Inline I1 LtSL1(L1 a, L1 b) { return SL1_(a) <  SL1_(b); }

Inline void BarC(void) { asm volatile("":::"memory"); } // Complier Barrier
Inline void BarM(void) { __builtin_ia32_mfence(); }     // Memory Barrier
Inline void BarR(void) { __builtin_ia32_lfence();}      // Read Barrier
Inline void BarW(void) { __builtin_ia32_sfence();}      // Write Barrier

/*Inline I1 ClockI1(void) {
	I1 aa, dd;
	asm volatile("rdtsc":"=a"(aa),"=d"(dd));
	return aa;
}
Inline L1 ClockL1(void) {
	I1 aa, dd;
	asm volatile("rdtsc":"=a"(aa),"=d"(dd));
	return (L1_(dd) << 32) | L1_(aa);
}*/
Inline void Pause(void) { asm volatile("pause":::"memory"); }

Inline I1 atomic_add_I1(I1 *a, I1 v) {
	asm volatile("lock xaddl %0,%1"
							 : "=r"(v),"=m"(*a)
							 : "0"(v),"m"(*a)
							 : "memory","cc");
	return v;
}
Inline L1 atomic_add_L1(L1 *a, L1 v) {
	asm volatile("lock xaddq %0,%1"
							 : "=r"(v),"=m"(*a)
							 : "0"(v),"m"(*a)
							 : "memory","cc");
	return v;
}
Inline I1 atomic_swap_I1(I1 *a, I1 v) {
	asm volatile("xchgl %0,%1"
							 : "=r"(v),"=m"(*a)
							 : "0"(v),"m"(*a)
							 : "memory");
	return v;
}
Inline L1 atomic_swap_L1(L1 *a, L1 v) {
	asm volatile("xchgq %0,%1"
							 : "=r"(v),"=m"(*a)
							 : "0"(v),"m"(*a)
							 : "memory");
	return v;
}

#define EachIndex(it, count) (L1 it = 0; it < (count); it += 1)
#define EachInRange(it, range) (L1 it = (range).min; it < (range).max; it += 1)

#define KiB(x) (1024 * (x))
#define MiB(x) (1024 * 1024 * (x))
#define GiB(x) (1024 * 1024 * 1024 * (x))

#define ArrayCount(arr) (sizeof(arr) / sizeof(arr[0]))

#define Min(a, b) ((a) < (b) ? (a) : (b))
#define Max(a, b) ((a) > (b) ? (a) : (b))

#define Crash(x) __builtin_trap()
#define StaticAssert(x) _Static_assert(x, "")
#define Assert(x) do { if (!(x)) {\
	fprintf(stderr, "Assert Failed (%s:%d) %s\n", __FILE__, __LINE__, #x);\
	fflush(stderr); \
	Crash(-1); \
	} } while (0)

#define Swap(a, b) do { typeof(a) temp = a; a = b; b = temp; } while(0)

#define AlignOf(T) __alignof(T)
#define AlignPow2(x,b) (((x) + (b) - 1)&(~((b) - 1)))

////////////////////////////////
//~ kti: Memory
#define MemoryZero(p, s) memset((p), 0, (s))
#define MemoryZeroStruct(p) MemoryZero((p), sizeof(*(p)))

////////////////////////////////
//~ kti: Linked List Building Macros

//- kti: linked list macro helpers
#define CheckNil(nil,p) ((p) == 0 || (p) == nil)
#define SetNil(nil,p) ((p) = nil)

//- kti: doubly-linked-lists
#define DLLInsert_NPZ(nil,f,l,p,n,next,prev) (CheckNil(nil,f) ? \
	((f) = (l) = (n), SetNil(nil,(n)->next), SetNil(nil,(n)->prev)) :\
	CheckNil(nil,p) ? \
	((n)->next = (f), (f)->prev = (n), (f) = (n), SetNil(nil,(n)->prev)) :\
	((p)==(l)) ? \
	((l)->next = (n), (n)->prev = (l), (l) = (n), SetNil(nil, (n)->next)) :\
	(((!CheckNil(nil,p) && CheckNil(nil,(p)->next)) ? (0) : ((p)->next->prev = (n))), ((n)->next = (p)->next), ((p)->next = (n)), ((n)->prev = (p))))
#define DLLPushBack_NPZ(nil,f,l,n,next,prev) DLLInsert_NPZ(nil,f,l,l,n,next,prev)
#define DLLPushFront_NPZ(nil,f,l,n,next,prev) DLLInsert_NPZ(nil,l,f,f,n,prev,next)
#define DLLRemove_NPZ(nil,f,l,n,next,prev) (((n) == (f) ? (f) = (n)->next : (0)),\
	((n) == (l) ? (l) = (l)->prev : (0)),\
	(CheckNil(nil,(n)->prev) ? (0) :\
	((n)->prev->next = (n)->next)),\
	(CheckNil(nil,(n)->next) ? (0) :\
	((n)->next->prev = (n)->prev)))

//- kti: singly-linked, doubly-headed lists (queues)
#define SLLQueuePush_NZ(nil,f,l,n,next) (CheckNil(nil,f)?\
	((f)=(l)=(n),SetNil(nil,(n)->next)):\
	((l)->next=(n),(l)=(n),SetNil(nil,(n)->next)))
#define SLLQueuePushFront_NZ(nil,f,l,n,next) (CheckNil(nil,f)?\
((f)=(l)=(n),SetNil(nil,(n)->next)):\
((n)->next=(f),(f)=(n)))
#define SLLQueuePop_NZ(nil,f,l,next) ((f)==(l)?\
(SetNil(nil,f),SetNil(nil,l)):\
((f)=(f)->next))

//- kti: singly-linked, singly-headed lists (stacks)
#define SLLStackPush_N(f,n,next) ((n)->next=(f), (f)=(n))
#define SLLStackPop_N(f,next) ((f)=(f)->next)

//- kti: doubly-linked-list helpers
#define DLLInsert_NP(f,l,p,n,next,prev) DLLInsert_NPZ(0,f,l,p,n,next,prev)
#define DLLPushBack_NP(f,l,n,next,prev) DLLPushBack_NPZ(0,f,l,n,next,prev)
#define DLLPushFront_NP(f,l,n,next,prev) DLLPushFront_NPZ(0,f,l,n,next,prev)
#define DLLRemove_NP(f,l,n,next,prev) DLLRemove_NPZ(0,f,l,n,next,prev)
#define DLLInsert(f,l,p,n) DLLInsert_NPZ(0,f,l,p,n,next,prev)
#define DLLPushBack(f,l,n) DLLPushBack_NPZ(0,f,l,n,next,prev)
#define DLLPushFront(f,l,n) DLLPushFront_NPZ(0,f,l,n,next,prev)
#define DLLRemove(f,l,n) DLLRemove_NPZ(0,f,l,n,next,prev)

//- kti: singly-linked, doubly-headed list helpers
#define SLLQueuePush_N(f,l,n,next) SLLQueuePush_NZ(0,f,l,n,next)
#define SLLQueuePushFront_N(f,l,n,next) SLLQueuePushFront_NZ(0,f,l,n,next)
#define SLLQueuePop_N(f,l,next) SLLQueuePop_NZ(0,f,l,next)
#define SLLQueuePush(f,l,n) SLLQueuePush_NZ(0,f,l,n,next)
#define SLLQueuePushFront(f,l,n) SLLQueuePushFront_NZ(0,f,l,n,next)
#define SLLQueuePop(f,l) SLLQueuePop_NZ(0,f,l,next)

//- kti: singly-linked, singly-headed list helpers
#define SLLStackPush(f,n) SLLStackPush_N(f,n,next)
#define SLLStackPop(f) SLLStackPop_N(f,next)

Global const I1 bitmask1  = 0x00000001;
Global const I1 bitmask2  = 0x00000003;
Global const I1 bitmask3  = 0x00000007;
Global const I1 bitmask4  = 0x0000000f;
Global const I1 bitmask5  = 0x0000001f;
Global const I1 bitmask6  = 0x0000003f;
Global const I1 bitmask7  = 0x0000007f;
Global const I1 bitmask8  = 0x000000ff;
Global const I1 bitmask9  = 0x000001ff;
Global const I1 bitmask10 = 0x000003ff;
Global const I1 bitmask11 = 0x000007ff;
Global const I1 bitmask12 = 0x00000fff;
Global const I1 bitmask13 = 0x00001fff;
Global const I1 bitmask14 = 0x00003fff;
Global const I1 bitmask15 = 0x00007fff;
Global const I1 bitmask16 = 0x0000ffff;
Global const I1 bitmask17 = 0x0001ffff;
Global const I1 bitmask18 = 0x0003ffff;
Global const I1 bitmask19 = 0x0007ffff;
Global const I1 bitmask20 = 0x000fffff;
Global const I1 bitmask21 = 0x001fffff;
Global const I1 bitmask22 = 0x003fffff;
Global const I1 bitmask23 = 0x007fffff;
Global const I1 bitmask24 = 0x00ffffff;
Global const I1 bitmask25 = 0x01ffffff;
Global const I1 bitmask26 = 0x03ffffff;
Global const I1 bitmask27 = 0x07ffffff;
Global const I1 bitmask28 = 0x0fffffff;
Global const I1 bitmask29 = 0x1fffffff;
Global const I1 bitmask30 = 0x3fffffff;
Global const I1 bitmask31 = 0x7fffffff;
Global const I1 bitmask32 = 0xffffffff;

Global const L1 bitmask33 = 0x00000001ffffffffull;
Global const L1 bitmask34 = 0x00000003ffffffffull;
Global const L1 bitmask35 = 0x00000007ffffffffull;
Global const L1 bitmask36 = 0x0000000fffffffffull;
Global const L1 bitmask37 = 0x0000001fffffffffull;
Global const L1 bitmask38 = 0x0000003fffffffffull;
Global const L1 bitmask39 = 0x0000007fffffffffull;
Global const L1 bitmask40 = 0x000000ffffffffffull;
Global const L1 bitmask41 = 0x000001ffffffffffull;
Global const L1 bitmask42 = 0x000003ffffffffffull;
Global const L1 bitmask43 = 0x000007ffffffffffull;
Global const L1 bitmask44 = 0x00000fffffffffffull;
Global const L1 bitmask45 = 0x00001fffffffffffull;
Global const L1 bitmask46 = 0x00003fffffffffffull;
Global const L1 bitmask47 = 0x00007fffffffffffull;
Global const L1 bitmask48 = 0x0000ffffffffffffull;
Global const L1 bitmask49 = 0x0001ffffffffffffull;
Global const L1 bitmask50 = 0x0003ffffffffffffull;
Global const L1 bitmask51 = 0x0007ffffffffffffull;
Global const L1 bitmask52 = 0x000fffffffffffffull;
Global const L1 bitmask53 = 0x001fffffffffffffull;
Global const L1 bitmask54 = 0x003fffffffffffffull;
Global const L1 bitmask55 = 0x007fffffffffffffull;
Global const L1 bitmask56 = 0x00ffffffffffffffull;
Global const L1 bitmask57 = 0x01ffffffffffffffull;
Global const L1 bitmask58 = 0x03ffffffffffffffull;
Global const L1 bitmask59 = 0x07ffffffffffffffull;
Global const L1 bitmask60 = 0x0fffffffffffffffull;
Global const L1 bitmask61 = 0x1fffffffffffffffull;
Global const L1 bitmask62 = 0x3fffffffffffffffull;
Global const L1 bitmask63 = 0x7fffffffffffffffull;
Global const L1 bitmask64 = 0xffffffffffffffffull;
