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
    map_identity_4mb(0xf000ff5a, 0x400000);
    map_identity_4mb((uint32_t)controller->mmio_base, 0x400000);

    uint16_t cmd = pci_config_read_dword(controller->pci_bus, controller->pci_slot, controller->pci_function, 0x04);
    cmd |= (1 << 2); // enable bus mastering
    cmd |= (1 << 1); // enable memory space
    pci_config_write_dword(controller->pci_bus, controller->pci_slot, controller->pci_function, 0x04, cmd);

    OHCIRegisters* ohci = (OHCIRegisters*)controller->mmio_base;

    ohci->cmd_status |= (1 << 0);        // reset controller
    while (ohci->cmd_status & (1 << 0)); // wait for reset to complete

    // uint8_t* hcca = aligned_alloc(256, 256);
    // ohci->hcca = (uint32_t)hcca;

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

static uint32_t read_port_status(OHCIRegisters* ohci, uint8_t port) {
    if (port >= 15) {
        debug_printf("Invalid port number: %d\n", port);
        return 0;
    }
    return ohci->rh_port_status[port];
}

static void write_port_status(OHCIRegisters* ohci, uint8_t port, uint32_t value) {
    if (port >= 15) {
        debug_printf("Invalid port number: %d\n", port);
        return;
    }
    ohci->rh_port_status[port] = value;
}

void ohci_port_reset(OHCIRegisters* ohci, uint8_t port) {
    if (port >= 15) {
        debug_printf("Invalid port number: %d\n", port);
        return;
    }

    uint32_t port_status = read_port_status(ohci, port);
    port_status |= OHCI_PORT_RESET; // set the reset bit
    write_port_status(ohci, port, port_status);

    // wait for at least 10ms (improved delay)
    volatile uint32_t delay = 0;
    for (delay = 0; delay < 1000000; delay++);

    port_status = read_port_status(ohci, port);
    port_status &= ~OHCI_PORT_RESET; // clear the reset bit
    write_port_status(ohci, port, port_status);

    // wait until the port is enabled with timeout
    volatile uint32_t timeout = 0;
    while (!(read_port_status(ohci, port) & OHCI_PORT_ENABLE) && timeout < 100000) {
        timeout++;
    }

    debug_printf("OHCI port %d has been reset\n", port + 1);
}

// static OHCIHCCA hcca_storage __attribute__((aligned(256)));

OHCIHCCA* init_ohci_scheduling(OHCIController* ohci_controller) {
    map_identity_4mb(ohci_controller->mmio_base & ~0xfff, 0x1000);
    OHCIRegisters* regs = (volatile OHCIRegisters*)ohci_controller->mmio_base;
    
    if (is_pointing_to_allocated_memory((void *)regs->hcca)) // free previous HCCA if exists
        free((void *)regs->hcca);

    // OHCIHCCA* hcca = &hcca_storage;
    OHCIHCCA* hcca = aligned_alloc(256, sizeof(OHCIHCCA));
    memset(hcca, 0, sizeof(OHCIHCCA));
    regs->hcca = (uint32_t)hcca;
    OHCIEndpointDescriptor* control_ed = aligned_alloc(16, sizeof(OHCIEndpointDescriptor));
    control_ed->flags =
        ED_FA(0) |
        ED_EN(0) |
        ED_DIR_FROM_TD |
        ED_SPEED_FULL |
        ED_MPS(8);

    control_ed->tail_pointer = 0;
    control_ed->head_pointer = 0;
    control_ed->next_ed = 0;

    regs->control_head_ed = (uint32_t)control_ed;

    uint8_t* setup_packet_buffer = aligned_alloc(16, 8);
    OHCITransferDescriptor* td = aligned_alloc(16, sizeof(OHCITransferDescriptor));
    td->flags = TD_DP_SETUP | TD_TOGGLE_0;
    td->cbp = (uint32_t)setup_packet_buffer;
    td->be = (uint32_t)(setup_packet_buffer + 7);

    td->next_td = 0;
    control_ed->tail_pointer = (uint32_t)td;
    control_ed->head_pointer = (uint32_t)td; // set the halt bit
    regs->control_head_ed = (uint32_t)control_ed;

    hcca->interrupt_table[0] = (uint32_t)control_ed; // this is for keyboard/mouse interrupts
    regs->cmd_status = (1 << 0);         // HCR
    while (regs->cmd_status & (1 << 0)); // wait for halt to complete

    regs->fm_interval = (11999 & 0x3FFF) | (0x2EDF << 16); // frame interval
    regs->control |= (2 << 6);                             // usb operational

    return hcca;
}

