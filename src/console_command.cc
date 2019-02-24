#include "liumos.h"

namespace ConsoleCommand {

static const GUID kByteAdressablePersistentMemory = {
    0x66F0D379,
    0xB4F3,
    0x4074,
    {0xAC, 0x43, 0x0D, 0x33, 0x18, 0xB7, 0x8C, 0xDB}};

static const GUID kNVDIMMControlRegion = {
    0x92F701F6,
    0x13B4,
    0x405D,
    {0x91, 0x0B, 0x29, 0x93, 0x67, 0xE8, 0x23, 0x4C}};

static void ShowNFIT_PrintMemoryMappingAttr(uint64_t attr) {
  PutString("  attr: ");
  PutHex64(attr);
  PutString(" =");
  if (attr & 0x10000)
    PutString(" MORE_RELIABLE");
  if (attr & 0x08000)
    PutString(" NV");
  if (attr & 0x04000)
    PutString(" XP");
  if (attr & 0x02000)
    PutString(" RP");
  if (attr & 0x01000)
    PutString(" WP");
  if (attr & 0x00010)
    PutString(" UCE");
  if (attr & 0x00008)
    PutString(" WB");
  if (attr & 0x00004)
    PutString(" WT");
  if (attr & 0x00002)
    PutString(" WC");
  if (attr & 0x00001)
    PutString(" UC");
  PutChar('\n');
}

static void ShowNFIT_PrintMemoryTypeGUID(ACPI::NFIT_SPARange* spa) {
  GUID* type_guid = reinterpret_cast<GUID*>(&spa->address_range_type_guid);
  PutString("  type:");
  if (IsEqualGUID(type_guid, &kByteAdressablePersistentMemory))
    PutString(" ByteAddressablePersistentMemory");
  else if (IsEqualGUID(type_guid, &kNVDIMMControlRegion))
    PutString(" NVDIMMControlRegion");
  else
    PutGUID(type_guid);
  PutChar('\n');
}

void ShowXSDT() {
  using namespace ACPI;
  XSDT* xsdt = rsdt->xsdt;
  const int num_of_xsdt_entries = (xsdt->length - kDescriptionHeaderSize) >> 3;

  PutString("Found 0x");
  PutHex64(num_of_xsdt_entries);
  PutString(" XSDT entries found:");
  for (int i = 0; i < num_of_xsdt_entries; i++) {
    const char* signature = static_cast<const char*>(xsdt->entry[i]);
    PutChar(' ');
    PutChar(signature[0]);
    PutChar(signature[1]);
    PutChar(signature[2]);
    PutChar(signature[3]);
  }
  PutString("\n");
}

void ShowNFIT() {
  using namespace ACPI;
  if (!nfit) {
    PutString("NFIT not found\n");
    return;
  }
  PutString("NFIT found\n");
  PutStringAndHex("NFIT Size in bytes", nfit->length);
  for (int i = 0; i < (int)(nfit->length - offsetof(NFIT, entry)) / 2;
       i += nfit->entry[i + 1] / 2) {
    NFITStructureType type = static_cast<NFITStructureType>(nfit->entry[i]);
    if (type == NFITStructureType::kSystemPhysicalAddressRangeStructure) {
      NFIT_SPARange* spa_range =
          reinterpret_cast<NFIT_SPARange*>(&nfit->entry[i]);
      PutStringAndHex("SPARange #", spa_range->spa_range_structure_index);
      PutStringAndHex("  Base", spa_range->system_physical_address_range_base);
      PutStringAndHex("  Length",
                      spa_range->system_physical_address_range_length);
      ShowNFIT_PrintMemoryMappingAttr(
          spa_range->address_range_memory_mapping_attribute);
      ShowNFIT_PrintMemoryTypeGUID(spa_range);
    } else if (type == NFITStructureType::kNVDIMMRegionMappingStructure) {
      NFIT_RegionMapping* rmap =
          reinterpret_cast<NFIT_RegionMapping*>(&nfit->entry[i]);
      PutString("Region Mapping\n");
      PutStringAndHex("  NFIT Device Handle #", rmap->nfit_device_handle);
      PutStringAndHex("  NVDIMM phys ID", rmap->nvdimm_physical_id);
      PutStringAndHex("  NVDIMM region ID", rmap->nvdimm_region_id);
      PutStringAndHex("  SPARange struct index",
                      rmap->spa_range_structure_index);
      PutStringAndHex("  ControlRegion struct index",
                      rmap->nvdimm_control_region_struct_index);
      PutStringAndHex("  region size", rmap->nvdimm_region_size);
      PutStringAndHex("  region offset", rmap->region_offset);
      PutStringAndHex("  NVDIMM phys addr region base",
                      rmap->nvdimm_physical_address_region_base);
      PutStringAndHex("  NVDIMM interleave_structure_index",
                      rmap->interleave_structure_index);
      PutStringAndHex("  NVDIMM interleave ways", rmap->interleave_ways);
      PutStringAndHex("  NVDIMM state flags", rmap->nvdimm_state_flags);
    } else if (type == NFITStructureType::kNVDIMMControlRegionStructure) {
      NFIT_ControlRegionStruct* ctrl_region =
          reinterpret_cast<NFIT_ControlRegionStruct*>(&nfit->entry[i]);
      PutStringAndHex("Control Region Struct #",
                      ctrl_region->nvdimm_control_region_struct_index);
    } else if (type == NFITStructureType::kInterleaveStructure) {
      NFIT_InterleaveStructure* interleave =
          reinterpret_cast<NFIT_InterleaveStructure*>(&nfit->entry[i]);
      PutStringAndHex("Interleave Struct #",
                      interleave->interleave_struct_index);
      PutStringAndHex("  Line size (in bytes)", interleave->line_size);
      PutString("  Lines = ");
      PutHex64(interleave->num_of_lines_described);
      PutString(" :");
      for (uint32_t line_index = 0;
           line_index < interleave->num_of_lines_described; line_index++) {
        PutString(" +");
        PutHex64(interleave->line_offsets[line_index]);
      }
      PutChar('\n');
    } else if (type == NFITStructureType::kFlushHintAddressStructure) {
      NFIT_FlushHintAddressStructure* flush_hint =
          reinterpret_cast<NFIT_FlushHintAddressStructure*>(&nfit->entry[i]);
      PutStringAndHex("Flush Hint Addresses for NFIT Device handle",
                      flush_hint->nfit_device_handle);
      PutString("  Addrs = ");
      PutHex64(flush_hint->num_of_flush_hint_addresses);
      PutString(" :");
      for (uint32_t index = 0; index < flush_hint->num_of_flush_hint_addresses;
           index++) {
        PutString(" @");
        PutHex64(flush_hint->flush_hint_addresses[index]);
      }
      PutChar('\n');
    } else if (type == NFITStructureType::kPlatformCapabilitiesStructure) {
      NFIT_PlatformCapabilities* caps =
          reinterpret_cast<NFIT_PlatformCapabilities*>(&nfit->entry[i]);
      const int cap_shift = 31 - caps->highest_valid_cap_bit;
      const uint32_t cap_bits =
          ((caps->capabilities << cap_shift) & 0xFFFF'FFFFUL) >> cap_shift;
      PutString("Platform Capabilities\n");
      PutStringAndBool("  Flush CPU Cache when power loss", cap_bits & 0b001);
      PutStringAndBool("  Flush Memory Controller when power loss",
                       cap_bits & 0b010);
      PutStringAndBool("  Hardware Mirroring Support", cap_bits & 0b100);
    } else {
      PutStringAndHex("Unknown entry. type", static_cast<int>(type));
    }
  }
  PutString("NFIT end\n");
}

void PutMPSINTIFlags(uint8_t flags) {
  PutString(" polarity=");
  uint8_t polarity = flags & 3;
  if (polarity == 0b00)
    PutString("same_as_bus_spec");
  if (polarity == 0b01)
    PutString("active-high");
  if (polarity == 0b10)
    PutString("reserved");
  if (polarity == 0b11)
    PutString("active-low");
  PutString(" trigger=");
  uint8_t trigger = (flags >> 2) & 3;
  if (trigger == 0b00)
    PutString("same_as_bus_spec");
  if (trigger == 0b01)
    PutString("edge");
  if (trigger == 0b10)
    PutString("reserved");
  if (trigger == 0b11)
    PutString("level");
}

void ShowMADT() {
  using namespace ACPI;
  for (int i = 0; i < (int)(madt->length - offsetof(MADT, entries));
       i += madt->entries[i + 1]) {
    uint8_t type = madt->entries[i];
    PutString("MADT(type=0x");
    PutHex64(type);
    PutString(") ");

    if (type == kProcessorLocalAPICInfo) {
      PutString("Processor 0x");
      PutHex64(madt->entries[i + 2]);
      PutString(" local_apic_id = 0x");
      PutHex64(madt->entries[i + 3]);
      PutString((madt->entries[i + 4] & 1) ? " Enabled" : "Disabled");
    } else if (type == kIOAPICInfo) {
      PutString("IOAPIC id=0x");
      PutHex64(madt->entries[i + 2]);
      PutString(" base=0x");
      PutHex64(*(uint32_t*)&madt->entries[i + 4]);
      PutString(" gsi_base=0x");
      PutHex64(*(uint32_t*)&madt->entries[i + 8]);
    } else if (type == kInterruptSourceOverrideInfo) {
      PutString("IRQ override: 0x");
      PutHex64(madt->entries[i + 3]);
      PutString(" -> 0x");
      PutHex64(*(uint32_t*)&madt->entries[i + 4]);
      PutMPSINTIFlags(madt->entries[i + 8]);
    } else if (type == kLocalAPICNMIStruct) {
      PutString("LAPIC NMI uid=0x");
      PutHex64(madt->entries[i + 2]);
      PutMPSINTIFlags(madt->entries[i + 3]);
      PutString(" LINT#=0x");
      PutHex64(madt->entries[i + 5]);
    } else if (type == kProcessorLocalx2APICStruct) {
      PutString("x2APIC id=0x");
      PutHex64(*(uint32_t*)&madt->entries[i + 4]);
      PutString((madt->entries[i + 8] & 1) ? " Enabled" : " Disabled");
      PutString(" acpi_processor_uid=0x");
      PutHex64(*(uint32_t*)&madt->entries[i + 12]);
    } else if (type == kLocalx2APICNMIStruct) {
      PutString("x2APIC NMI");
      PutMPSINTIFlags(madt->entries[i + 2]);
      PutString(" acpi_processor_uid=0x");
      PutHex64(*(uint32_t*)&madt->entries[i + 4]);
      PutString(" LINT#=0x");
      PutHex64(madt->entries[i + 8]);
    }
    PutString("\n");
  }
}

void ShowSRAT() {
  using namespace ACPI;
  if (!srat) {
    PutString("SRAT not found.\n");
    return;
  }
  PutString("SRAT found.\n");
  for (auto& it : *srat) {
    PutString("SRAT(type=0x");
    PutHex64(it.type);
    PutString(") ");

    if (it.type == SRAT::Entry::kTypeLAPICAffinity) {
      SRAT::LAPICAffinity* e = reinterpret_cast<SRAT::LAPICAffinity*>(&it);
      PutString("LAPIC Affinity id=0x");
      PutHex64(e->apic_id);
      const uint32_t proximity_domain = e->proximity_domain_low |
                                        (e->proximity_domain_high[0] << 8) |
                                        (e->proximity_domain_high[1] << 16) |
                                        (e->proximity_domain_high[2] << 24);
      PutString(" proximity_domain=0x");
      PutHex64(proximity_domain);
    } else if (it.type == SRAT::Entry::kTypeMemoryAffinity) {
      SRAT::MemoryAffinity* e = reinterpret_cast<SRAT::MemoryAffinity*>(&it);
      PutString("Memory Affinity [");
      PutHex64ZeroFilled(e->base_address);
      PutString(", ");
      PutHex64ZeroFilled(e->base_address + e->size);
      PutString(")\n  proximity_domain=0x");
      PutHex64(e->proximity_domain);
      if (e->flags & 1)
        PutString(" Enabled");
      if (e->flags & 2)
        PutString(" Hot-pluggable");
      if (e->flags & 4)
        PutString(" Non-volatile");
    } else if (it.type == SRAT::Entry::kTypeLx2APICAffinity) {
      SRAT::Lx2APICAffinity* e = reinterpret_cast<SRAT::Lx2APICAffinity*>(&it);
      PutString("Lx2APIC Affinity id=0x");
      PutHex64(e->x2apic_id);
      PutString(" proximity_domain=0x");
      PutHex64(e->proximity_domain);
    }
    PutString("\n");
  }
}

void ShowSLIT() {
  using namespace ACPI;
  if (!slit) {
    PutString("SLIT not found.\n");
    return;
  }
  PutString("SLIT found.\n");
  PutStringAndHex("num_of_system_localities", slit->num_of_system_localities);
  for (uint64_t y = 0; y < slit->num_of_system_localities; y++) {
    for (uint64_t x = 0; x < slit->num_of_system_localities; x++) {
      if (x)
        PutString(", ");
      PutHex64(slit->entry[y * slit->num_of_system_localities + x]);
    }
    PutString("\n");
  }
}

void ShowEFIMemoryMap() {
  PutStringAndHex("Map entries", efi_memory_map.GetNumberOfEntries());
  for (int i = 0; i < efi_memory_map.GetNumberOfEntries(); i++) {
    const EFI::MemoryDescriptor* desc = efi_memory_map.GetDescriptor(i);
    desc->Print();
    PutString("\n");
  }
}

void Free() {
  page_allocator->Print();
}

bool IsEqualString(const char* a, const char* b) {
  while (*a == *b) {
    if (*a == 0)
      return true;
    a++;
    b++;
  }
  return false;
}

void label(uint64_t i) {
  PutString("0x");
  PutHex64(i);
  PutString(",");
}

uint64_t get_seconds() {
  return hpet.ReadMainCounterValue();
}

void TestMem() {
  constexpr uint64_t kRangeMin = 1ULL << 10;
  constexpr uint64_t kRangeMax = 1ULL << 24;
  uint64_t nextstep, i, index;
  uint64_t csize, stride;
  uint64_t steps, tsteps;
  uint64_t sec0, sec1, tick_sum_with_mem_read, tick_sum_without_mem_read;
  uint64_t kDurationTick = (uint64_t)0.1 * 1e15 / hpet.GetFemtosecndPerCount();
  int* array = nullptr;
  array = page_allocator->AllocPages<int*>(
      (sizeof(int) * kRangeMax + kPageSize - 1) >> kPageSizeExponent);

  PutString(" ,");
  for (stride = 1; stride <= kRangeMax / 2; stride = stride * 2)
    label(stride * sizeof(int));
  PutString("\n");

  for (csize = kRangeMin; csize <= kRangeMax; csize = csize * 2) {
    label(csize * sizeof(int));
    for (stride = 1; stride <= csize / 2; stride = stride * 2) {
      for (index = 0; index < csize; index = index + stride) {
        array[index] = (int)(index + stride);
      }
      array[index - stride] = 0;

      // measure time spent on (reading data from memory) + (loop, instrs,
      // etc...)
      steps = 0;
      nextstep = 0;
      sec0 = get_seconds();
      do {
        for (i = stride; i != 0; i = i - 1) {
          nextstep = 0;
          do
            nextstep = array[nextstep];
          while (nextstep != 0);
        }
        steps = steps + 1;
        sec1 = get_seconds();
      } while ((sec1 - sec0) < kDurationTick);  // originary 20.0
      tick_sum_with_mem_read = sec1 - sec0;

      // measure time spent on (loop, instrs, etc...) only
      tsteps = 0;
      sec0 = get_seconds();
      do {
        for (i = stride; i != 0; i = i - 1) {
          index = 0;
          do
            index = index + stride;
          while (index < csize);
        }
        tsteps = tsteps + 1;
        sec1 = get_seconds();
      } while (tsteps < steps);
      tick_sum_without_mem_read = sec1 - sec0;

      // avoid negative value
      if (tick_sum_without_mem_read >= tick_sum_with_mem_read) {
        tick_sum_without_mem_read = tick_sum_with_mem_read;
      }

      const uint64_t tick_sum_of_mem_read =
          tick_sum_with_mem_read - tick_sum_without_mem_read;
      const uint64_t pico_second_per_mem_read = tick_sum_of_mem_read *
                                                hpet.GetFemtosecndPerCount() /
                                                (steps * csize) / 1000;
      PutString("0x");
      PutHex64(pico_second_per_mem_read > 0 ? pico_second_per_mem_read : 1);
      PutString(", ");
    };
    PutString("\n");
  };
}

void Time() {
  PutStringAndHex("HPET main counter", hpet.ReadMainCounterValue());
}

void Process(TextBox& tbox) {
  const char* line = tbox.GetRecordedString();
  if (IsEqualString(line, "hello")) {
    PutString("Hello, world!\n");
  } else if (IsEqualString(line, "show xsdt")) {
    ShowXSDT();
  } else if (IsEqualString(line, "show nfit")) {
    ShowNFIT();
  } else if (IsEqualString(line, "show madt")) {
    ShowMADT();
  } else if (IsEqualString(line, "show srat")) {
    ShowSRAT();
  } else if (IsEqualString(line, "show slit")) {
    ShowSLIT();
  } else if (IsEqualString(line, "show mmap")) {
    ShowEFIMemoryMap();
  } else if (IsEqualString(line, "show hpet")) {
    hpet.Print();
  } else if (IsEqualString(line, "test mem")) {
    TestMem();
  } else if (IsEqualString(line, "free")) {
    Free();
  } else if (IsEqualString(line, "time")) {
    Time();
  } else if (IsEqualString(line, "hello.bin")) {
    ParseELFFile(hello_bin_file);
  } else if (IsEqualString(line, "liumos.elf")) {
    ParseELFFile(liumos_elf_file);
  } else if (IsEqualString(line, "help")) {
    PutString("hello: Nothing to say.\n");
    PutString("show xsdt: Print XSDT Entries\n");
    PutString("show nfit: Print NFIT Entries\n");
    PutString("show madt: Print MADT Entries\n");
    PutString("show srat: Print SRAT Entries\n");
    PutString("show slit: Print SLIT Entries\n");
    PutString("show mmap: Print UEFI MemoryMap\n");
    PutString("test mem: Test memory access \n");
    PutString("free: show memory free entries\n");
    PutString("time: show HPET main counter value\n");
  } else {
    PutString("Command not found: ");
    PutString(tbox.GetRecordedString());
    tbox.putc('\n');
  }
}

}  // namespace ConsoleCommand
