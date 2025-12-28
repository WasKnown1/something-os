#ifndef OHCI_H
#define OHCI_H

#include <stdint.h>
#include <alloc.h>

#define OHCI_CTRL_CBSR_1_1 (0 << 0)
#define OHCI_CTRL_PLE      (1 << 2)
#define OHCI_CTRL_IE       (1 << 3)
#define OHCI_CTRL_CLE      (1 << 4)
#define OHCI_CTRL_BLE      (1 << 5)
#define OHCI_CTRL_HCFS_OP  (2 << 6)
#define OHCI_PORT_RESET    (1 << 4)
#define OHCI_PORT_ENABLE   (1 << 2)
#define ED_FA(addr)        ((addr) & 0x7F)
#define ED_EN(endpoint)    (((endpoint) & 0xF) << 7)
#define ED_DIR_FROM_TD     (0 << 11)
#define ED_SPEED_FULL      (0 << 13)
#define ED_SKIP            (1 << 14)
#define ED_MPS(mps)        (((mps) & 0x7FF) << 16)
#define TD_CC_NOERROR      (0 << 28)
#define TD_DP_SETUP        (0 << 19)
#define TD_DP_OUT          (1 << 19)
#define TD_DP_IN           (2 << 19)
#define TD_TOGGLE_0        (0 << 24)
#define TD_TOGGLE_1        (1 << 24)
#define TD_TOGGLE_FROM_ED  (2 << 24)

typedef struct {
    uint32_t mmio_base;
    uint16_t pci_bus;
    uint8_t pci_slot;
    uint8_t pci_function;
} OHCIController;

typedef volatile struct {
    uint32_t revision;
    uint32_t control;
    uint32_t cmd_status;
    uint32_t intr_status;
    uint32_t intr_enable;
    uint32_t intr_disable;
    uint32_t hcca;
    uint32_t period_current_ed;
    uint32_t control_head_ed;
    uint32_t control_current_ed;
    uint32_t bulk_head_ed;
    uint32_t bulk_current_ed;
    uint32_t done_head;
    uint32_t fm_interval;
    uint32_t fm_remaining;
    uint32_t fm_number;
    uint32_t periodic_start;
    uint32_t ls_threshold;
    uint32_t rh_desc_a;
    uint32_t rh_desc_b;
    uint32_t rh_status;
    uint32_t rh_port_status[15];
} OHCIRegisters;

typedef struct OHCIHCCA {
    uint32_t interrupt_table[32];
    uint16_t frame_number;
    uint16_t pad1;
    uint32_t done_head;
    uint8_t reserved[116];
} __attribute__((packed, aligned(256))) OHCIHCCA;

typedef struct {
    uint32_t flags;
    uint32_t tail_pointer;
    uint32_t head_pointer;
    uint32_t next_ed;
} __attribute__((packed)) OHCIEndpointDescriptor;

typedef struct {
    uint32_t flags;
    uint32_t cbp;
    uint32_t next_td;
    uint32_t be;
} __attribute__((packed)) OHCITransferDescriptor;

typedef DynamicArray(OHCIController *) OHCIControllerArray;

void print_ohci_mmio_base_function(OHCIController* controller);
OHCIControllerArray pci_scan_for_ohci(void);
OHCIRegisters *init_ohci_controller(OHCIController* controller);
void detect_ohci_ports(OHCIRegisters* ohci);
void ohci_port_reset(OHCIRegisters* ohci, uint8_t port);
OHCIHCCA* init_ohci_scheduling(OHCIController* ohci_controller);

#endif // OHCI_H