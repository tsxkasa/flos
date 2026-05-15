#include <kernel/printk.h>
#include <pic/apic/apic.h>
#include <pic/apic/lapic.h>
#include <pic/pic.h>
#include <stdint.h>
#include <uacpi/acpi.h>
#include <uacpi/tables.h>

static struct apic_info apic;

static void parse_lapic(struct acpi_madt_lapic *lapic) {
  if (!lapic)
    return;

  if (lapic->hdr.length < sizeof(struct acpi_madt_lapic)) {
    printk(LOG_WARN "LAPIC: invalid length %u\n", lapic->hdr.length);
    return;
  }

  if (apic.cpu_count >= MAX_CPU) {
    printk(LOG_WARN "LAPIC: too many CPUs\n");
    return;
  }

  struct apic_cpu *cpu = &apic.cpus[apic.cpu_count++];

  cpu->apic_uid = lapic->uid;
  cpu->apic_id = lapic->id;
  cpu->enabled = (lapic->flags & 1) != 0;

  if (!cpu->enabled) {
    printk(LOG_INFO "LAPIC: CPU %u (APIC %u) disabled\n", cpu->apic_uid,
           cpu->apic_id);
    return;
  }

  printk(LOG_INFO "LAPIC: CPU %u -> APIC ID %u (enabled)\n", cpu->apic_uid,
         cpu->apic_id);
}

static void parse_ioapic(struct acpi_madt_ioapic *ioapic) {
  if (!ioapic)
    return;

  if (ioapic->hdr.length < sizeof(struct acpi_madt_ioapic)) {
    printk(LOG_INFO "IOAPIC: invalid length %u\n", ioapic->hdr.length);
    return;
  }

  if (apic.ioapic_count >= MAX_IOAPIC) {
    printk(LOG_WARN "IOAPIC: too many IOAPIC\n");
    return;
  }

  struct ioapic_info *ioa = &apic.ioapic[apic.ioapic_count++];

  ioa->id = ioapic->id;
  ioa->addr = ioapic->address;
  ioa->gsi_base = ioapic->gsi_base;

  printk(LOG_INFO "IOAPIC: id=%u addr=0x%x GSI base=%u\n", ioa->id, ioa->addr,
         ioa->gsi_base);
}

static void parse_iso(struct acpi_madt_interrupt_source_override *iso) {
  if (!iso)
    return;

  if (iso->hdr.length < sizeof(struct acpi_madt_interrupt_source_override)) {
    printk(LOG_WARN "ISO: invalid length %u\n", iso->hdr.length);
    return;
  }

  if (iso->bus != 0) {
    printk(LOG_WARN "ISO: unsupported bus\n");
    return;
  }

  if (apic.iso_count > MAX_ISO) {
    printk(LOG_WARN "ISO: too many ISOs\n");
    return;
  }

  struct apic_iso *aiso = &apic.iso[apic.iso_count++];

  aiso->source_irq = iso->source;
  aiso->gsi = iso->gsi;
  aiso->flags = iso->flags;

  uint8_t polarity = aiso->flags & 0x3;
  uint8_t trigger = (aiso->flags >> 2) & 0x3;

  printk(LOG_INFO "ISO: bus=%u IRQ=%u -> GSI=%u (pol=%u trig=%u)\n", 0,
         aiso->source_irq, aiso->gsi, polarity, trigger);
}

static void parse_madt(void) {
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
}

void init_apic(void) {
  parse_madt();
  pic_mask_all();

  init_lapic();

  printk(LOG_INFO "APIC initialized\n");
}
