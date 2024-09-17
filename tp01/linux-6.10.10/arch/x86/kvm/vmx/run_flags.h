/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __KVM_X86_VMX_RUN_FLAGS_H
#define __KVM_X86_VMX_RUN_FLAGS_H

#define VMX_RUN_VMRESUME_SHIFT		0
#define VMX_RUN_SAVE_SPEC_CTRL_SHIFT	1

#define VMX_RUN_VMRESUME		BIT(VMX_RUN_VMRESUME_SHIFT)
#define VMX_RUN_SAVE_SPEC_CTRL		BIT(VMX_RUN_SAVE_SPEC_CTRL_SHIFT)

#endif /* __KVM_X86_VMX_RUN_FLAGS_H */
