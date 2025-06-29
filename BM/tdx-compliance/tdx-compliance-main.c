// SPDX-License-Identifier: GPL-2.0-only
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/list.h>

#include <linux/kprobes.h>

#include "asm/trapnr.h"

#include "tdx-compliance.h"
#include "tdx-compliance-cpuid.h"
#include "tdx-compliance-cr.h"
#include "tdx-compliance-msr.h"

MODULE_AUTHOR("Yi Sun");

/*
 * Global Variables Summary:
 * - stat_total: Count the total number of cases of TDX compliance tests.
 *
 * - stat_pass: Count the number of cases in TDX compliance tests that
 *              passed the test according to the TDX Architecture
 *              Specification.
 *
 * - stat_fail: Count the number of cases in TDX compliance tests that
 *              failed to pass the test according to the TDX Architecture
 *              Specification.
 *
 * - cnt_log: Count the length of logs.
 *
 */
int stat_total, stat_pass, stat_fail, cnt_log;
int operation;
int spec_version;
char case_name[256];
char version_name[32];
char *buf_ret;
bool kretprobe_switch;
static struct dentry *f_tdx_tests, *d_tdx;
static u64 tdcs_td_ctl;
static u64 tdcs_feature_pv_ctl;
LIST_HEAD(cpuid_list);

