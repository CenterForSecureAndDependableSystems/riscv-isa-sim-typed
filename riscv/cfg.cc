// See LICENSE for license details.

#include "config.h"
#include "cfg.h"
#include "mmu.h"
#include "decode.h"
#include "encoding.h"
#include "platform.h"

tag_regions_t::tag_regions_t(const std::vector<tag_mapping_cfg_t>& tag_mappings) {
  for(const auto& cfg : tag_mappings) {
    mem_to_tag.insert(std::make_pair(cfg.get_mapped_base(), cfg));
    tag_to_mem.insert(std::make_pair(cfg.get_base(), cfg));
  }
}

reg_t tag_regions_t::get_tag_addr(reg_t mem_addr) const {
  auto it = mem_to_tag.upper_bound(mem_addr);
  if(mem_to_tag.empty() || it == mem_to_tag.begin())
    return 0; // addr is before any mapped regions
  const tag_mapping_cfg_t& m = std::prev(it)->second;

  if(mem_addr >= m.get_mapped_base() + m.get_size())
    return 0; // Past the mapped region

  return mem_addr + m.get_offset(); 
}

reg_t tag_regions_t::get_base_addr(reg_t tag_addr) const {
  auto it = tag_to_mem.upper_bound(tag_addr);
  if(tag_to_mem.empty() || it == tag_to_mem.begin())
    return 0; // addr is before any tag regions
  const tag_mapping_cfg_t& m = std::prev(it)->second;

  if(tag_addr >= m.get_base() + m.get_size())
    return 0; // Past the tag region

  return tag_addr - m.get_offset(); 
}

bool tag_regions_t::addr_in_tag_region(reg_t addr) const {
  auto it = tag_to_mem.upper_bound(addr);
  if(tag_to_mem.empty() || it == tag_to_mem.begin())
    return false; // addr is before any tag regions
  const tag_mapping_cfg_t& m = std::prev(it)->second;

  return addr <= m.get_inclusive_end();
}

bool tag_regions_t::intersects_tag_region(reg_t addr, reg_t len) const {
  reg_t end = addr + len;
  auto lower = tag_to_mem.upper_bound(addr);
  auto upper = tag_to_mem.upper_bound(end);

  if(lower != upper) {
    return true; // Range intersects at least one other tag region
  }
  if(lower == tag_to_mem.begin()) {
    return false; // lower and upper precede any tag regions
    // Return early so we don't run std::prev on a begin() iterator
  }

  const tag_mapping_cfg_t& m = std::prev(lower)->second;
  if(addr <= m.get_inclusive_end()) {
    return true; // Range starts inside a tag region
  }

  return false;
}

mem_cfg_t::mem_cfg_t(reg_t base, reg_t size) : base(base), size(size)
{
  assert(mem_cfg_t::check_if_supported(base, size));
}

tag_mapping_cfg_t::tag_mapping_cfg_t(reg_t base, reg_t size, reg_t mapped_base) 
  : mem_cfg_t(base, size)
{

}

bool mem_cfg_t::check_if_supported(reg_t base, reg_t size)
{
  // The truth of these conditions should be ensured by whatever is creating
  // the regions in the first place, but we have them here to make sure that
  // we can't end up describing memory regions that don't make sense. They
  // ask that the page size is a multiple of the minimum page size, that the
  // page is aligned to the minimum page size, that the page is non-empty,
  // that the size doesn't overflow size_t, and that the top address is still
  // representable in a reg_t.
  //
  // Note: (base + size == 0) part of the assertion is to handle cases like
  //   { base = 0xffff_ffff_ffff_f000, size: 0x1000 }
  return (size % PGSIZE == 0) &&
         (base % PGSIZE == 0) &&
         (size_t(size) == size) &&
         (size > 0) &&
         ((base + size > base) || (base + size == 0));
}

cfg_t::cfg_t()
{
  // The default system configuration
  initrd_bounds    = std::make_pair((reg_t)0, (reg_t)0);
  bootargs         = nullptr;
  isa              = DEFAULT_ISA;
  priv             = DEFAULT_PRIV;
  misaligned       = false;
  endianness       = endianness_little;
  pmpregions       = 16;
  pmpgranularity   = (1 << PMP_SHIFT);
  mem_layout       = std::vector<mem_cfg_t>({mem_cfg_t(reg_t(DRAM_BASE), (size_t)2048 << 20)});
  hartids          = std::vector<size_t>({0});
  explicit_hartids = false;
  real_time_clint  = false;
  trigger_count    = 4;
}
