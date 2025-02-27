#include "tag_regions.h"

tag_regions_t::tag_regions_t(const std::vector<tag_mapping_cfg_t>& tag_mappings) {
  for(const auto& cfg : tag_mappings) {
    mem_to_tag.insert(std::make_pair(cfg.get_mapped_base(), cfg));
    tag_to_mem.insert(std::make_pair(cfg.get_base(), cfg));
  }
}

reg_t tag_regions_t::get_tag_addr(reg_t mem_addr) const {
  auto it = mem_to_tag.upper_bound(mem_addr);
  if(mem_to_tag.empty() || it == mem_to_tag.begin())
	throw tag_region_fault(mem_addr); // addr is before any mapped regions
  const tag_mapping_cfg_t& m = std::prev(it)->second;

  if(mem_addr >= m.get_mapped_base() + m.get_size())
	throw tag_region_fault(mem_addr); // Past the mapped region

  return mem_addr + m.get_offset(); 
}

reg_t tag_regions_t::get_base_addr(reg_t tag_addr) const {
  auto it = tag_to_mem.upper_bound(tag_addr);
  if(tag_to_mem.empty() || it == tag_to_mem.begin())
	throw tag_region_fault(tag_addr); // addr is before any tag regions
  const tag_mapping_cfg_t& m = std::prev(it)->second;

  if(tag_addr >= m.get_base() + m.get_size())
	throw tag_region_fault(tag_addr); // Past the tag region

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