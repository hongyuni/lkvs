#!/bin/bash
# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2023 Intel Corporation

# Author:   Hongyu Ning <hongyu.ning@intel.com>
#
# History:  5, Dec., 2023 - Hongyu Ning - creation


# @desc This script prepare and run $TESTCASE in for $FEATURE tdx
# in tdx VM; please note, $FEATURE name and subfolder name under guest-test
# must be the exactly same, here FEATURE=tdx

###################### Variables ######################
## common variables example ##
local SCRIPT_DIR_LOCAL
SCRIPT_DIR_LOCAL="$( cd "$( dirname "$0" )" && pwd )"
echo "$SCRIPT_DIR_LOCAL"
# get test scenario config for $FEATURE tdx test_executor
source "$SCRIPT_DIR_LOCAL"/../test_params.py
## end of common variables example ##

###################### Functions ######################
## $FEATURE specific Functions ##
guest_attest_test() {
  selftest_item=$1
  guest_test_prepare tdx_attest_check.sh
  guest_test_source_code tdx_attest_test_suite tdx_guest_test || \
  die "Failed to prepare guest test source code for $selftest_item"
  guest_test_entry tdx_attest_check.sh "-t $selftest_item" || \
  die "Failed on $TESTCASE tdx_attest_check.sh -t $selftest_item"
  [[ "$GCOV" == "off" ]] && guest_test_close
}

guest_tsm_attest() {
  test_item=$1
  guest_test_prepare tdx_attest_check.sh
  guest_test_entry tdx_attest_check.sh "-t $test_item" || \
  die "Failed on $TESTCASE tdx_attest_check.sh -t $test_item"
  [[ "$GCOV" == "off" ]] && guest_test_close
}

###################### Do Works ######################
## common works example ## 
cd "$(dirname "$0")" 2>/dev/null || exit 1
source ../../.env

# get test_executor common functions:
# function based on sshpass to scp common.sh and $1 test_script.sh to Guest VM
## guest_test_prepare
# function based on sshpass to scp $1 source_code_dir and compile $2 test_binary in Guest VM
## guest_test_source_code
# function based on sshpass to execute $1 test_script.sh and potential $2 script params in Guest VM
## guest_test_entry
# function based on sshpass to close VM
## guest_test_close
source "$SCRIPT_DIR"/guest.test_executor.sh

cd "$SCRIPT_DIR_LOCAL" || die "fail to switch to $SCRIPT_DIR_LOCAL"
## end of common works example ##

