#include "liumos.h"

void InitEFI(EFISystemTable* system_table) {
  _system_table = system_table;
  _system_table->boot_services->SetWatchdogTimer(0, 0, 0, NULL);
  _system_table->boot_services->LocateProtocol(
      &EFI_GraphicsOutputProtocolGUID, NULL,
      (void**)&efi_graphics_output_protocol);
}

void IntHandler03() {
  EFIPutString(L"Int03\r\n");
}

void PrintIDTGateDescriptor(IDTGateDescriptor* desc) {
  EFIPrintStringAndHex(L"desc.ofs", ((uint64_t)desc->offset_high << 32) |
                                        ((uint64_t)desc->offset_mid << 16) |
                                        desc->offset_low);
  EFIPrintStringAndHex(L"desc.segmt", desc->segment_descriptor);
  EFIPrintStringAndHex(L"desc.IST", desc->interrupt_stack_table);
  EFIPrintStringAndHex(L"desc.type", desc->type);
  EFIPrintStringAndHex(L"desc.DPL", desc->descriptor_privilege_level);
  EFIPrintStringAndHex(L"desc.present", desc->present);
}

void efi_main(void* ImageHandle, struct EFI_SYSTEM_TABLE* system_table) {
  InitEFI(system_table);
  EFIClearScreen();
  EFIPutString(L"Hello, liumOS world!...\r\n");
  EFIGetMemoryMap();
  EFIPrintStringAndHex(L"Num of table entries",
                       system_table->number_of_table_entries);
  ACPI_RSDP* rsdp = EFIGetConfigurationTableByUUID(&EFI_ACPITableGUID);
  ACPI_XSDT* xsdt = rsdp->xsdt;
  EFIPrintStringAndHex(L"ACPI Table address", (uint64_t)rsdp);
  EFIPutnCString(rsdp->signature, 8);
  EFIPrintStringAndHex(L"XSDT Table address", (uint64_t)xsdt);
  EFIPutnCString(xsdt->signature, 4);
  int num_of_xsdt_entries = (xsdt->length - ACPI_DESCRIPTION_HEADER_SIZE) >> 3;
  EFIPrintStringAndHex(L"XSDT Table entries", num_of_xsdt_entries);
  EFIPrintStringAndHex(L"XSDT Table entries", num_of_xsdt_entries);

  uint8_t* vram = efi_graphics_output_protocol->mode->frame_buffer_base;
  int xsize = efi_graphics_output_protocol->mode->info->horizontal_resolution;
  int ysize = efi_graphics_output_protocol->mode->info->vertical_resolution;

  for (int y = 0; y < ysize; y++) {
    for (int x = 0; x < xsize; x++) {
      vram[(xsize * y + x) * 4] = x;
      vram[(xsize * y + x) * 4 + 1] = y;
      vram[(xsize * y + x) * 4 + 2] = x + y;
    }
  }

  GDTR gdtr;
  ReadGDTR(&gdtr);
  EFIPrintStringAndHex(L"gdtr.base", gdtr.base);
  EFIPrintStringAndHex(L"gdtr.limit", gdtr.limit);

  IDTR idtr;
  ReadIDTR(&idtr);
  EFIPrintStringAndHex(L"idtr.base", idtr.base);
  EFIPrintStringAndHex(L"idtr.limit", idtr.limit);

  EFIPrintStringAndHex(L"sizeof(IDTGateDescriptor)", sizeof(IDTGateDescriptor));

  IDTGateDescriptor* sample_desc = NULL;
  for (int i = 0; i * sizeof(IDTGateDescriptor) < idtr.limit; i++) {
    if (!idtr.base[i].present)
      continue;
    sample_desc = &idtr.base[i];
    break;
  }

  PrintIDTGateDescriptor(&idtr.base[1]);

  idtr.base[0x03] = *sample_desc;
  IDTGateDescriptor* debug_exception_desc = &idtr.base[0x03];
  IDTGateDescriptor* gp_fault_desc = &idtr.base[13];


  idtr.base[0x03].type = 0xf;
  idtr.base[0x03].offset_low = (uint64_t)AsmIntHandler03 & 0xffff;
  idtr.base[0x03].offset_mid = ((uint64_t)AsmIntHandler03 >> 16) & 0xffff;
  idtr.base[0x03].offset_high = ((uint64_t)AsmIntHandler03 >> 32) & 0xffffffff;

  EFIPrintStringAndHex(L"AsmIntHandler03", AsmIntHandler03);
  PrintIDTGateDescriptor(debug_exception_desc);
  PrintIDTGateDescriptor(gp_fault_desc);

  WriteIDTR(&idtr);
  Int03();

  ACPI_NFIT* nfit = NULL;
  ACPI_HPET* hpet = NULL;

  for (int i = 0; i < num_of_xsdt_entries; i++) {
    if (IsEqualStringWithSize(xsdt->entry[i], "NFIT", 4))
      nfit = xsdt->entry[i];
    if (IsEqualStringWithSize(xsdt->entry[i], "HPET", 4))
      hpet = xsdt->entry[i];
  }

  if (nfit) {
    EFIPutCString("NFIT found.");
    EFIPrintStringAndHex(L"NFIT Size", nfit->length);
    EFIPrintStringAndHex(L"First NFIT Structure Type", nfit->entry[0]);
    EFIPrintStringAndHex(L"First NFIT Structure Size", nfit->entry[1]);
    if (nfit->entry[0] == kSystemPhysicalAddressRangeStructure) {
      ACPI_NFIT_SPARange* spa_range = (ACPI_NFIT_SPARange*)&nfit->entry[0];
      EFIPrintStringAndHex(L"SPARange Base",
                           spa_range->system_physical_address_range_base);
      EFIPrintStringAndHex(L"SPARange Length",
                           spa_range->system_physical_address_range_length);
      EFIPrintStringAndHex(L"SPARange Attribute",
                           spa_range->address_range_memory_mapping_attribute);
      EFIPrintStringAndHex(L"SPARange TypeGUID[0]",
                           spa_range->address_range_type_guid[0]);
      EFIPrintStringAndHex(L"SPARange TypeGUID[1]",
                           spa_range->address_range_type_guid[1]);
      uint64_t* p = (uint64_t*)spa_range->system_physical_address_range_base;
      EFIPrintStringAndHex(L"PMEM Previous value: ", *p);
      *p = *p + 3;
      EFIPrintStringAndHex(L"PMEM value after write: ", *p);
    }
  }

  if (hpet) {
    EFIPutCString("HPET found.");
    EFIPrintStringAndHex(L"base addr", (uint64_t)hpet->base_address.address);
    HPETRegisterSpace* hpet_registers = hpet->base_address.address;
    uint64_t general_config = hpet_registers->general_configuration;
    EFIPrintStringAndHex(L"geneal cap",
                         hpet_registers->general_capabilities_and_id);
    EFIPrintStringAndHex(L"geneal configuration", general_config);
    general_config |= 1;
    hpet_registers->general_configuration = general_config;

    EFIPrintStringAndHex(L"geneal configuration",
                         hpet_registers->general_configuration);
    while (1) {
      EFIPrintStringAndHex(L"counter", hpet_registers->main_counter_value);

      EFIGetChar();
    };
  }

  // EFIPrintMemoryMap();
  while (1) {
    wchar_t c = EFIGetChar();
    EFIPutChar(c);
  };
}