#define SIZE_BUF		(PAGE_SIZE << 3)
#define pr_buf(fmt, ...)				\
	(cnt_log += sprintf(buf_ret + cnt_log, fmt, ##__VA_ARGS__))\

#define pr_tdx_tests(fmt, ...)				\
	pr_info("%s: " pr_fmt(fmt),			\
		module_name(THIS_MODULE), ##__VA_ARGS__)\

#define CR_ERR_INFO					\
	"Error: CR compliance test failed,"

#define MSR_ERR_INFO			\
	"Error: MSR compliance test failed,"

#define MSR_MULTIBYTE_ERR_INFO		\
	"Error: MSR multiple bytes difference,"

#define PROCFS_NAME		"tdx-tests"
#define OPMASK_CPUID		1
#define OPMASK_CR		2
#define OPMASK_MSR		4
#define OPMASK_KRETKPROBE       8
#define OPMASK_UNREGISTER       16
#define TRIGGER_CPUID           32
#define OPMASK_DUMP			0x800
#define OPMASK_SINGLE		0x8000

#define CPUID_DUMP_PATTERN	\
	"eax(%08x) ebx(%08x) ecx(%08x) edx(%08x)\n"

static char *result_str(int ret)
{
	switch (ret) {
	case 1:
		return "PASS";
	case 0:
		return "NRUN";
	case -1:
		return "FAIL";
	}

	return "UNKNOWN";
}

void parse_version(void)
{
	if (strstr(version_name, "1.0"))
		spec_version = VER1_0;
	else if (strstr(version_name, "1.5"))
		spec_version = VER1_5;
	else if (strstr(version_name, "2.0"))
		spec_version = VER2_0;
	else
		spec_version = (VER1_0 | VER1_5 | VER2_0);
}

static char *case_version(int ret)
{
	switch (ret) {
	case VER1_0:
		return "1.0";
	case VER1_5:
		return "1.5";
	case VER2_0:
		return "2.0";
	}

	return "";
}

void parse_input(char *s)
{
	memset(case_name, 0, sizeof(case_name));
	memset(version_name, 0, sizeof(version_name));
	char *space = strchr(s, ' ');

	if (space) {
		size_t length_case = space - s;

		strncpy(case_name, s, length_case);
		case_name[length_case] = '\0';

		size_t length_ver = strlen(space + 1);

		strncpy(version_name, space + 1, length_ver);
	} else {
		strcpy(case_name, s);
		strcpy(version_name, "generic");
	}

	parse_version();
}

static int check_results_msr(struct test_msr *t)
{
		if (t->excp.expect == t->excp.val)
			return 1;

		pr_buf(MSR_ERR_INFO "exception %d, but expect_exception %d\n",
		       t->excp.val, t->excp.expect);
		return -1;
}

static int run_cpuid(struct test_cpuid *t)
{
	int cpu_id = smp_processor_id();

	t->regs.eax.val = t->leaf;
	t->regs.ecx.val = t->subleaf;
	tdcs_td_ctl = t->tdcs_td_ctl;
	tdcs_feature_pv_ctl = t->tdcs_feature_pv_ctl;
	setup_tdcs_ctl();
	__cpuid(&t->regs.eax.val, &t->regs.ebx.val, &t->regs.ecx.val, &t->regs.edx.val);
	pr_info("From CPU core %d\n", cpu_id);

	return 0;
}

static int _native_read_msr(unsigned int msr, u64 *val)
{
	int err;
	u32 low = 0, high = 0;

	asm volatile("1: rdmsr ; xor %[err],%[err]\n"
		     "2:\n\t"
		     _ASM_EXTABLE_TYPE_REG(1b, 2b, EX_TYPE_FAULT, %[err])
		     : [err] "=r" (err), "=a" (low), "=d" (high)
		     : "c" (msr));

	*val = ((low) | (u64)(high) << 32);
	if (err)
		err = (int)low;
	return err;
}

static int _native_write_msr(unsigned int msr, u64 *val)
{
	int err;
	u32 low = (u32)(*val), high = (u32)((*val) >> 32);

	asm volatile("1: wrmsr ; xor %[err],%[err]\n"
		     "2:\n\t"
		     _ASM_EXTABLE_TYPE_REG(1b, 2b, EX_TYPE_FAULT, %[err])
		     : [err] "=a" (err)
		     : "c" (msr), "0" (low), "d" (high)
		     : "memory");

	return err;
}

static int read_msr_native(struct test_msr *c)
{
	int i, err, tmp;

	err = _native_read_msr((u32)(c->msr.msr_num), &c->msr.val.q);

	for (i = 1; i < c->size; i++) {
		tmp = _native_read_msr((u32)(c->msr.msr_num) + i, &c->msr.val.q);

		if (err != tmp) {
			pr_buf(MSR_MULTIBYTE_ERR_INFO
			       "MSR(%x): %d(byte0) and %d(byte%d)\n",
			       c->msr.msr_num, err, tmp, i);
			return -1;
		}
	}
	return err;
}

static int write_msr_native(struct test_msr *c)
{
	int i, err, tmp;

	err = _native_write_msr((u32)(c->msr.msr_num), &c->msr.val.q);

	for (i = 1; i < c->size; i++) {
		tmp = _native_write_msr((u32)(c->msr.msr_num) + i, &c->msr.val.q);

		if (err != tmp) {
			pr_buf(MSR_MULTIBYTE_ERR_INFO
			       "MSR(%x): %d(byte0) and %d(byte%d)\n",
			       c->msr.msr_num, err, tmp, i);
			return -1;
		}
	}
	return err;
}

static int run_all_msr(void)
{
	struct test_msr *t = msr_cases;
	int i = 0;

	pr_tdx_tests("Testing MSR...\n");

	for (i = 0; i < ARRAY_SIZE(msr_cases); i++, t++) {
		if (operation & 0x8000 && strcmp(case_name, t->name) != 0)
			continue;

		if (!(spec_version & t->version))
			continue;

		if (operation & 0x800) {
			pr_buf("%s %s\n", t->name, case_version(t->version));
			continue;
		}

		if (t->pre_condition)
			t->pre_condition(t);
		if (t->run_msr_rw)
			t->excp.val = t->run_msr_rw(t);

		t->ret = check_results_msr(t);
		t->ret == 1 ? stat_pass++ : stat_fail++;

		pr_buf("%d: %s_%s:\t %s\n", ++stat_total, t->name, version_name,
		       result_str(t->ret));
	}
	return 0;
}

static int check_results_cpuid(struct test_cpuid *t)
{
	if (t->regs.eax.mask == 0 && t->regs.ebx.mask == 0 &&
	    t->regs.ecx.mask == 0 && t->regs.edx.mask == 0)
		return 0;

	if (t->regs.eax.expect == (t->regs.eax.val & t->regs.eax.mask) &&
	    t->regs.ebx.expect == (t->regs.ebx.val & t->regs.ebx.mask) &&
	    t->regs.ecx.expect == (t->regs.ecx.val & t->regs.ecx.mask) &&
	    t->regs.edx.expect == (t->regs.edx.val & t->regs.edx.mask))
		return 1;

	/*
	 * Show the detail that results in the failure,
	 * CPUID here focus on the fixed bit, not actual cpuid val.
	 */
	pr_buf("CPUID: %s_%s\n", t->name, version_name);
	pr_buf("CPUID	 :" CPUID_DUMP_PATTERN,
	       (t->regs.eax.val & t->regs.eax.mask), (t->regs.ebx.val & t->regs.ebx.mask),
	       (t->regs.ecx.val & t->regs.ecx.mask), (t->regs.edx.val & t->regs.edx.mask));

	pr_buf("CPUID exp:" CPUID_DUMP_PATTERN,
	       t->regs.eax.expect, t->regs.ebx.expect,
	       t->regs.ecx.expect, t->regs.edx.expect);

	pr_buf("CPUID msk:" CPUID_DUMP_PATTERN,
	       t->regs.eax.mask, t->regs.ebx.mask,
	       t->regs.ecx.mask, t->regs.edx.mask);
	return -1;
}

int check_results_cr(struct test_cr *t)
{
	t->reg.val &= t->reg.mask;
	if (t->reg.val == (t->reg.mask * t->reg.expect) &&
	    t->excp.expect == t->excp.val)
		return 1;

	pr_buf(CR_ERR_INFO "output/exception %llx/%d, but expect %llx/%d\n",
	       t->reg.val, t->excp.val, t->reg.expect, t->excp.expect);
	return -1;
}

static int run_all_cpuid(void)
{
	struct test_cpuid *t;

	pr_tdx_tests("Testing CPUID start\n");
	list_for_each_entry(t, &cpuid_list, list) {
		pr_info("New case start ...");
		if (operation & 0x8000 && strcmp(case_name, t->name) != 0)
			continue;

		if (!(spec_version & t->version))
			continue;

		if (operation & 0x800) {
			pr_buf("%s %s\n", t->name, case_version(t->version));
			continue;
		}
		if (kretprobe_switch)
			pr_info("leaf:%X subleaf:%X\n", t->leaf, t->subleaf);

		run_cpuid(t);

		t->ret = check_results_cpuid(t);
		if (t->ret == 1)
			stat_pass++;
		else if (t->ret == -1)
			stat_fail++;
		if (kretprobe_switch)
			pr_info("CPUID output: eax:%X, ebx:%X, ecx:%X, edx:%X\n",
				t->regs.eax.val, t->regs.ebx.val, t->regs.ecx.val, t->regs.edx.val);

		pr_buf("%d: %s_%s:\t %s\n",
			++stat_total, t->name, version_name, result_str(t->ret));
		pr_info("Case end!");
	}
	pr_tdx_tests("CPUID test end!\n");
	return 0;
}

static u64 get_cr0(void)
{
	u64 cr0;

	asm volatile("mov %%cr0,%0\n\t" : "=r" (cr0) : __FORCE_ORDER);

	return cr0;
}

static u64 get_cr4(void)
{
	u64 cr4;

	asm volatile("mov %%cr4,%0\n\t" : "=r" (cr4) : __FORCE_ORDER);

	return cr4;
}

int _native_write_cr0(u64 val)
{
	int err;

	asm volatile("1: mov %1,%%cr0; xor %[err],%[err]\n"
		     "2:\n\t"
		     _ASM_EXTABLE_TYPE_REG(1b, 2b, EX_TYPE_FAULT, %[err])
		     : [err] "=a" (err)
		     : "r" (val)
		     : "memory");
	return err;
}

int _native_write_cr4(u64 val)
{
	int err;

	asm volatile("1: mov %1,%%cr4; xor %[err],%[err]\n"
		     "2:\n\t"
		     _ASM_EXTABLE_TYPE_REG(1b, 2b, EX_TYPE_FAULT, %[err])
		     : [err] "=a" (err)
		     : "r" (val)
		     : "memory");
	return err;
}

static int run_all_cr(void)
{
	struct test_cr *t;
	int i = 0;

	t = cr_list;
	pr_tdx_tests("Testing Control Register...\n");

	for (i = 0; i < ARRAY_SIZE(cr_list); i++, t++) {
		if (operation & 0x8000 && strcmp(case_name, t->name) != 0)
			continue;

		if (!(spec_version & t->version))
			continue;

		if (operation & 0x800) {
			pr_buf("%s %s\n", t->name, case_version(t->version));
			continue;
		}

		if (t->run_cr_get)
			t->reg.val = t->run_cr_get();

		if (t->run_cr_set) {
			if (t->pre_condition) {
				if (t->pre_condition(t) != 0) {
					pr_buf("%d: %s:\t %s\n",
					       ++stat_total, t->name, "SKIP");
					continue;
				}
			}

			t->excp.val = t->run_cr_set(t->reg.mask);
		}

		t->ret = check_results_cr(t);
		t->ret == 1 ? stat_pass++ : stat_fail++;

		pr_buf("%d: %s_%s:\t %s\n", ++stat_total, t->name, version_name,
		       result_str(t->ret));
	}
	return 0;
}

unsigned int count;
static int ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	int retval = regs_return_value(regs);

	count++;
	pr_info("handle_cpuid count %d\n", count);
	pr_info("handle_cpuid returned %d\n", retval);
	if (retval < 0)
		pr_info("#GP trigger.\n");
	else
		pr_info("#VE trigger.\n");

	return 0;
}

static struct kretprobe my_kretprobe = {
	.handler = ret_handler,
	/*
	 * Here can modify the detection functions, such as hanlde_cpuid, read_msr,
	 * write_msr, handle_mmio, handle_io, etc.
	 * It should be noted that the detected function must be exposed to the kernel,
	 * that is, the address corresponding to the function needs to be in /proc/kallsyms.
	 */
	.kp.symbol_name = "handle_cpuid",
};

static int run_kretprobe(void)
{
	// Register the kretprobe.
	int ret;

	ret = register_kretprobe(&my_kretprobe);
	if (ret < 0) {
		pr_err("register_kprobe failed, returned %d\n", ret);
		return ret;
	}
	kretprobe_switch = true;
	pr_info("Detect the return value of the %s.\n", my_kretprobe.kp.symbol_name);
	pr_info("Planted kprobe at %p\n", my_kretprobe.kp.addr);

	return 0;
}

static int unregister(void)
{
	// Unregister the kretprobe.
	unregister_kretprobe(&my_kretprobe);
	kretprobe_switch = false;
	pr_info("kprobe at %p unregistered. Please rmmod tdx_compliance.\n", my_kretprobe.kp.addr);
	return 0;
}

static int trigger_cpuid(unsigned int *A, unsigned int *B, unsigned int *C, unsigned int *D)
{
	pr_info("CPUID leaf:%X, subleaf:%X\n", *A, *C);
	__asm__ volatile("cpuid"
			: "=a" (*A), "=b" (*B), "=c" (*C), "=d" (*D)
			: "a" (*A), "c"(*C)
			:);
	pr_info("CPUID output: eax:%X, ebx:%X, ecx:%X, edx:%X\n", *A, *B, *C, *D);
	return 0;
}

static ssize_t
tdx_tests_proc_read(struct file *file, char __user *buffer,
		    size_t count, loff_t *ppos)
{
	return simple_read_from_buffer(buffer, count, ppos, buf_ret, cnt_log);
}

static ssize_t
tdx_tests_proc_write(struct file *file,
		     const char __user *buffer,
		     size_t count, loff_t *f_pos)
{
	char *str_input;

	str_input = kzalloc((count + 1), GFP_KERNEL);

	if (!str_input)
		return -ENOMEM;

	if (copy_from_user(str_input, buffer, count)) {
		kfree(str_input);
		return -EFAULT;
	}

	if (*(str_input + strlen(str_input) - 1) == '\n')
		*(str_input + strlen(str_input) - 1) = '\0';

	parse_input(str_input);

	if (strncmp(case_name, "cpuid", 5) == 0)
		operation |= OPMASK_CPUID;
	else if (strncmp(case_name, "cr", 2) == 0)
		operation |= OPMASK_CR;
	else if (strncmp(case_name, "msr", 3) == 0)
		operation |= OPMASK_MSR;
	else if (strncmp(case_name, "all", 3) == 0)
		operation |= OPMASK_CPUID | OPMASK_CR | OPMASK_MSR;
	else if (strncmp(case_name, "list", 4) == 0)
		operation |= OPMASK_DUMP | OPMASK_CPUID | OPMASK_CR | OPMASK_MSR;
	else if (strncmp(case_name, "kretprobe", 9) == 0)
		operation |= OPMASK_KRETKPROBE;
	else if (strncmp(case_name, "unregister", 10) == 0)
		operation |= OPMASK_UNREGISTER;
	else if (strncmp(case_name, "trigger_cpuid", 13) == 0)
		operation |= TRIGGER_CPUID;
	else
		operation |= OPMASK_SINGLE | OPMASK_CPUID | OPMASK_CR | OPMASK_MSR;

	cnt_log = 0;
	stat_total = 0;
	stat_pass = 0;
	stat_fail = 0;

	memset(buf_ret, 0, SIZE_BUF);

	if (operation & OPMASK_CPUID)
		run_all_cpuid();
	if (operation & OPMASK_CR)
		run_all_cr();
	if (operation & OPMASK_MSR)
		run_all_msr();
	if (operation & OPMASK_KRETKPROBE)
		run_kretprobe();
	if (operation & OPMASK_UNREGISTER)
		unregister();
	if (operation & TRIGGER_CPUID) {
		unsigned int A, B, C, D;

		if (sscanf(version_name, "%x %x %x %x", &A, &B, &C, &D) == 4)
			trigger_cpuid(&A, &B, &C, &D);
		else
			pr_info("Error parsing input string.\n");
	}

	if (!(operation & OPMASK_DUMP))
		pr_buf("Total:%d, PASS:%d, FAIL:%d, SKIP:%d\n",
			stat_total, stat_pass, stat_fail,
			stat_total - stat_pass - stat_fail);

	kfree(str_input);
	operation = 0;
	return count;
}

const struct file_operations data_file_fops = {
	.owner = THIS_MODULE,
	.write = tdx_tests_proc_write,
	.read = tdx_tests_proc_read,
};

u64 __tdx_hypercall(struct tdx_module_args *args)
{
	/*
	 * For TDVMCALL explicitly set RCX to the bitmap of shared registers.
	 * The caller isn't expected to set @args->rcx anyway.
	 */
	args->rcx = TDVMCALL_EXPOSE_REGS_MASK;

	/*
	 * Failure of __tdcall_saved_ret() indicates a failure of the TDVMCALL
	 * mechanism itself and that something has gone horribly wrong with
	 * the TDX module, so panic.
	 */
	if (__tdcall_saved_ret(TDG_VP_VMCALL, args))
		abort();

	if (args->r10)
		pr_info("%s err:\n"
			"R10=0x%016llx, R11=0x%016llx, R12=0x%016llx\n"
			"R13=0x%016llx, R14=0x%016llx, R15=0x%016llx\n",
			__func__, args->r10, args->r11, args->r12, args->r13, args->r14,
			args->r15);

	/* TDVMCALL leaf return code is in R10 */
	return args->r10;
}

u64 tdcall(u64 fn, struct tdx_module_args *args)
{
	u64 r;

	r = __tdcall_ret(fn, args);
	if (r)
		panic("TDCALL %lld failed (Buggy TDX module!)\n", fn);
	return r;
}

u64 tdg_vm_read(u64 field, u64 *val)
{
	u64 ret;
	struct tdx_module_args arg = {
		.rcx = 0,
		.rdx = field,
	};

	ret =  __tdcall_ret(TDG_VM_RD, &arg);
	if (!ret)
		*val = arg.r8;

	return ret;
}

u64 tdg_vm_write(u64 field, u64 val, u64 mask)
{
	struct tdx_module_args arg = {
		.rcx = 0,
		.rdx = field,
		.r8 = val,
		.r9 = mask,
	};

	return tdcall(TDG_VM_WR, &arg);
}

static void setup_tdcs_ctl(void)
{
	struct tdx_module_args arg;
	u64 r;
	u64 v;
	u64 mask;
	bool ve_reduce;
	bool vcpu_toplgy;

	arg.rdx = TDX_GLOBAL_FIELD_FEATURES0;
	r = tdcall(TDG_SYS_RD, &arg);
	if (r) {
		pr_info("WARN: Failed to get TDX FEATURES0, err:0x%llx\n", r);
		return;
	}

	pr_info("TDX FEATURES0:0x%llx\n", arg.r8);

	r = tdg_vm_read(TDX_TDCS_FIELD_TD_CTL, &v);
	if (!r)
		pr_info("TDX_TDCS_FILED_TD_CTL before set:0x%llx\n", v);
	else
		pr_info("Failed to read TDX_TDCS_FILED_TD_CTL\n");

	r = tdg_vm_read(TDX_TDCS_FIELD_FEATURE_PV_CTL, &v);
	if (!r)
		pr_info("TDX_TDCS_FIELD_FEATURE_PV_CTL before set:0x%llx\n", v);
	else
		pr_info("Failed to read TDX_TDCS_FIELD_FEATURE_PV_CTL\n");

	ve_reduce = arg.r8 & MD_FIELD_ID_FEATURES0_VE_REDUCE;
	vcpu_toplgy = arg.r8 & MD_FIELD_ID_FEATURES0_VCPU_TPLGY;
	if ((ve_reduce || vcpu_toplgy) && tdcs_td_ctl != 0xbad) {
		mask = tdcs_td_ctl ? tdcs_td_ctl : 0x800000000000000eULL;
		r = tdg_vm_write(TDX_TDCS_FIELD_TD_CTL, tdcs_td_ctl, mask);
		if (!r)
			pr_info("TDX_TDCS_FILED_TD_CTL set to 0x%llx\n", tdcs_td_ctl);
		else
			pr_info("Failed to set TDX_TDCS_FILED_TD_CTL set to 0x%llx, err:0x%llx\n",
				tdcs_td_ctl, r);
	} else {
		pr_info("TDX_TDCS_FILED_TD_CTL is not supported or not set (0x%llx)\n",
			tdcs_td_ctl);
	}

	if (ve_reduce && tdcs_td_ctl != 0xbad) {
		mask = tdcs_feature_pv_ctl ? tdcs_feature_pv_ctl : -1ULL;
		r = tdg_vm_write(TDX_TDCS_FIELD_FEATURE_PV_CTL,
				 tdcs_feature_pv_ctl, mask);
		if (!r)
			pr_info("TDX_TDCS_FIELD_FEATURE_PV_CTL set to 0x%llx\n",
					tdcs_feature_pv_ctl);
		else
			pr_info("Failed to set TDX_TDCS_FIELD_FEATURE_PV_CTL 0x%llx, err:0x%llx\n",
				tdcs_feature_pv_ctl, r);
	} else {
		pr_info("TDX_TDCS_FIELD_FEATURE_PV_CTL is not supported or not set (tdcs_ctl=0x%llx)\n",
			tdcs_td_ctl);
	}

	r = tdg_vm_read(TDX_TDCS_FIELD_TD_CTL, &v);
	if (!r)
		pr_info("TDX_TDCS_FILED_TD_CTL after set:0x%llx\n", v);
	else
		pr_info("Failed to read TDX_TDCS_FILED_TD_CTL\n");

	r = tdg_vm_read(TDX_TDCS_FIELD_FEATURE_PV_CTL, &v);
	if (!r)
		pr_info("TDX_TDCS_FIELD_FEATURE_PV_CTL after set:0x%llx\n", v);
	else
		pr_info("Failed to read TDX_TDCS_FIELD_FEATURE_PV_CTL\n");
}

static int __init tdx_tests_init(void)
{
	d_tdx = debugfs_create_dir("tdx", NULL);
	if (!d_tdx)
		return -ENOENT;

	f_tdx_tests = debugfs_create_file(PROCFS_NAME, 0644, d_tdx, NULL,
					  &data_file_fops);

	if (!f_tdx_tests) {
		debugfs_remove_recursive(d_tdx);
		return -ENOENT;
	}

	buf_ret = kzalloc(SIZE_BUF, GFP_KERNEL);
	if (!buf_ret)
		return -ENOMEM;

	initial_cpuid();

	cur_cr0 = get_cr0();
	cur_cr4 = get_cr4();
	pr_buf("cur_cr0: %016llx, cur_cr4: %016llx\n", cur_cr0, cur_cr4);

	return 0;
}

static void __exit tdx_tests_exit(void)
{
	struct test_cpuid *t, *tmp;

	list_for_each_entry_safe(t, tmp, &cpuid_list, list) {
		list_del(&t->list);
		kfree(t);
	}
	kfree(buf_ret);
	debugfs_remove_recursive(d_tdx);
	pr_info("The tdx_compliance module has been removed.\n");
}

module_init(tdx_tests_init);
module_exit(tdx_tests_exit);
MODULE_LICENSE("GPL");
