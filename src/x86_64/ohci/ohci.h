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

typedef DynamicArray(OHCIController *) OHCIControllerArray;

void print_ohci_mmio_base_function(OHCIController* controller);
OHCIControllerArray pci_scan_for_ohci(void);
OHCIRegisters *init_ohci_controller(OHCIController* controller);
void detect_ohci_ports(OHCIRegisters* ohci);

#endif // OHCI_H