#include <kernel/printk.h>
#include <pic/apic/apic.h>
#include <stdint.h>
#include <uacpi/acpi.h>
#include <uacpi/tables.h>

void parse_lapic(struct acpi_madt_lapic *lapic) {
  if (!lapic)
    return;

  if (lapic->hdr.length < sizeof(struct acpi_madt_lapic)) {
    printk("LAPIC: invalid length %u\n", lapic->hdr.length);
    return;
  }

  uint8_t cpu = lapic->uid;
  uint8_t apic = lapic->id;
  uint32_t flags = lapic->flags;

  if (!(flags & 1)) {
    printk(LOG_INFO "LAPIC: CPU %u (APIC %u) disabled\n", cpu, apic);
    return;
  }

  printk(LOG_INFO "LAPIC: CPU %u -> APIC ID %u (enabled)\n", cpu, apic);
}

void parse_ioapic(struct acpi_madt_ioapic *ioapic) {
  if (!ioapic)
    return;

  if (ioapic->hdr.length < sizeof(struct acpi_madt_ioapic)) {
    printk(LOG_INFO "IOAPIC: invalid length %u\n", ioapic->hdr.length);
    return;
  }

  uint8_t id = ioapic->id;
  uint32_t addr = ioapic->address;
  uint32_t gsi_base = ioapic->gsi_base;

  printk(LOG_INFO "IOAPIC: id=%u addr=0x%x GSI base=%u\n", id, addr, gsi_base);
}

void parse_iso(struct acpi_madt_interrupt_source_override *iso) {
  if (!iso)
    return;

  if (iso->hdr.length < sizeof(struct acpi_madt_interrupt_source_override)) {
    printk(LOG_WARN "ISO: invalid length %u\n", iso->hdr.length);
    return;
  }

  uint8_t bus = iso->bus;
  uint8_t source = iso->source;
  uint32_t gsi = iso->gsi;
  uint16_t flags = iso->flags;

  uint8_t polarity = flags & 0x3;
  uint8_t trigger = (flags >> 2) & 0x3;

  printk(LOG_INFO "ISO: bus=%u IRQ=%u -> GSI=%u (pol=%u trig=%u)\n", bus,
         source, gsi, polarity, trigger);
}

void init_apic(void) {
  uacpi_table tbl;

  if (uacpi_table_find_by_signature(ACPI_MADT_SIGNATURE, &tbl) !=
      UACPI_STATUS_OK) {
    return;
  }

  struct acpi_madt *madt = (struct acpi_madt *)tbl.ptr;

  uint8_t *ptr = (uint8_t *)madt->entries;
  uint8_t *end = (uint8_t *)madt + madt->hdr.length;

  while (ptr + sizeof(struct acpi_entry_hdr) <= end) {
    struct acpi_entry_hdr *h = (struct acpi_entry_hdr *)ptr;

    if (h->length < sizeof(struct acpi_entry_hdr)) {
      break;
    }

    /* Prevent overrun */
    if (ptr + h->length > end) {
      break;
    }

    switch (h->type) {
    case ACPI_MADT_ENTRY_TYPE_LAPIC:
      parse_lapic((struct acpi_madt_lapic *)ptr);
      break;

    case ACPI_MADT_ENTRY_TYPE_IOAPIC:
      parse_ioapic((struct acpi_madt_ioapic *)ptr);
      break;

    case ACPI_MADT_ENTRY_TYPE_INTERRUPT_SOURCE_OVERRIDE:
      parse_iso((struct acpi_madt_interrupt_source_override *)ptr);
      break;

    default:
      break;
    }

    ptr += h->length;
  }

  printk(LOG_INFO "APIC initialized\n");
}
