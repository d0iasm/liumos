#pragma once

#include "liumos.h"
#include "paging.h"

template <typename VirtType, typename PhysType>
PhysType v2p(VirtType v) {
  return reinterpret_cast<PhysType>(
      liumos->kernel_pml4->v2p(reinterpret_cast<uint64_t>(v)));
}

template <typename T>
T AllocMemoryForMappedIO(uint64_t byte_size) {
  return liumos->kernel_heap_allocator->AllocPages<T>(
      ByteSizeToPageSize(byte_size), kPageAttrMemMappedIO);
}