#include "device.h"
#include "kheap.h"
#include "framebuffer.h"

device_t* m_devices = 0;
uint32_t m_devices_number; 

void devices_init() {
   m_devices = malloc(sizeof(device_t) * 64);
   m_devices_number = 0;
}

uint32_t devices_add(device_t dev) {
   m_devices[m_devices_number] = dev;
   term_printf("[DEVICES] Added new device %s, on index %x\n", dev.name, m_devices_number);
   return m_devices_number++;
}

device_t* devices_at(uint32_t index) {
   return &m_devices[index];
}

uint32_t devices_number() {
   return m_devices_number;
}