## $FEATURE specific code path ##
# select test_functions by $TESTCASE
case "$TESTCASE" in
  TD_BOOT)
    guest_test_prepare tdx_guest_boot_check.sh
    guest_test_entry tdx_guest_boot_check.sh "-v $VCPU -s $SOCKETS -m $MEM" || \
    die "Failed on TD_BOOT test tdx_guest_boot_check.sh -v $VCPU -s $SOCKETS -m $MEM"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  GUEST_TESTCASE_EXAMPLE)
    guest_test_prepare guest_test.sh
    guest_test_source_code test_source_code_dir_example test_binary_example
    guest_test_entry guest_test.sh "-t $TESTCASE" || \
    die "Failed on $TESTCASE guest_test.sh -t $TESTCASE"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  TD_ATTEST_VERIFY_REPORT)
    guest_attest_test "global.verify_report" || \
    die "Failed on $TESTCASE"
    ;;
  TD_ATTEST_VERITY_REPORTMAC)
    guest_attest_test "global.verify_reportmac" || \
    die "Failed on $TESTCASE"
    ;;
  TD_ATTEST_VERIFY_RTMR_EXTEND)
    guest_attest_test "global.verify_rtmr_extend" || \
    die "Failed on $TESTCASE"
    ;;
  TD_ATTEST_VERIFY_QUOTE)
    guest_attest_test "global.verify_quote" || \
    die "Failed on $TESTCASE"
    ;;
  TD_NET_SPEED)
    guest_test_prepare tdx_speed_test.sh
    guest_test_entry tdx_speed_test.sh || \
    die "Failed on TD_NET_SPEED tdx_speed_test.sh"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  TD_TSM_ATTEST_QUOTE_PRECHECK)
    guest_tsm_attest "tsm.get_quote.precheck" || \
    die "Failed on $TESTCASE"
    ;;
  TD_TSM_ATTEST_QUOTE)
    guest_tsm_attest "tsm.get_quote" || \
    die "Failed on $TESTCASE"
    ;;
  TD_TSM_ATTEST_QUOTE_NEG)
    guest_tsm_attest "tsm.get_quote.negative" || \
    die "Failed on $TESTCASE"
    ;;
  TD_MEM_EBIZZY_FUNC)
    guest_test_prepare tdx_mem_test.sh
    guest_test_source_code tdx_ebizzy_test_suite ebizzy || \
    die "Failed to prepare guest test source code of tdx_ebizzy_test_suite"
    guest_test_entry tdx_mem_test.sh "-t EBIZZY_FUNC" || \
    die "Failed on $TESTCASE tdx_mem_test.sh -t EBIZZY_FUNC"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  TD_MEM_ACPT_T_1C_8G_1W)
    guest_test_prepare tdx_mem_test.sh
    guest_test_entry tdx_mem_test.sh "-t MEM_ACPT_T_1C_8G_1W" || \
    die "Failed on $TESTCASE tdx_mem_test.sh -t MEM_ACPT_T_1C_8G_1W"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  TD_MEM_ACPT_T_1C_8G_32W)
    guest_test_prepare tdx_mem_test.sh
    guest_test_entry tdx_mem_test.sh "-t MEM_ACPT_T_1C_8G_32W" || \
    die "Failed on $TESTCASE tdx_mem_test.sh -t MEM_ACPT_T_1C_8G_32W"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  TD_MEM_ACPT_T_1C_32G_1W)
    guest_test_prepare tdx_mem_test.sh
    guest_test_entry tdx_mem_test.sh "-t MEM_ACPT_T_1C_32G_1W" || \
    die "Failed on $TESTCASE tdx_mem_test.sh -t MEM_ACPT_T_1C_32G_1W"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  TD_MEM_ACPT_T_1C_32G_32W)
    guest_test_prepare tdx_mem_test.sh
    guest_test_entry tdx_mem_test.sh "-t MEM_ACPT_T_1C_32G_32W" || \
    die "Failed on $TESTCASE tdx_mem_test.sh -t MEM_ACPT_T_1C_32G_32W"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  TD_MEM_ACPT_T_1C_96G_1W)
    guest_test_prepare tdx_mem_test.sh
    guest_test_entry tdx_mem_test.sh "-t MEM_ACPT_T_1C_96G_1W" || \
    die "Failed on $TESTCASE tdx_mem_test.sh -t MEM_ACPT_T_1C_96G_1W"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  TD_MEM_ACPT_T_1C_96G_32W)
    guest_test_prepare tdx_mem_test.sh
    guest_test_entry tdx_mem_test.sh "-t MEM_ACPT_T_1C_96G_32W" || \
    die "Failed on $TESTCASE tdx_mem_test.sh -t MEM_ACPT_T_1C_96G_32W"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  TD_MEM_ACPT_T_32C_8G_32W)
    guest_test_prepare tdx_mem_test.sh
    guest_test_entry tdx_mem_test.sh "-t MEM_ACPT_T_32C_8G_32W" || \
    die "Failed on $TESTCASE tdx_mem_test.sh -t MEM_ACPT_T_32C_8G_32W"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  TD_MEM_ACPT_T_32C_8G_256W)
    guest_test_prepare tdx_mem_test.sh
    guest_test_entry tdx_mem_test.sh "-t MEM_ACPT_T_32C_8G_256W" || \
    die "Failed on $TESTCASE tdx_mem_test.sh -t MEM_ACPT_T_32C_8G_256W"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  TD_MEM_ACPT_T_32C_32G_32W)
    guest_test_prepare tdx_mem_test.sh
    guest_test_entry tdx_mem_test.sh "-t MEM_ACPT_T_32C_32G_32W" || \
    die "Failed on $TESTCASE tdx_mem_test.sh -t MEM_ACPT_T_32C_32G_32W"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  TD_MEM_ACPT_T_32C_32G_256W)
    guest_test_prepare tdx_mem_test.sh
    guest_test_entry tdx_mem_test.sh "-t MEM_ACPT_T_32C_32G_256W" || \
    die "Failed on $TESTCASE tdx_mem_test.sh -t MEM_ACPT_T_32C_32G_256W"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  TD_MEM_ACPT_T_32C_96G_32W)
    guest_test_prepare tdx_mem_test.sh
    guest_test_entry tdx_mem_test.sh "-t MEM_ACPT_T_32C_96G_32W" || \
    die "Failed on $TESTCASE tdx_mem_test.sh -t MEM_ACPT_T_32C_96G_32W"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  TD_MEM_ACPT_T_32C_96G_256W)
    guest_test_prepare tdx_mem_test.sh
    guest_test_entry tdx_mem_test.sh "-t MEM_ACPT_T_32C_96G_256W" || \
    die "Failed on $TESTCASE tdx_mem_test.sh -t MEM_ACPT_T_32C_96G_256W"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  TD_MEM_ACPT_FUNC)
    guest_test_prepare tdx_mem_test.sh
    guest_test_entry tdx_mem_test.sh "-t MEM_ACPT_FUNC" || \
    die "Failed on $TESTCASE tdx_mem_test.sh -t MEM_ACPT_FUNC"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  TD_MEM_ACPT_CAL)
    guest_test_prepare tdx_mem_test.sh
    guest_test_entry tdx_mem_test.sh "-t MEM_ACPT_CAL" || \
    die "Failed on $TESTCASE tdx_mem_test.sh -t MEM_ACPT_CAL"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  TD_MEM_ACPT_NEG)
    guest_test_prepare tdx_mem_test.sh
    guest_test_entry tdx_mem_test.sh "-t MEM_ACPT_NEG" || \
    die "Failed on $TESTCASE tdx_mem_test.sh -t MEM_ACPT_NEG"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  TD_VE_HALT)
    guest_test_prepare tdx_test_module.sh
    guest_test_source_code tdx_halt_test_module halt_test || \
    die "Failed to prepare guest test kernel module for $TESTCASE"
    guest_test_entry tdx_test_module.sh "halt_test" || \
    die "Failed on $TESTCASE tdx_test_module.sh halt_test"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  TD_GUEST_CPUINFO)
    guest_test_prepare tdx_guest_bat_test.sh
    guest_test_entry tdx_guest_bat_test.sh "-t GUEST_CPUINFO" || \
    die "Failed on $TESTCASE tdx_guest_bat_test.sh -t GUEST_CPUINFO"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  TD_GUEST_KCONFIG)
    guest_test_prepare tdx_guest_bat_test.sh
    guest_test_entry tdx_guest_bat_test.sh "-t GUEST_KCONFIG" || \
    die "Failed on $TESTCASE tdx_guest_bat_test.sh -t GUEST_KCONFIG"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  TD_GUEST_DRIVER_KCONFIG)
    guest_test_prepare tdx_guest_bat_test.sh
    guest_test_entry tdx_guest_bat_test.sh "-t GUEST_DRIVER_KCONFIG" || \
    die "Failed on $TESTCASE tdx_guest_bat_test.sh -t GUEST_DRIVER_KCONFIG"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  TD_GUEST_LAZY_ACCEPT_KCONFIG)
    guest_test_prepare tdx_guest_bat_test.sh
    guest_test_entry tdx_guest_bat_test.sh "-t GUEST_LAZY_ACCEPT_KCONFIG" || \
    die "Failed on $TESTCASE tdx_guest_bat_test.sh -t GUEST_LAZY_ACCEPT_KCONFIG"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  TD_GUEST_TSM_REPORTS_KCONFIG)
    guest_test_prepare tdx_guest_bat_test.sh
    guest_test_entry tdx_guest_bat_test.sh "-t GUEST_TSM_REPORTS_KCONFIG" || \
    die "Failed on $TESTCASE tdx_guest_bat_test.sh -t GUEST_TSM_REPORTS_KCONFIG"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  TD_GUEST_ATTEST_DEV)
    guest_test_prepare tdx_guest_bat_test.sh
    guest_test_entry tdx_guest_bat_test.sh "-t GUEST_ATTEST_DEV" || \
    die "Failed on $TESTCASE tdx_guest_bat_test.sh -t GUEST_ATTEST_DEV"
    [[ "$GCOV" == "off" ]] && guest_test_close
    ;;
  :)
    test_print_err "Must specify the test scenario option by [-t]"
    usage && exit 1
    ;;
  \?)
    test_print_err "Input test case option $TESTCASE is not supported"
    usage && exit 1
    ;;
esac