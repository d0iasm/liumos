#include "liumos.h"

void IA_PDT::Print() {
  for (int i = 0; i < kNumOfPDE; i++) {
    if (!entries[i].IsPresent())
      continue;
    PutString("PDT[");
    PutHex64(i);
    PutString("]:\n");
    if (entries[i].IsPage()) {
      PutString(" 2MB Page\n");
      continue;
    }
    PutStringAndHex(" addr", entries[i].GetTableAddr());
  }
}

void IA_PDPT::Print() {
  for (int i = 0; i < kNumOfPDPTE; i++) {
    if (!entries[i].IsPresent())
      continue;
    PutString("PDPT[");
    PutHex64(i);
    PutString("]:\n");
    if (entries[i].IsPage()) {
      PutString(" 1GB Page\n");
      continue;
    }
    IA_PDT* pdt = entries[i].GetTableAddr();
    pdt->Print();
  }
}

void IA_PML4::Print() {
  for (int i = 0; i < kNumOfPML4E; i++) {
    if (!entries[i].IsPresent())
      continue;
    PutString("PML4[");
    PutHex64(i);
    PutString("]:\n");
    IA_PDPT* pdpt = entries[i].GetTableAddr();
    pdpt->Print();
  }
}