#ifndef PIO_H
#define PIO_H

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ ("inb %w1, %b0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ ("outb %b0, %w1" : : "a"(value), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t value;
    __asm__ ("inw %w1, %w0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outw(uint16_t port, uint16_t value) {
    __asm__ ("outw %w0, %w1" : : "a"(value), "Nd"(port));
}

#endif // PIO_H