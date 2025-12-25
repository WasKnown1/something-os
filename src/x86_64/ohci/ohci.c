#include <ohci.h>
#include <pci.h>
#include <log.h>
#include <alloc.h>
#include <paging.h>
#include <string.h>

OHCIControllerArray ohci_controllers = DYNAMIC_ARRAY_INIT;

void print_ohci_mmio_base_function(OHCIController* controller) {
    debug_printf("OHCI MMIO Base that was found: %p\n", controller->mmio_base);
}

OHCIControllerArray pci_scan_for_ohci(void) {
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {

            uint16_t vendor = pci_config_read_dword(bus, slot, 0, 0);

            if (vendor == 0xffff)
                continue; // no device

            uint32_t class_reg = pci_config_read_dword(bus, slot, 0, 0x08);

            uint8_t class    = (class_reg >> 24) & 0xFF;
            uint8_t subclass = (class_reg >> 16) & 0xFF;
            uint8_t progif   = (class_reg >> 8)  & 0xFF;

            if (class == 0x0c && subclass == 0x03 && progif == 0x10) {
                debug_printf("OHCI controller found at %x:%x:%x\n", bus, slot, 0);

                uint32_t bar0 = pci_config_read_dword(bus, slot, 0, 0x10);

                uint32_t mmio_base = bar0 & ~0xF;
                OHCIController controller = {
                    .mmio_base = mmio_base,
                    .pci_bus = bus,
                    .pci_slot = slot,
                    .pci_function = 0
                };
                OHCIController* allocated_controller = malloc(sizeof(OHCIController));
                memcpy(allocated_controller, &controller, sizeof(OHCIController));
                dynamic_array_push(OHCIController *, ohci_controllers, allocated_controller);
                debug_printf("OHCI MMIO base: %p\n", mmio_base);

            }
        }
    }

    if (dynamic_array_size(ohci_controllers) == 0)
        debug_printf("no OHCI controller found\n");
    else
        dynamic_array_print(ohci_controllers, print_ohci_mmio_base_function);

    return ohci_controllers;
}

OHCIRegisters *init_ohci_controller(OHCIController* controller) {
    map_identity_4mb(controller->mmio_base & ~0x3fffff, 0x400000);

    uint16_t cmd = pci_config_read_dword(controller->pci_bus, controller->pci_slot, controller->pci_function, 0x04);
    cmd |= (1 << 2); // enable bus mastering
    cmd |= (1 << 1); // enable memory space
    pci_config_write_dword(controller->pci_bus, controller->pci_slot, controller->pci_function, 0x04, cmd);

    OHCIRegisters* ohci = (OHCIRegisters*)controller->mmio_base;

    ohci->cmd_status |= (1 << 0);        // reset controller
    while (ohci->cmd_status & (1 << 0)); // wait for reset to complete

    uint8_t* hcca = aligned_alloc(256, 256);
    ohci->hcca = (uint32_t)hcca;

    ohci->control =
        OHCI_CTRL_CBSR_1_1 |
        OHCI_CTRL_PLE |
        OHCI_CTRL_IE |
        OHCI_CTRL_CLE |
        OHCI_CTRL_BLE |
        OHCI_CTRL_HCFS_OP;

    ohci->intr_disable = 0xffffffff;
    ohci->intr_enable  = (1 << 2); // enable some interrupts

    return ohci;
}

void detect_ohci_ports(OHCIRegisters* ohci) {
    uint32_t rhda = ohci->rh_desc_a;
    uint8_t port_count = rhda & 0xff;

    for (uint8_t port = 0; port < port_count; port++) {
        uint32_t port_status = ohci->rh_port_status[port];
        if (port_status & (1 << 0)) {
            debug_printf("OHCI port %d: device connected\n", port + 1);
        } else {
            debug_printf("OHCI port %d: no device connected\n", port + 1);
        }
    }
}