/* SPDX-License-Identifier: GPL-2.0-only */
#include "tdx-compliance.h"

#define DEF_CPUID_TEST(_leaf, _subleaf)		\
{						\
	.name = "CPUID_" #_leaf "_" #_subleaf,	\
	.leaf = _leaf,				\
	.subleaf = _subleaf,			\
}

#define EXP_CPUID_BIT_CTL(_leaf, _subleaf, _reg, _bit_nr, _val, _vsn, _td_ctl, _pv_ctl) do {\
	struct test_cpuid *t;						\
	int bnr = _bit_nr;						\
	t = kzalloc(sizeof(struct test_cpuid), GFP_KERNEL);		\
	t->name = "CPUID(" #_leaf "," #_subleaf ")." #_reg "[" #_bit_nr "]";\
	t->version = (_vsn);						\
	t->leaf = (_leaf);						\
	t->subleaf = (_subleaf);					\
	t->regs._reg.mask = BIT(bnr);					\
	t->regs._reg.expect = BIT(bnr) * (_val);			\
	t->tdcs_td_ctl = (_td_ctl);					\
	t->tdcs_feature_pv_ctl = (_pv_ctl);				\
	list_add_tail(&t->list, &cpuid_list);				\
} while (0)

#define EXP_CPUID_BIT(_leaf, _subleaf, _reg, _bit_nr, _val, _vsn)	\
	EXP_CPUID_BIT_CTL(_leaf, _subleaf, _reg, _bit_nr, _val, _vsn, 0xbad, 0xbad)

#define EXP_CPUID_BYTE_CTL(_leaf, _subleaf, _reg, _val, _vsn, _td_ctl, _pv_ctl) do {\
	struct test_cpuid *t;						\
	t = kzalloc(sizeof(struct test_cpuid), GFP_KERNEL);		\
	t->name = "CPUID(" #_leaf "," #_subleaf ")." #_reg;		\
	t->version = (_vsn);						\
	t->leaf = (_leaf);						\
	t->subleaf = (_subleaf);					\
	t->regs._reg.mask = 0xffffffff;					\
	t->regs._reg.expect = (_val);					\
	t->tdcs_td_ctl = (_td_ctl);					\
	t->tdcs_feature_pv_ctl = (_pv_ctl);				\
	list_add_tail(&t->list, &cpuid_list);				\
} while (0)

#define EXP_CPUID_BYTE(_leaf, _subleaf, _reg, _val, _vsn)		\
	EXP_CPUID_BYTE_CTL(_leaf, _subleaf, _reg, _val, _vsn, 0xbad, 0xbad)

#define EXP_CPUID_RES_BITS_CTL(_leaf, _subleaf, _reg, _bit_s, _bit_e, _vsn, _td_ctl, _pv_ctl) do {\
	int i = 0;								\
	struct test_cpuid *t;							\
	t = kzalloc(sizeof(struct test_cpuid), GFP_KERNEL);			\
	t->name = "CPUID(" #_leaf "," #_subleaf ")." #_reg "[" #_bit_e ":" #_bit_s "]";\
	t->version = (_vsn);							\
	t->leaf = (_leaf);							\
	t->subleaf = (_subleaf);						\
	for (i = _bit_s; i <= (_bit_e); i++) {					\
		t->regs._reg.mask |= BIT(i);					\
	}									\
	t->tdcs_td_ctl = (_td_ctl);						\
	t->tdcs_feature_pv_ctl = (_pv_ctl);					\
	list_add_tail(&t->list, &cpuid_list);					\
} while (0)


#define EXP_CPUID_RES_BITS(_leaf, _subleaf, _reg, _bit_s, _bit_e, _vsn) \
	EXP_CPUID_RES_BITS_CTL(_leaf, _subleaf, _reg, _bit_s, _bit_e, _vsn, 0xbad, 0xbad)

