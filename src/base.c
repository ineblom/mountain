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

typedef float            F1;
typedef double           D1;
typedef signed char      SB1; typedef unsigned char      B1;
typedef signed short     SW1; typedef unsigned short     W1;
typedef signed int       SI1; typedef unsigned int       I1;
typedef signed long long SL1; typedef unsigned long long L1;
typedef float            F3 __attribute__((ext_vector_type(3)));
typedef float            F4 __attribute__((ext_vector_type(4)));

#define F1_MAX 3.4028234664e+38
#define F1_MIN 1.1754943508e-38
#define I1_MAX 0xFFFFFFFF

typedef F1 *Restrict F1R; typedef F1 Volatile* F1V;
typedef D1 *Restrict D1R; typedef D1 Volatile* D1V;
typedef B1 *Restrict B1R; typedef B1 Volatile* B1V;
typedef W1 *Restrict W1R; typedef W1 Volatile* W1V;
typedef I1 *Restrict I1R; typedef I1 Volatile* I1V;
typedef L1 *Restrict L1R; typedef L1 Volatile* L1V;

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

#define F1R_(x) ((F1R)(x))
#define D1R_(x) ((D1R)(x))
#define B1R_(x) ((B1R)(x))
#define W1R_(x) ((W1R)(x))
#define I1R_(x) ((I1R)(x))
#define L1R_(x) ((L1R)(x))

#define F1V_(x) ((F1V)(x))
#define D1V_(x) ((D1V)(x))
#define B1V_(x) ((B1V)(x))
#define W1V_(x) ((W1V)(x))
#define I1V_(x) ((I1V)(x))
#define L1V_(x) ((L1V)(x))

#define TR(T) T *Restrict
#define TV(T) T Volatile*
#define TR_(T,var) ((T *Restrict)L1_(var))
#define TV_(T,var) ((T Volatile*)L1_(var))

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

Inline I1 AtomicAddI1(I1V a, I1 v) {
	asm volatile("lock xaddl %0,%1"
							 : "=r"(v),"=m"(*a)
							 : "0"(v),"m"(*a)
							 : "memory","cc");
	return v;
}
Inline L1 AtomicAddL1(L1V a, L1 v) {
	asm volatile("lock xaddq %0,%1"
							 : "=r"(v),"=m"(*a)
							 : "0"(v),"m"(*a)
							 : "memory","cc");
	return v;
}
Inline I1 AtomicSwapI1(I1V a, I1 v) {
	asm volatile("xchgl %0,%1"
							 : "=r"(v),"=m"(*a)
							 : "0"(v),"m"(*a)
							 : "memory");
	return v;
}
Inline L1 AtomicSwapL1(L1V a, L1 v) {
	asm volatile("xchgq %0,%1"
							 : "=r"(v),"=m"(*a)
							 : "0"(v),"m"(*a)
							 : "memory");
	return v;
}

#define EachIndex(it, count) (L1 it = 0; it < (count); it += 1)
#define EachInRange(it, range) (L1 it = (range).min; it < (range).max; it += 1)
 
#define KiB(x) (1024 * x)
#define MiB(x) (1024 * 1024 * x)
#define GiB(x) (1024 * 1024 * 1024 * x)

#define ArrayLength(arr) (sizeof(arr) / sizeof(arr[0]))

#define Min(a, b) ((a) < (b) ? (a) : (b))
#define Max(a, b) ((a) > (b) ? (a) : (b))

#define Crash(x) __builtin_trap()
#define Assert(x) do { if (!(x)) {\
	fprintf(stderr, "Assert Failed (%s:%d) %s\n", __FILE__, __LINE__, #x);\
	fflush(stderr); \
	Crash(-1); \
	} } while (0)

#define Swap(a, b) do { typeof(a) temp = a; a = b; b = temp; } while(0)

#define AlignOf(T) __alignof(T)
#define AlignPow2(x,b) (((x) + (b) - 1)&(~((b) - 1)))