#ifdef AUTOGEN_CPUID
extern void initial_cpuid(void);
#else
void initial_cpuid(void)
{
	/* CPUID(0x0) */
	EXP_CPUID_BYTE(0x0, 0, eax, 0x00000021, VER1_0);	//"MaxIndex"
	EXP_CPUID_BYTE(0x0, 0, eax, 0x00000023, VER1_5 | VER2_0);	//"MaxIndex"
	EXP_CPUID_BYTE(0x0, 0, ebx, 0x756e6547, VER1_5);	//"Genu"
	EXP_CPUID_BYTE(0x0, 0, ecx, 0x6c65746e, VER1_5);	//"ntel"
	EXP_CPUID_BYTE(0x0, 0, edx, 0x49656e69, VER1_5);	//"ineI"

	/* CPUID(0x1).EAX */
	EXP_CPUID_RES_BITS(0x1, 0, eax, 14, 15, VER1_0 | VER1_5 | VER2_0);	//Reserved_15_14
	EXP_CPUID_RES_BITS(0x1, 0, eax, 28, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved_31_28

	/* CPUID(0x1).EBX */
	EXP_CPUID_RES_BITS(0x1, 0, ebx, 0, 7, VER1_5);		//Brand Index
	/* CLFLUSH Line Size */
	EXP_CPUID_BIT(0x1, 0, ebx, 8, 0, VER1_0 | VER1_5 | VER2_0);
	EXP_CPUID_BIT(0x1, 0, ebx, 9, 0, VER1_0 | VER1_5 | VER2_0);
	EXP_CPUID_BIT(0x1, 0, ebx, 10, 0, VER1_0 | VER1_5 | VER2_0);
	EXP_CPUID_BIT(0x1, 0, ebx, 11, 1, VER1_0 | VER1_5 | VER2_0);
	EXP_CPUID_BIT(0x1, 0, ebx, 12, 0, VER1_0 | VER1_5 | VER2_0);
	EXP_CPUID_BIT(0x1, 0, ebx, 13, 0, VER1_0 | VER1_5 | VER2_0);
	EXP_CPUID_BIT(0x1, 0, ebx, 14, 0, VER1_0 | VER1_5 | VER2_0);
	EXP_CPUID_BIT(0x1, 0, ebx, 15, 0, VER1_0 | VER1_5 | VER2_0);

	/* CPUID(0x1).ECX */
	EXP_CPUID_BIT(0x1, 0, ecx, 0, 1, VER1_5);		//SSE3
	EXP_CPUID_BIT(0x1, 0, ecx, 1, 1, VER1_5);		//PCLMULQDQ
	EXP_CPUID_BIT(0x1, 0, ecx, 2, 1, VER1_5);		//DTES64
	EXP_CPUID_BIT(0x1, 0, ecx, 3, 0, VER1_0 | VER2_0);		//MONITOR
	EXP_CPUID_BIT(0x1, 0, ecx, 4, 1, VER1_5);		//DS-CPL
	EXP_CPUID_BIT(0x1, 0, ecx, 5, 0, VER1_0 | VER1_5 | VER2_0);		//VMX
	EXP_CPUID_BIT(0x1, 0, ecx, 6, 0, VER1_0 | VER1_5 | VER2_0);		//SMX
	EXP_CPUID_BIT(0x1, 0, ecx, 9, 1, VER1_5);		//SSSE3
	EXP_CPUID_BIT(0x1, 0, ecx, 13, 1, VER1_0 | VER1_5 | VER2_0);		//CMPXCHG16B
	EXP_CPUID_BIT(0x1, 0, ecx, 15, 1, VER1_0 | VER1_5 | VER2_0);		//PDCM
	EXP_CPUID_BIT(0x1, 0, ecx, 16, 0, VER1_0 | VER1_5 | VER2_0);		//Reserved_16
	EXP_CPUID_BIT(0x1, 0, ecx, 17, 1, VER1_5);		//PCID
	EXP_CPUID_BIT(0x1, 0, ecx, 19, 1, VER1_5);		//SSE4_1
	EXP_CPUID_BIT(0x1, 0, ecx, 20, 1, VER1_5);		//SSE4_2
	EXP_CPUID_BIT(0x1, 0, ecx, 21, 1, VER1_0 | VER1_5 | VER2_0);		//x2APIC
	EXP_CPUID_BIT(0x1, 0, ecx, 22, 1, VER1_5);		//MOVBE
	EXP_CPUID_BIT(0x1, 0, ecx, 23, 1, VER1_5);		//POPCNT
	EXP_CPUID_BIT(0x1, 0, ecx, 25, 1, VER1_0 | VER1_5 | VER2_0);		//AESNI
	EXP_CPUID_BIT(0x1, 0, ecx, 26, 1, VER1_0 | VER1_5 | VER2_0);		//XSAVE
	EXP_CPUID_BIT(0x1, 0, ecx, 30, 1, VER1_0 | VER1_5 | VER2_0);		//RDRAND
	EXP_CPUID_BIT(0x1, 0, ecx, 31, 1, VER1_0 | VER1_5 | VER2_0);		//Reserved_31

	/* CPUID(0x1).EDX */
	EXP_CPUID_BIT(0x1, 0, edx, 0, 1, VER1_5);		//FPU
	EXP_CPUID_BIT(0x1, 0, edx, 1, 1, VER1_5);		//VME
	EXP_CPUID_BIT(0x1, 0, edx, 2, 1, VER1_5);		//DE
	EXP_CPUID_BIT(0x1, 0, edx, 3, 1, VER1_5);		//PSE
	EXP_CPUID_BIT(0x1, 0, edx, 4, 1, VER1_5);		//TSC
	EXP_CPUID_BIT(0x1, 0, edx, 5, 1, VER1_0 | VER1_5 | VER2_0);		//MSR
	EXP_CPUID_BIT(0x1, 0, edx, 6, 1, VER1_0 | VER1_5 | VER2_0);		//PAE
	EXP_CPUID_BIT(0x1, 0, edx, 8, 1, VER1_5);		//CX8
	EXP_CPUID_BIT(0x1, 0, edx, 9, 1, VER1_0 | VER1_5 | VER2_0);		//APIC
	EXP_CPUID_BIT(0x1, 0, edx, 10, 0, VER1_0 | VER1_5 | VER2_0);		//Reserved_10
	EXP_CPUID_BIT(0x1, 0, edx, 11, 1, VER1_5);		//SEP
	EXP_CPUID_BIT(0x1, 0, edx, 13, 1, VER1_5);		//PGE
	EXP_CPUID_BIT(0x1, 0, edx, 15, 1, VER1_5);		//CMOV
	EXP_CPUID_BIT(0x1, 0, edx, 16, 1, VER1_5);		//PAT
	EXP_CPUID_BIT(0x1, 0, edx, 17, 0, VER1_5);		//PSE-36
	EXP_CPUID_BIT(0x1, 0, edx, 19, 1, VER1_0 | VER1_5 | VER2_0);		//CLFSH
	EXP_CPUID_BIT(0x1, 0, edx, 20, 0, VER1_0 | VER1_5 | VER2_0);		//Reserved_20
	EXP_CPUID_BIT(0x1, 0, edx, 21, 1, VER1_0 | VER1_5 | VER2_0);		//DS
	EXP_CPUID_BIT(0x1, 0, edx, 23, 1, VER1_5);		//MMX
	EXP_CPUID_BIT(0x1, 0, edx, 24, 1, VER1_5);		//FXSR
	EXP_CPUID_BIT(0x1, 0, edx, 25, 1, VER1_5);		//SSE
	EXP_CPUID_BIT(0x1, 0, edx, 26, 1, VER1_5);		//SSE2
	EXP_CPUID_BIT(0x1, 0, edx, 30, 0, VER1_0 | VER1_5 | VER2_0);		//Reserved_30

	/* CPUID(0x3) */
	EXP_CPUID_RES_BITS(0x3, 0, eax, 0, 31, VER1_0 | VER1_5 | VER2_0);		//Reserved_0_31
	EXP_CPUID_RES_BITS(0x3, 0, ebx, 0, 31, VER1_0 | VER1_5 | VER2_0);		//Reserved_0_31
	EXP_CPUID_RES_BITS(0x3, 0, ecx, 0, 31, VER1_0 | VER1_5 | VER2_0);		//Reserved_0_31
	EXP_CPUID_RES_BITS(0x3, 0, edx, 0, 31, VER1_0 | VER1_5 | VER2_0);		//Reserved_0_31

	/* CPUID(0x4, 0x0).EAX */
	EXP_CPUID_RES_BITS(0x4, 0, eax, 10, 13, VER1_5 | VER2_0);	//Reserved_13_10

	/* CPUID(0x4, 0x0).EBX */
	/* L */
	EXP_CPUID_BIT(0x4, 0, ebx, 0, 1, VER1_5);
	EXP_CPUID_BIT(0x4, 0, ebx, 1, 1, VER1_5);
	EXP_CPUID_BIT(0x4, 0, ebx, 2, 1, VER1_5);
	EXP_CPUID_BIT(0x4, 0, ebx, 3, 1, VER1_5);
	EXP_CPUID_BIT(0x4, 0, ebx, 4, 1, VER1_5);
	EXP_CPUID_BIT(0x4, 0, ebx, 5, 1, VER1_5);
	EXP_CPUID_BIT(0x4, 0, ebx, 6, 0, VER1_5);
	EXP_CPUID_BIT(0x4, 0, ebx, 7, 0, VER1_5);
	EXP_CPUID_BIT(0x4, 0, ebx, 8, 0, VER1_5);
	EXP_CPUID_BIT(0x4, 0, ebx, 9, 0, VER1_5);
	EXP_CPUID_BIT(0x4, 0, ebx, 10, 0, VER1_5);
	EXP_CPUID_BIT(0x4, 0, ebx, 11, 0, VER1_5);

	/* CPUID(0x4, 0x0).EDX */
	EXP_CPUID_BIT(0x4, 0, edx, 2, 0, VER1_5 | VER2_0);		//Reserved_2

	/* CPUID(0x4, 0x1).EAX */
	EXP_CPUID_RES_BITS(0x4, 1, eax, 10, 13, VER1_5 | VER2_0);	//Reserved_13_10

	/* CPUID(0x4, 0x1).EBX */
	/* L */
	EXP_CPUID_BIT(0x4, 1, ebx, 0, 1, VER1_5);
	EXP_CPUID_BIT(0x4, 1, ebx, 1, 1, VER1_5);
	EXP_CPUID_BIT(0x4, 1, ebx, 2, 1, VER1_5);
	EXP_CPUID_BIT(0x4, 1, ebx, 3, 1, VER1_5);
	EXP_CPUID_BIT(0x4, 1, ebx, 4, 1, VER1_5);
	EXP_CPUID_BIT(0x4, 1, ebx, 5, 1, VER1_5);
	EXP_CPUID_BIT(0x4, 1, ebx, 6, 0, VER1_5);
	EXP_CPUID_BIT(0x4, 1, ebx, 7, 0, VER1_5);
	EXP_CPUID_BIT(0x4, 1, ebx, 8, 0, VER1_5);
	EXP_CPUID_BIT(0x4, 1, ebx, 9, 0, VER1_5);
	EXP_CPUID_BIT(0x4, 1, ebx, 10, 0, VER1_5);
	EXP_CPUID_BIT(0x4, 1, ebx, 11, 0, VER1_5);

	/* CPUID(0x4, 0x1).EDX */
	EXP_CPUID_BIT(0x4, 1, edx, 2, 0, VER1_5 | VER2_0);		//Reserved_2

	/* CPUID(0x4, 0x2).EAX */
	EXP_CPUID_RES_BITS(0x4, 2, eax, 10, 13, VER1_5 | VER2_0);	//Reserved_13_10

	/* CPUID(0x4, 0x2).EBX */
	/* L */
	EXP_CPUID_BIT(0x4, 2, ebx, 0, 1, VER1_5);
	EXP_CPUID_BIT(0x4, 2, ebx, 1, 1, VER1_5);
	EXP_CPUID_BIT(0x4, 2, ebx, 2, 1, VER1_5);
	EXP_CPUID_BIT(0x4, 2, ebx, 3, 1, VER1_5);
	EXP_CPUID_BIT(0x4, 2, ebx, 4, 1, VER1_5);
	EXP_CPUID_BIT(0x4, 2, ebx, 5, 1, VER1_5);
	EXP_CPUID_BIT(0x4, 2, ebx, 6, 0, VER1_5);
	EXP_CPUID_BIT(0x4, 2, ebx, 7, 0, VER1_5);
	EXP_CPUID_BIT(0x4, 2, ebx, 8, 0, VER1_5);
	EXP_CPUID_BIT(0x4, 2, ebx, 9, 0, VER1_5);
	EXP_CPUID_BIT(0x4, 2, ebx, 10, 0, VER1_5);
	EXP_CPUID_BIT(0x4, 2, ebx, 11, 0, VER1_5);

	/* CPUID(0x4, 0x2).EDX */
	EXP_CPUID_BIT(0x4, 2, edx, 2, 0, VER1_5 | VER2_0);		//Reserved_2

	/* CPUID(0x4, 0x3).EAX */
	EXP_CPUID_RES_BITS(0x4, 3, eax, 10, 13, VER1_5 | VER2_0);	//Reserved_13_10

	/* CPUID(0x4, 0x3).EBX */
	/* L */
	EXP_CPUID_BIT(0x4, 3, ebx, 0, 1, VER1_5);
	EXP_CPUID_BIT(0x4, 3, ebx, 1, 1, VER1_5);
	EXP_CPUID_BIT(0x4, 3, ebx, 2, 1, VER1_5);
	EXP_CPUID_BIT(0x4, 3, ebx, 3, 1, VER1_5);
	EXP_CPUID_BIT(0x4, 3, ebx, 4, 1, VER1_5);
	EXP_CPUID_BIT(0x4, 3, ebx, 5, 1, VER1_5);
	EXP_CPUID_BIT(0x4, 3, ebx, 6, 0, VER1_5);
	EXP_CPUID_BIT(0x4, 3, ebx, 7, 0, VER1_5);
	EXP_CPUID_BIT(0x4, 3, ebx, 8, 0, VER1_5);
	EXP_CPUID_BIT(0x4, 3, ebx, 9, 0, VER1_5);
	EXP_CPUID_BIT(0x4, 3, ebx, 10, 0, VER1_5);
	EXP_CPUID_BIT(0x4, 3, ebx, 11, 0, VER1_5);

	/* CPUID(0x4, 0x3).EDX */
	EXP_CPUID_RES_BITS(0x4, 3, edx, 3, 31, VER1_5 | VER2_0);		//Reserved_31_3

	/* CPUID(0x4, 0x4).EAX */
	EXP_CPUID_RES_BITS(0x4, 4, eax, 0, 4, VER1_0 | VER1_5 | VER2_0);		//Type
	EXP_CPUID_RES_BITS(0x4, 4, eax, 5, 7, VER1_0 | VER1_5 | VER2_0);		//Level
	EXP_CPUID_BIT(0x4, 4, eax, 8, 0, VER1_0 | VER1_5 | VER2_0);		//Self Initializing
	EXP_CPUID_BIT(0x4, 4, eax, 9, 0, VER1_0 | VER1_5 | VER2_0);		//Fully Associative
	EXP_CPUID_RES_BITS(0x4, 4, eax, 10, 13, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_RES_BITS(0x4, 4, eax, 14, 25, VER1_0 | VER1_5 | VER2_0);	//Addressable IDs Sharing this Cache
	EXP_CPUID_RES_BITS(0x4, 4, eax, 26, 31, VER1_0 | VER1_5 | VER2_0);	//Addressable IDs for Cores in Package

	/* CPUID(0x4, 0x4).EBX */
	EXP_CPUID_RES_BITS(0x4, 4, ebx, 0, 11, VER1_0 | VER1_5 | VER2_0);		//L
	EXP_CPUID_RES_BITS(0x4, 4, ebx, 12, 21, VER1_0 | VER1_5 | VER2_0);	//P
	EXP_CPUID_RES_BITS(0x4, 4, ebx, 22, 31, VER1_0 | VER1_5 | VER2_0);	//W

	/* CPUID(0x4, 0x4).ECX */
	EXP_CPUID_BYTE(0x4, 4, ecx, 0, VER1_0 | VER1_5 | VER2_0);			//Number of Sets

	/* CPUID(0x4, 0x4).EDX */
	EXP_CPUID_BIT(0x4, 4, edx, 0, 0, VER1_0 | VER1_5 | VER2_0);		//WBINVD
	EXP_CPUID_BIT(0x4, 4, edx, 1, 0, VER1_0 | VER1_5 | VER2_0);		//Cache Inclusiveness
	EXP_CPUID_BIT(0x4, 4, edx, 2, 0, VER1_0 | VER1_5 | VER2_0);		//Complex Cache Indexing
	EXP_CPUID_RES_BITS(0x4, 4, edx, 3, 31, VER1_0 | VER1_5 | VER2_0);		//Reserved

	/* CPUID(0x7, 0x0).EAX */
	EXP_CPUID_BYTE(0x7, 0, eax, 2, VER1_5 | VER2_0);			//Max Sub-Leaves
	EXP_CPUID_BYTE(0x7, 0, eax, 1, VER1_0);			//Max Sub-Leaves

	/* CPUID(0x7, 0x0).EBX */
	EXP_CPUID_BIT(0x7, 0x0, ebx, 0, 1, VER1_0 | VER1_5 | VER2_0);		//FSGSBASE
	EXP_CPUID_BIT(0x7, 0x0, ebx, 1, 0, VER1_0 | VER1_5 | VER2_0);		//IA32_TSC_ADJUST
	EXP_CPUID_BIT(0x7, 0x0, ebx, 2, 0, VER1_0 | VER1_5 | VER2_0);		//SGX
	EXP_CPUID_BIT(0x7, 0x0, ebx, 6, 1, VER1_5);		//FDP_EXCPTN_ONLY
	EXP_CPUID_BIT(0x7, 0x0, ebx, 7, 1, VER1_5);		//SMEP
	EXP_CPUID_BIT(0x7, 0x0, ebx, 10, 1, VER1_5);		//INVPCID
	EXP_CPUID_BIT(0x7, 0x0, ebx, 11, 1, VER2_0);		//RTM
	EXP_CPUID_BIT(0x7, 0x0, ebx, 13, 1, VER1_5);		//FCS/FDS Deprecation
	EXP_CPUID_BIT(0x7, 0x0, ebx, 14, 0, VER1_0 | VER1_5 | VER2_0);		//MPX
	EXP_CPUID_BIT(0x7, 0x0, ebx, 18, 1, VER1_0 | VER1_5 | VER2_0);		//RDSEED
	EXP_CPUID_BIT(0x7, 0x0, ebx, 20, 1, VER1_0 | VER1_5 | VER2_0);		//SMAP/CLAC/STAC
	EXP_CPUID_BIT(0x7, 0x0, ebx, 22, 0, VER1_0);		//PCOMMIT
	EXP_CPUID_BIT(0x7, 0x0, ebx, 23, 1, VER1_0 | VER1_5 | VER2_0);		//CLFLUSHOPT
	EXP_CPUID_BIT(0x7, 0x0, ebx, 24, 1, VER1_0 | VER1_5 | VER2_0);		//CLWB
	EXP_CPUID_BIT(0x7, 0x0, ebx, 29, 1, VER1_0 | VER1_5 | VER2_0);		//SHA

	/* CPUID(0x7, 0x0).ECX */
	EXP_CPUID_BIT(0x7, 0x0, ecx, 15, 0, VER1_0 | VER1_5 | VER2_0);		//FZM
	EXP_CPUID_RES_BITS(0x7, 0x0, ecx, 17, 21, VER1_0 | VER1_5 | VER2_0);	//MAWAU for MPX
	EXP_CPUID_BIT(0x7, 0x0, ecx, 24, 1, VER1_0 | VER1_5 | VER2_0);		//BUSLOCK
	EXP_CPUID_BIT(0x7, 0x0, ecx, 26, 0, VER1_5 | VER2_0);		//Reserved
	EXP_CPUID_BIT(0x7, 0x0, ecx, 27, 1, VER1_5);		//MOVDIRI
	EXP_CPUID_BIT(0x7, 0x0, ecx, 28, 1, VER1_0 | VER1_5 | VER2_0);		//MOVDIR64B
	EXP_CPUID_BIT(0x7, 0x0, ecx, 29, 0, VER1_0 | VER1_5 | VER2_0);		//ENQCMD
	EXP_CPUID_BIT(0x7, 0x0, ecx, 30, 0, VER1_0 | VER1_5 | VER2_0);		//SGX_LC

	/* CPUID(0x7, 0x0).EDX */
	EXP_CPUID_RES_BITS(0x7, 0x0, edx, 0, 1, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_RES_BITS(0x7, 0x0, edx, 6, 7, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_BIT(0x7, 0x0, edx, 9, 0, VER1_0 | VER1_5 | VER2_0);		//MCU_OPT supported
	EXP_CPUID_BIT(0x7, 0x0, edx, 10, 1, VER1_5);		//MD_CLEAR supported
	EXP_CPUID_RES_BITS(0x7, 0x0, edx, 11, 12, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_BIT(0x7, 0x0, edx, 13, 0, VER1_0 | VER1_5 | VER2_0);		//RTM_FORCE_ABORT_SUPPORT(Reserved in 1.0)
	EXP_CPUID_BIT(0x7, 0x0, edx, 17, 0, VER1_0 | VER1_5 | VER2_0);		//Reserved
	EXP_CPUID_BIT(0x7, 0x0, edx, 21, 0, VER1_0 | VER1_5 | VER2_0);		//Reserved
	EXP_CPUID_BIT(0x7, 0x0, edx, 26, 1, VER1_0 | VER1_5 | VER2_0);		//IBRS
	EXP_CPUID_BIT(0x7, 0x0, edx, 27, 1, VER1_5);		//STIBP
	EXP_CPUID_BIT(0x7, 0x0, edx, 29, 1, VER1_0 | VER1_5 | VER2_0);		//IA32_ARCH_CAPABILITIES Support
	EXP_CPUID_BIT(0x7, 0x0, edx, 31, 1, VER1_0 | VER1_5 | VER2_0);		//SSBD(Speculative Store Bypass Disable)

	/* CPUID(0x7, 0x1).EAX */
	EXP_CPUID_RES_BITS(0x7, 0x1, eax, 0, 3, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_RES_BITS(0x7, 0x1, eax, 6, 9, VER1_0);	//Reserved
	EXP_CPUID_BIT(0x7, 0x1, eax, 7, 0, VER1_5 | VER2_0);		//Reserved
	EXP_CPUID_BIT(0x7, 0x1, eax, 8, 0, VER2_0);		//Reserved
	EXP_CPUID_BIT(0x7, 0x1, eax, 9, 0, VER1_5 | VER2_0);		//Reserved
	EXP_CPUID_RES_BITS(0x7, 0x1, eax, 13, 21, VER1_0 | VER1_5);	//Reserved
	EXP_CPUID_RES_BITS(0x7, 0x1, eax, 13, 18, VER2_0);	//Reserved
	EXP_CPUID_BIT(0x7, 0x1, eax, 20, 0, VER2_0);		//HRESET
	EXP_CPUID_BIT(0x7, 0x1, eax, 22, 0, VER1_0 | VER1_5 | VER2_0);		//HRESET
	EXP_CPUID_RES_BITS(0x7, 0x1, eax, 23, 31, VER1_0);	//Reserved
	EXP_CPUID_RES_BITS(0x7, 0x1, eax, 23, 25, VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_RES_BITS(0x7, 0x1, eax, 27, 31, VER1_5 | VER2_0);	//Reserved

	/* CPUID(0x7, 0x1).EBX */
	EXP_CPUID_RES_BITS(0x7, 0x1, ebx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved

	/* CPUID(0x7, 0x1).ECX */
	EXP_CPUID_RES_BITS(0x7, 0x1, ecx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved

	/* CPUID(0x7, 0x1).EDX */
//TODO: big change here.
//	EXP_CPUID_RES_BITS(0x7, 0x1, edx, 0, 31, VER2_0);	//Reserved
	EXP_CPUID_BIT(0x7, 0x1, edx, 4, 1, VER1_5);
	EXP_CPUID_BIT(0x7, 0x1, edx, 5, 1, VER1_5);

	/* CPUID(0x7, 0x2).EAX */
	EXP_CPUID_RES_BITS(0x7, 0x2, eax, 0, 31, VER1_5);	//Reserved

	/* CPUID(0x7, 0x2).EBX */
	EXP_CPUID_RES_BITS(0x7, 0x2, ebx, 0, 31, VER1_5);	//Reserved

	/* CPUID(0x7, 0x2).ECX */
	EXP_CPUID_RES_BITS(0x7, 0x2, ecx, 0, 31, VER1_5);	//Reserved

	/* CPUID(0x7, 0x2).EDX */
	EXP_CPUID_BIT(0x7, 0x2, edx, 0, 1, VER1_5);		//PSFD
	EXP_CPUID_BIT(0x7, 0x2, edx, 1, 1, VER1_5);		//IPRED_CTRL
	EXP_CPUID_BIT(0x7, 0x2, edx, 2, 1, VER1_5);		//RRSBA_CTRL
	EXP_CPUID_BIT(0x7, 0x2, edx, 4, 1, VER1_5);		//BHI_CTRL
	EXP_CPUID_RES_BITS(0x7, 0x2, edx, 6, 31, VER1_5);	//Reserved

	/* CPUID(0x8) */
	EXP_CPUID_RES_BITS(0x8, 0x0, eax, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_RES_BITS(0x8, 0x0, ebx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_RES_BITS(0x8, 0x0, ecx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_RES_BITS(0x8, 0x0, edx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved

	/* CPUID(0xa, 0x0).EDX */
	EXP_CPUID_RES_BITS(0xa, 0x0, edx, 13, 14, VER1_5 | VER2_0);	//Reserved
//TODO: Need check Attributes & Native	PERFMON
//	EXP_CPUID_BIT(0xa, 0x0, edx, 15, 1, VER1_5 | VER2_0);		//AnyThread Deprecation
	EXP_CPUID_RES_BITS(0xa, 0x0, edx, 16, 31, VER1_5 | VER2_0);	//Reserved

	/* CPUID(0xd, 0x0).EAX */
	EXP_CPUID_BIT(0xd, 0x0, eax, 0, 1, VER1_0 | VER1_5 | VER2_0);		//X87
	EXP_CPUID_BIT(0xd, 0x0, eax, 1, 1, VER1_0 | VER1_5 | VER2_0);		//SSE
	EXP_CPUID_BIT(0xd, 0x0, eax, 3, 0, VER1_0 | VER1_5 | VER2_0);		//PL_BNDREGS
	EXP_CPUID_BIT(0xd, 0x0, eax, 4, 0, VER1_0 | VER1_5 | VER2_0);		//PL_BNDCFS
	EXP_CPUID_BIT(0xd, 0x0, eax, 8, 0, VER1_0 | VER1_5 | VER2_0);		//Reserved
	EXP_CPUID_RES_BITS(0xd, 0x0, eax, 10, 16, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_RES_BITS(0xd, 0x0, eax, 19, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved

	/* CPUID(0xd, 0x0).EDX */
	EXP_CPUID_RES_BITS(0xd, 0x0, edx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved

	/* CPUID(0xd, 0x1).EAX */
	EXP_CPUID_BIT(0xd, 0x1, eax, 0, 1, VER1_0 | VER1_5 | VER2_0);		//Supports XSAVEOPT
	EXP_CPUID_BIT(0xd, 0x1, eax, 1, 1, VER1_0 | VER1_5 | VER2_0);		//Supports XSAVEC and compacted XRSTOR
	EXP_CPUID_BIT(0xd, 0x1, eax, 2, 1, VER1_5);		//Supports XGETBV with ECX = 1
	EXP_CPUID_BIT(0xd, 0x1, eax, 3, 1, VER1_0 | VER1_5 | VER2_0);		//Supports XSAVES/XRSTORS and IA32_XSS
	EXP_CPUID_RES_BITS(0xd, 0x1, eax, 5, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved

	/* CPUID(0xd, 0x1).ECX */
	EXP_CPUID_RES_BITS(0xd, 0x1, ecx, 0, 7, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_BIT(0xd, 0x1, ecx, 9, 0, VER1_0 | VER1_5 | VER2_0);		//Reserved
	EXP_CPUID_BIT(0xd, 0x1, ecx, 10, 0, VER1_0 | VER1_5 | VER2_0);		//PASID
	EXP_CPUID_BIT(0xd, 0x1, ecx, 13, 0, VER1_0 | VER1_5 | VER2_0);		//HDC
	EXP_CPUID_BIT(0xd, 0x1, ecx, 16, 0, VER1_0 | VER1_5 | VER2_0);		//HDC
	EXP_CPUID_RES_BITS(0xd, 0x1, ecx, 17, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved

	/* CPUID(0xd, 0x1).EDX */
	EXP_CPUID_RES_BITS(0xd, 0x1, edx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved

	/* CPUID(0xd, 0x2-0x12).EDX */
	EXP_CPUID_RES_BITS(0xd, 0x2, edx, 0, 31, VER1_5 | VER2_0);	//Reserved_0_31
	EXP_CPUID_RES_BITS(0xd, 0x3, edx, 0, 31, VER1_5 | VER2_0);	//Reserved_0_31
	EXP_CPUID_RES_BITS(0xd, 0x4, edx, 0, 31, VER1_5 | VER2_0);	//Reserved_0_31
	EXP_CPUID_RES_BITS(0xd, 0x5, edx, 0, 31, VER1_5 | VER2_0);	//Reserved_0_31
	EXP_CPUID_RES_BITS(0xd, 0x6, edx, 0, 31, VER1_5 | VER2_0);	//Reserved_0_31
	EXP_CPUID_RES_BITS(0xd, 0x7, edx, 0, 31, VER1_5 | VER2_0);	//Reserved_0_31
	EXP_CPUID_RES_BITS(0xd, 0x8, edx, 0, 31, VER1_5 | VER2_0);	//Reserved_0_31
	EXP_CPUID_RES_BITS(0xd, 0x9, edx, 0, 31, VER1_5 | VER2_0);	//Reserved_0_31
	EXP_CPUID_RES_BITS(0xd, 0xa, edx, 0, 31, VER1_5 | VER2_0);	//Reserved_0_31
	EXP_CPUID_RES_BITS(0xd, 0xb, edx, 0, 31, VER1_5 | VER2_0);	//Reserved_0_31
	EXP_CPUID_RES_BITS(0xd, 0xc, edx, 0, 31, VER1_5 | VER2_0);	//Reserved_0_31
	EXP_CPUID_RES_BITS(0xd, 0xd, edx, 0, 31, VER1_5 | VER2_0);	//Reserved_0_31
	EXP_CPUID_RES_BITS(0xd, 0xe, edx, 0, 31, VER1_5 | VER2_0);	//Reserved_0_31
	EXP_CPUID_RES_BITS(0xd, 0xf, edx, 0, 31, VER1_5 | VER2_0);	//Reserved_0_31
	EXP_CPUID_RES_BITS(0xd, 0x10, edx, 0, 31, VER1_5 | VER2_0);	//Reserved_0_31
	EXP_CPUID_RES_BITS(0xd, 0x11, edx, 0, 31, VER1_5 | VER2_0);	//Reserved_0_31
	EXP_CPUID_RES_BITS(0xd, 0x12, edx, 0, 31, VER1_5 | VER2_0);	//Reserved_0_31

	/* CPUID(0xe, 0x0) */
	EXP_CPUID_RES_BITS(0xe, 0x0, eax, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_RES_BITS(0xe, 0x0, ebx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_RES_BITS(0xe, 0x0, ecx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_RES_BITS(0xe, 0x0, edx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved

	/* CPUID(0x11, 0x0) */
	EXP_CPUID_RES_BITS(0x11, 0x0, eax, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_RES_BITS(0x11, 0x0, ebx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_RES_BITS(0x11, 0x0, ecx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_RES_BITS(0x11, 0x0, edx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved

	/* CPUID(0x12, 0x0) */
	EXP_CPUID_RES_BITS(0x12, 0x0, eax, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_RES_BITS(0x12, 0x0, ebx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_RES_BITS(0x12, 0x0, ecx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_RES_BITS(0x12, 0x0, edx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved

	/* CPUID(0x13, 0x0) */
	EXP_CPUID_RES_BITS(0x13, 0x0, eax, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_RES_BITS(0x13, 0x0, ebx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_RES_BITS(0x13, 0x0, ecx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_RES_BITS(0x13, 0x0, edx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved

	/* CPUID(0x15, 0x0).EAX */
	EXP_CPUID_BYTE(0x15, 0x0, eax, 0x1, VER1_0 | VER1_5 | VER2_0);		//Denominator
	/* CPUID(0x15, 0x0).ECX */
	EXP_CPUID_BYTE(0x15, 0x0, ecx, 0x017d7840, VER1_0 | VER1_5 | VER2_0);	//Nominal ART Frequency
	/* CPUID(0x15, 0x0).EDX */
	EXP_CPUID_RES_BITS(0x15, 0x0, edx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved

	/* CPUID(0x19, 0x0).EAX */
	EXP_CPUID_RES_BITS(0x19, 0x0, eax, 3, 31, VER1_5);		//Reserved

	/* CPUID(0x19, 0x0).EBX */
	EXP_CPUID_BIT(0x19, 0x0, ebx, 1, 0, VER1_5);		//Reserved
	EXP_CPUID_BIT(0x19, 0x0, ebx, 3, 0, VER1_5);		//Reserved
	EXP_CPUID_BIT(0x19, 0x0, ebx, 4, 0, VER1_5);		//IW Key Backup Support
	EXP_CPUID_RES_BITS(0x19, 0x0, ebx, 5, 31, VER1_5);	//Reserved

	/* CPUID(0x19, 0x0).ECX */
	EXP_CPUID_BIT(0x19, 0x0, ecx, 0, 0, VER1_5);			//LOADIWKey Support
	EXP_CPUID_BIT(0x19, 0x0, ecx, 1, 0, VER1_0 | VER2_0);			//Random IWKey Support
	EXP_CPUID_RES_BITS(0x19, 0x0, ecx, 2, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved
	/* CPUID(0x19, 0x0).EDX */
	EXP_CPUID_RES_BITS(0x19, 0x0, edx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved

	/* CPUID(0x20, 0x0) */
	EXP_CPUID_RES_BITS(0x20, 0x0, eax, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_RES_BITS(0x20, 0x0, ebx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_RES_BITS(0x20, 0x0, ecx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_RES_BITS(0x20, 0x0, edx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved

	/* CPUID(0x21, 0x0).EAX */
	EXP_CPUID_BYTE(0x21, 0x0, eax, 0x0, VER1_0 | VER1_5 | VER2_0);		//Maximum sub-leaf
	/* CPUID(0x21, 0x0).EBX */
	EXP_CPUID_BYTE(0x21, 0x0, ebx, 0x65746E49, VER1_0 | VER1_5 | VER2_0);	//"Intel"
	/* CPUID(0x21, 0x0).ECX */
	EXP_CPUID_BYTE(0x21, 0x0, ecx, 0x20202020, VER1_0 | VER1_5 | VER2_0);	//"    "
	/* CPUID(0x21, 0x0).EDX */
	EXP_CPUID_BYTE(0x21, 0x0, edx, 0x5844546C, VER1_0 | VER1_5 | VER2_0);	//"lTDX"

	/* CPUID(0x22, 0x0) */
	EXP_CPUID_RES_BITS(0x22, 0x0, eax, 0, 31, VER1_5);	//Reserved
	EXP_CPUID_RES_BITS(0x22, 0x0, ebx, 0, 31, VER1_5);	//Reserved
	EXP_CPUID_RES_BITS(0x22, 0x0, ecx, 0, 31, VER1_5);	//Reserved
	EXP_CPUID_RES_BITS(0x22, 0x0, edx, 0, 31, VER1_5);	//Reserved

	/* CPUID(0x23, 0x0).EAX */
	EXP_CPUID_RES_BITS(0x23, 0x0, eax, 4, 5, VER1_5);	//Valig sub-leaf bitmap
	EXP_CPUID_RES_BITS(0x23, 0x0, eax, 6, 31, VER1_5);	//Reserved

	/* CPUID(0x23, 0x0).ECX */
	EXP_CPUID_RES_BITS(0x23, 0x0, ecx, 0, 31, VER1_5);	//Reserved
	/* CPUID(0x23, 0x0).EDX */
	EXP_CPUID_RES_BITS(0x23, 0x0, edx, 0, 31, VER1_5);	//Reserved

	/* CPUID(0x23, 0x1).EBX */
	EXP_CPUID_RES_BITS(0x23, 0x1, ebx, 0, 3, VER1_5);	//Fixed counter bitmap
	EXP_CPUID_RES_BITS(0x23, 0x1, ebx, 4, 31, VER1_5);	//Fixed counter bitmap
	/* CPUID(0x23, 0x1).ECX */
	EXP_CPUID_RES_BITS(0x23, 0x1, ecx, 0, 31, VER1_5);	//Reserved
	/* CPUID(0x23, 0x1).EDX */
	EXP_CPUID_RES_BITS(0x23, 0x1, edx, 0, 31, VER1_5);	//Reserved

	/* CPUID(0x23, 0x2) */
	EXP_CPUID_RES_BITS(0x23, 0x2, eax, 0, 31, VER1_5);	//Reserved
	EXP_CPUID_RES_BITS(0x23, 0x2, ebx, 0, 31, VER1_5);	//Reserved
	EXP_CPUID_RES_BITS(0x23, 0x2, ecx, 0, 31, VER1_5);	//Reserved
	EXP_CPUID_RES_BITS(0x23, 0x2, edx, 0, 31, VER1_5);	//Reserved

	/* CPUID(0x23, 0x3).EBX */
	EXP_CPUID_RES_BITS(0x23, 0x3, ebx, 0, 31, VER1_5);	//Reserved
	/* CPUID(0x23, 0x3).ECX */
	EXP_CPUID_RES_BITS(0x23, 0x3, ecx, 0, 31, VER1_5);	//Reserved
	/* CPUID(0x23, 0x3).EDX */
	EXP_CPUID_RES_BITS(0x23, 0x3, edx, 0, 31, VER1_5);	//Reserved

	/* CPUID(0x80000000).EAX */
	EXP_CPUID_BYTE(0x80000000, 0x0, eax, 0x80000008, VER1_5);	//MaxIndex
	/* CPUID(0x80000000).EBX */
	EXP_CPUID_RES_BITS(0x80000000, 0x0, ebx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved
	/* CPUID(0x80000000).ECX */
	EXP_CPUID_RES_BITS(0x80000000, 0x0, ecx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved
	/* CPUID(0x80000000).EDX */
	EXP_CPUID_RES_BITS(0x80000000, 0x0, edx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved

	/* CPUID(0x80000001).EAX */
	EXP_CPUID_RES_BITS(0x80000001, 0x0, eax, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved
	/* CPUID(0x80000001).EBX */
	EXP_CPUID_RES_BITS(0x80000001, 0x0, ebx, 0, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved
	/* CPUID(0x80000001).ECX */
	EXP_CPUID_BIT(0x80000001, 0x0, ecx, 0, 1, VER1_5);		//LAHF/SAHF in 64-bit Mode
	EXP_CPUID_RES_BITS(0x80000001, 0x0, ecx, 1, 4, VER1_0 | VER1_5 | VER2_0);		//Reserved
	EXP_CPUID_BIT(0x80000001, 0x0, ecx, 5, 1, VER1_5);		//LZCNT
	EXP_CPUID_RES_BITS(0x80000001, 0x0, ecx, 6, 7, VER1_0 | VER1_5 | VER2_0);		//Reserved
	EXP_CPUID_BIT(0x80000001, 0x0, ecx, 8, 1, VER1_5);		//PREFETCHW
	EXP_CPUID_RES_BITS(0x80000001, 0x0, ecx, 9, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved

	/* CPUID(0x80000001).EDX */
	EXP_CPUID_RES_BITS(0x80000001, 0x0, edx, 0, 10, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_RES_BITS(0x80000001, 0x0, edx, 12, 19, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_BIT(0x80000001, 0x0, edx, 20, 1, VER1_0 | VER1_5 | VER2_0);		//Execute Dis Bit
	EXP_CPUID_RES_BITS(0x80000001, 0x0, edx, 21, 25, VER1_0 | VER1_5 | VER2_0);	//Reserved
	EXP_CPUID_BIT(0x80000001, 0x0, edx, 26, 1, VER1_0 | VER1_5 | VER2_0);		//1GB Pages
	EXP_CPUID_BIT(0x80000001, 0x0, edx, 27, 1, VER1_0 | VER1_5 | VER2_0);		//RDTSCP
	EXP_CPUID_BIT(0x80000001, 0x0, edx, 28, 0, VER1_0 | VER1_5 | VER2_0);		//Reserved
	EXP_CPUID_BIT(0x80000001, 0x0, edx, 29, 1, VER1_0 | VER1_5 | VER2_0);		//Intel 64
	EXP_CPUID_RES_BITS(0x80000001, 0x0, edx, 30, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved

	/* CPUID(0x80000007).EAX */
	EXP_CPUID_RES_BITS(0x80000007, 0x0, eax, 0, 31, VER1_5);	//Reserved
	/* CPUID(0x80000007).EBX */
	EXP_CPUID_RES_BITS(0x80000007, 0x0, ebx, 0, 31, VER1_5);	//Reserved
	/* CPUID(0x80000007).ECX */
	EXP_CPUID_RES_BITS(0x80000007, 0x0, ecx, 0, 31, VER1_5);	//Reserved
	/* CPUID(0x80000007).EDX */
	EXP_CPUID_RES_BITS(0x80000007, 0x0, edx, 0, 7, VER1_5);		//Reserved_7_0
	EXP_CPUID_BIT(0x80000007, 0x0, edx, 8, 1, VER1_5);		//Invariant TSC
	EXP_CPUID_RES_BITS(0x80000007, 0x0, edx, 9, 31, VER1_5);	//Reserved_31_9

	/* CPUID(0x80000008).EAX */
	/* CPUID.EAX[0:7] Number of Physical Address Bits */
	EXP_CPUID_BIT(0x80000008, 0x0, eax, 0, 0, VER1_0 | VER1_5 | VER2_0);
	EXP_CPUID_BIT(0x80000008, 0x0, eax, 1, 0, VER1_0 | VER1_5 | VER2_0);
	EXP_CPUID_BIT(0x80000008, 0x0, eax, 2, 1, VER1_0 | VER1_5 | VER2_0);
	EXP_CPUID_BIT(0x80000008, 0x0, eax, 3, 0, VER1_0 | VER1_5 | VER2_0);
	EXP_CPUID_BIT(0x80000008, 0x0, eax, 4, 1, VER1_0 | VER1_5 | VER2_0);
	EXP_CPUID_BIT(0x80000008, 0x0, eax, 5, 1, VER1_0 | VER1_5 | VER2_0);
	EXP_CPUID_BIT(0x80000008, 0x0, eax, 6, 0, VER1_0 | VER1_5 | VER2_0);
	EXP_CPUID_BIT(0x80000008, 0x0, eax, 7, 0, VER1_0 | VER1_5 | VER2_0);
	EXP_CPUID_RES_BITS(0x80000008, 0x0, eax, 16, 31, VER1_0 | VER1_5 | VER2_0);	//Reserved

	/* CPUID(0x80000008).EBX */
	EXP_CPUID_RES_BITS(0x80000008, 0x0, ebx, 0, 8, VER1_0 | VER1_5 | VER2_0);		//Reserved
	EXP_CPUID_BIT(0x80000008, 0x0, ebx, 9, 1, VER2_0);			//WBNOINVD support
	EXP_CPUID_RES_BITS(0x80000008, 0x0, ebx, 10, 31, VER1_0 | VER2_0);	//Reserved

	/* CPUID(0x80000008).ECX */
	EXP_CPUID_RES_BITS(0x80000008, 0x0, ecx, 0, 31, VER1_0 | VER2_0);	//Reserved

	/* CPUID(0x80000008).EDX */
	EXP_CPUID_RES_BITS(0x80000008, 0x0, edx, 0, 31, VER1_0 | VER2_0);	//Reserved

/********* The following test cases are defined for #VE reduction. *********/

/*
 * There are 3 configurations in total:
 * 1. By default -- when #VE Reduction is enabled: TDCS.TD_CTRL.REDUCE_VE ==1 ,
 * TDCS.FEATURE_PARAVIRT_CTLS is all-0.
 * 2. TD_CTLS.REDUCE_VE == 1, FEATURE_PARAVIRT_CTLS == 1
 * 3. Backward-Compatible -- when #VE reduction is not enabled: TD_CTLS.REDUCE_VE is 0.
 */

/* 1. By default, TDCS.TD_CTRL.REDUCE_VE is set, TDCS.FEATURE_PARAVIRT_CTLS is all-0 */

	/* CPUID(0x1) */
	/* EST(est) */
	EXP_CPUID_BIT(0x1, 0, ecx, 7, 0x0, VER1_5);
	/* TSC_DEADLINE(tsc-deadline) */
	EXP_CPUID_BIT(0x1, 0, ecx, 24, 0x0, VER1_5);
	/* MCA(mce) */
	EXP_CPUID_BIT(0x1, 0, edx, 7, 0x0, VER1_5);
	/* MTRR(mtrr) */
	EXP_CPUID_BIT(0x1, 0, edx, 12, 0x0, VER1_5);
	/* MCA(mca) */
	EXP_CPUID_BIT(0x1, 0, edx, 14, 0x0, VER1_5);
	/* TM(acpi) */
	EXP_CPUID_BIT(0x1, 0, edx, 22, 0x0, VER1_5);

	/* CPUID(0x2) */
	EXP_CPUID_BYTE(0x2, 0, eax, 0x00feff01, VER1_5);
	EXP_CPUID_BYTE(0x2, 0, ebx, 0, VER1_5);
	EXP_CPUID_BYTE(0x2, 0, ecx, 0, VER1_5);
	EXP_CPUID_BYTE(0x2, 0, edx, 0, VER1_5);

	/* CPUID(0x6) */
	EXP_CPUID_BIT(0x6, 0, eax, 2, 0x1, VER1_5);
	EXP_CPUID_RES_BITS(0x6, 0, eax, 0, 1, VER1_5);
	EXP_CPUID_RES_BITS(0x6, 0, eax, 3, 31,  VER1_5);
	EXP_CPUID_BYTE(0x6, 0, ebx, 0x0, VER1_5);
	EXP_CPUID_BYTE(0x6, 0, ecx, 0x0, VER1_5);
	EXP_CPUID_BYTE(0x6, 0, edx, 0x0, VER1_5);


	/* CPUID(0x7) */
	/* CORE_CAPABILITIES(core-capability) */
	EXP_CPUID_BIT(0x7, 0, edx, 30, 0x0, VER1_5);
	/* RDT_M(pqm) */
	EXP_CPUID_BIT(0x7, 0, ebx, 12, 0x0, VER1_5);
	/* RDT_A(rdta) */
	EXP_CPUID_BIT(0x7, 0, ebx, 15, 0x0, VER1_5);
	/* PCONFIG(pconfig) */
	EXP_CPUID_BIT(0x7, 0, edx, 18, 0x0, VER1_5);
	/* TME(tme) */
	EXP_CPUID_BIT(0x7, 0, ecx, 13, 0x0, VER1_5);

	/* CPUID(0x9), enumerated by virtual CPUID(1).ECX[18] */
	EXP_CPUID_BIT(0x1, 0, ecx, 18, 0x0, VER1_5);
	EXP_CPUID_BYTE(0x9, 0, eax, 0x0, VER1_5);

	/* CPUID(0xc), reserved */
	EXP_CPUID_BYTE(0xc, 0, eax, 0x0, VER1_5);
	EXP_CPUID_BYTE(0xc, 0, ebx, 0x0, VER1_5);
	EXP_CPUID_BYTE(0xc, 0, ecx, 0x0, VER1_5);
	EXP_CPUID_BYTE(0xc, 0, edx, 0x0, VER1_5);

	/* CPUID(0xb), Per SDM, CPUID(0x1F, *) is a preferred superset to leaf CPUID(0xB,*) */
	/* The cpuid value is different for different cpu topology.
	 * Test cases for cpu topology must be tested under specified vcpu configuration.
	 * Disable these cases by default, configure cpu topology in QEMU command before enable the cases here
	 */

	/* -smp 16,sockets=1,dies=2,threads=2,cores=4, cpuid of cpu0 */
	/* Uncomment the following cases before testing */
//	EXP_CPUID_BYTE(0xb, 0, eax, 0x1, VER1_5);
//	EXP_CPUID_BYTE(0xb, 0, ebx, 0x2, VER1_5);
//	EXP_CPUID_BYTE(0xb, 0, ecx, 0x0100, VER1_5);
//	EXP_CPUID_BYTE(0xb, 0, edx, 0, VER1_5);
//	EXP_CPUID_BYTE(0xb, 1, eax, 0x4, VER1_5);
//	EXP_CPUID_BYTE(0xb, 1, ebx, 0x10, VER1_5);
//	EXP_CPUID_BYTE(0xb, 1, ecx, 0x0201, VER1_5);
//	EXP_CPUID_BYTE(0xb, 1, edx, 0, VER1_5);
//	EXP_CPUID_BYTE(0xb, 2, eax, 0, VER1_5);
//	EXP_CPUID_BYTE(0xb, 2, ebx, 0, VER1_5);
//	EXP_CPUID_BYTE(0xb, 2, ecx, 0x2, VER1_5);
//	EXP_CPUID_BYTE(0xb, 2, edx, 0, VER1_5);

	/* -smp 12,sockets=1,threads=3,cores=4, cpuid of cpu11 */
	/* Uncomment the following cases before testing */
//	EXP_CPUID_BYTE(0xb, 0, eax, 0x2, VER1_5);
//	EXP_CPUID_BYTE(0xb, 0, ebx, 0x3, VER1_5);
//	EXP_CPUID_BYTE(0xb, 0, ecx, 0x0100, VER1_5);
//	EXP_CPUID_BYTE(0xb, 0, edx, 0xe, VER1_5);
//	EXP_CPUID_BYTE(0xb, 1, eax, 0x4, VER1_5);
//	EXP_CPUID_BYTE(0xb, 1, ebx, 0xc, VER1_5);
//	EXP_CPUID_BYTE(0xb, 1, ecx, 0x0201, VER1_5);
//	EXP_CPUID_BYTE(0xb, 1, edx, 0xe, VER1_5);
//	EXP_CPUID_BYTE(0xb, 2, eax, 0x0, VER1_5);
//	EXP_CPUID_BYTE(0xb, 2, ebx, 0x0, VER1_5);
//	EXP_CPUID_BYTE(0xb, 2, ecx, 0x2, VER1_5);
//	EXP_CPUID_BYTE(0xb, 2, edx, 0xe, VER1_5);


	/* CPUID(0x1f) -smp 16,sockets=1,dies=2,threads=2,cores=4, cpuid of cpu0 */
	/* Uncomment the following cases before testing */
//	EXP_CPUID_BYTE(0x1f, 0, eax, 0x1, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 0, ebx, 0x2, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 0, ecx, 0x0100, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 0, edx, 0, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 1, eax, 0x3, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 1, ebx, 0x8, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 1, ecx, 0x0201, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 1, edx, 0, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 2, eax, 0x4, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 2, ebx, 0x10, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 2, ecx, 0x502, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 2, edx, 0, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 3, eax, 0, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 3, ebx, 0, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 3, ecx, 0x3, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 3, edx, 0, VER1_5);

	/* CPUID(0x1f) -smp 48,sockets=2,dies=2,threads=3,cores=4, cpuid of cpu47 */
	/* Uncomment the following cases before testing */
//	EXP_CPUID_BYTE(0x1f, 0, eax, 0x2, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 0, ebx, 0x3, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 0, ecx, 0x0100, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 0, edx, 0x3e, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 1, eax, 0x4, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 1, ebx, 0xc, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 1, ecx, 0x0201, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 1, edx, 0x3e, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 2, eax, 0x5, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 2, ebx, 0x18, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 2, ecx, 0x0502, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 2, edx, 0x3e, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 3, eax, 0, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 3, ebx, 0, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 3, ecx, 0x3, VER1_5);
//	EXP_CPUID_BYTE(0x1f, 3, edx, 0x3e, VER1_5);

/* Configuration 2 and 3 is disabled by default.
 * Enable or disable the following features in QEMU command before enable the cases here
 */

/* 2. TD_CTLS.REDUCE_VE == 1, FEATURE_PARAVIRT_CTLS == 1 */
//	/* CPUID(0x1) */
//	/* EST(est) */
//	EXP_CPUID_BIT_CTL(0x1, 0, ecx, 7, 0x0, VER1_5, 8, BIT(2)); //not supported by VMM
//	/* TSC_DEADLINE(tsc-deadline) */
//	EXP_CPUID_BIT_CTL(0x1, 0, ecx, 24, 0x1, VER1_5, 8, BIT(11)); //enable it by qemu
//	EXP_CPUID_BIT_CTL(0x1, 0, ecx, 24, 0x0, VER1_5, 8, BIT(11)); //disable it by qemu
//	/* MCA(mce) */
//	EXP_CPUID_BIT_CTL(0x1, 0, edx, 7, 0x1, VER1_5, 8, BIT(3)); //enable it by qemu
//	EXP_CPUID_BIT_CTL(0x1, 0, edx, 7, 0x0, VER1_5, 8, BIT(3)); //disable it by qemu
//	/* MTRR(mtrr) */
//	EXP_CPUID_BIT_CTL(0x1, 0, edx, 12, 0x1, VER1_5, 8, BIT(4)); //enable it by qemu
//	EXP_CPUID_BIT_CTL(0x1, 0, edx, 12, 0x0, VER1_5, 8, BIT(4)); //disable it by qemu
//	/* MCA(mca) */
//	EXP_CPUID_BIT_CTL(0x1, 0, edx, 14, 0x1, VER1_5, 8, BIT(3)); //enable it by qemu
//	EXP_CPUID_BIT_CTL(0x1, 0, edx, 14, 0x0, VER1_5, 8, BIT(3)); //disable it by qemu
//	/* TM(acpi) */
//	EXP_CPUID_BIT_CTL(0x1, 0, edx, 22, 0x0, VER1_5, 8, BIT(8)); //not supported by VMM
//
//	/* CPUID(0x2) */
//	EXP_CPUID_BYTE_CTL(0x2, 0, eax, 0x00feff01, VER1_5, 4, 0);
//	EXP_CPUID_BYTE_CTL(0x5, 0, ebx, 0, VER1_5, 4, 0);
//	EXP_CPUID_BYTE_CTL(0x2, 0, ecx, 0, VER1_5, 4, 0);
//	EXP_CPUID_BYTE_CTL(0x2, 0, edx, 0, VER1_5, 4, 0);
//	/* CPUID(0x7) */
//	/* CORE_CAPABILITIES(core-capability) */
//	EXP_CPUID_BIT_CTL(0x7, 0, edx, 30, 0x1, VER1_5, 8, BIT(0)); //enable it by qemu
//	EXP_CPUID_BIT_CTL(0x7, 0, edx, 30, 0x0, VER1_5, 8, BIT(0)); //disable it by qemu
//	/* RDT_M(pqm) */
//	EXP_CPUID_BIT_CTL(0x7, 0, ebx, 12, 0x0, VER1_5, 8, BIT(7)); //not supported by qemu
//	/* RDT_A(rdta) */
//	EXP_CPUID_BIT_CTL(0x7, 0, ebx, 15, 0x0, VER1_5, 8, BIT(6)); //not supported by qemu
//	/* PCONFIG(pconfig) */
//	EXP_CPUID_BIT_CTL(0x7, 0, edx, 18, 0x0, VER1_5, 8, BIT(5)); //not supported by qemu
//	/* TME(tme) */
//	EXP_CPUID_BIT_CTL(0x7, 0, ecx, 13, 0x0, VER1_5, 8, BIT(10)); //not supported by qemu
//
//	/* CPUID(0x9), enumerated by virtual CPUID(1).ECX[18] */
//	/* DCA(dca) */
//	EXP_CPUID_BIT_CTL(0x1, 0, ecx, 18, 0x0, VER1_5, 8, BIT(1)); //not supported by VMM
//	EXP_CPUID_BYTE_CTL(0x9, 0, eax, 0x0, VER1_5, 8, BIT(1)); //virtual CPUID(1).ECX[18] == 0
//	EXP_CPUID_BYTE_CTL(0x9, 0, ebx, 0x0, VER1_5, 8, BIT(1)); //virtual CPUID(1).ECX[18] == 0
//	EXP_CPUID_BYTE_CTL(0x9, 0, ecx, 0x0, VER1_5, 8, BIT(1)); //virtual CPUID(1).ECX[18] == 0
//	EXP_CPUID_BYTE_CTL(0x9, 0, edx, 0x0, VER1_5, 8, BIT(1)); //virtual CPUID(1).ECX[18] == 0


/* 3. Backward-Compatible -- when #VE reduction is not enabled: TD_CTLS.REDUCE_VE is 0. */
//	/* CPUID(0x1) */
//	/* EST(est) */
//	EXP_CPUID_BIT_CTL(0x1, 0, ecx, 7, 0x0, VER1_5, 0, 0); //not supported by VMM
//	/* TSC_DEADLINE(tsc-deadline) */
//	EXP_CPUID_BIT_CTL(0x1, 0, ecx, 24, 0x1, VER1_5, 0, 0); //enable it by qemu
//	EXP_CPUID_BIT_CTL(0x1, 0, ecx, 24, 0x0, VER1_5, 0, 0); //disable it by qemu
//	/* MCA(mce) */
//	EXP_CPUID_BIT_CTL(0x1, 0, edx, 7, 0x1, VER1_5, 0, 0); //enable it by qemu
//	EXP_CPUID_BIT_CTL(0x1, 0, edx, 7, 0x1, VER1_5, 0, 0); //disable it by qemu
//	/* MTRR(mtrr) */
//	EXP_CPUID_BIT_CTL(0x1, 0, edx, 12, 0x1, VER1_5, 0, 0); //enable it by qemu
//	EXP_CPUID_BIT_CTL(0x1, 0, edx, 12, 0x1, VER1_5, 0, 0); //disable it by qemu
//	/* MCA(mca) */
//	EXP_CPUID_BIT_CTL(0x1, 0, edx, 14, 0x1, VER1_5, 0, 0); //enable it by qemu
//	EXP_CPUID_BIT_CTL(0x1, 0, edx, 14, 0x1, VER1_5, 0, 0); //disable it by qemu
//	/* TM(acpi) */
//	EXP_CPUID_BIT_CTL(0x1, 0, edx, 22, 0x0, VER1_5, 0, 0); //not supported by VMM
//
//	/* CPUID(0x2) */
//	EXP_CPUID_BYTE_CTL(0x2, 0, eax, 0, VER1_5, 0, 0); //trigger #VE
//
//	/* CPUID(0x6) */
//	EXP_CPUID_BIT_CTL(0x6, 0, eax, 2, 0x0, VER1_5, 0, 0);//trigger #VE
//
//	/* CPUID(0x7) */
//	/* CORE_CAPABILITIES(core-capability) */
//	EXP_CPUID_BIT_CTL(0x7, 0, edx, 30, 0x1, VER1_5, 0, 0); //enable it by qemu
//	EXP_CPUID_BIT_CTL(0x7, 0, edx, 30, 0x1, VER1_5, 0, 0); //disable it by qemu
//	/* RDT_M(pqm) */
//	EXP_CPUID_BIT_CTL(0x7, 0, ebx, 12, 0x0, VER1_5, 0, 0); //not supported by qemu
//	/* RDT_A(rdta) */
//	EXP_CPUID_BIT_CTL(0x7, 0, ebx, 15, 0x0, VER1_5, 0, 0); //not supported by qemu
//	/* PCONFIG(pconfig) */
//	EXP_CPUID_BIT_CTL(0x7, 0, edx, 18, 0x0, VER1_5, 0, 0); //not supported by qemu
//	/* TME(tme) */
//	EXP_CPUID_BIT_CTL(0x7, 0, ecx, 13, 0x0, VER1_5, 0, 0); //not supported by qemu
//
//	/* CPUID(0x9), enumerated by virtual CPUID(1).ECX[18] */
//	/* DCA(dca) */
//	EXP_CPUID_BIT_CTL(0x1, 0, ecx, 18, 0x0, VER1_5, 0, 0); //not supported by VMM
//	EXP_CPUID_BYTE_CTL(0x9, 0, eax, 0x0, VER1_5, 0, 0); //VM_CTLS.CPUID_VE_DISABLE is false, trigger #VE
//
//	/* CPUID(0xb), Per SDM, CPUID(0x1F, *) is a preferred superset to leaf CPUID(0xB,*) */
//	EXP_CPUID_BYTE_CTL(0xb, 0, eax, 0, VER1_5, 0, 0); //trigger #VE
//
//	/* CPUID(0xc) Reserved */
//	EXP_CPUID_BYTE_CTL(0xc, 0, eax, 0, VER1_5, 0, 0); //trigger #VE
//
//	/* CPUID(0x1f) */
//	EXP_CPUID_BYTE_CTL(0x1f, 0, eax, 0, VER1_5, 0, 0); //trigger #VE
}
#endif
