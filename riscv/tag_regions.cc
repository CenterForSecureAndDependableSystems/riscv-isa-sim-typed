#include "config.h"
#include "tag_regions.h"
#include <sstream>

tag_regions_t::tag_regions_t(std::vector<tag_region_t> const& tag_mappings) {
  for(const auto& cfg : tag_mappings) {
    map_region(cfg);
  }
}

// void tag_regions_t::map_region(reg_t base, reg_t, reg_t len, reg_t tag_base) {
//   map_region(tag_mapping_cfg_t(base, len, tag_base));
// }

void tag_regions_t::map_region(tag_region_t const& cfg, bool remap) {
  reg_t mapped_base = cfg.get_mapped_base();
  reg_t tag_base = cfg.get_tag_base();
  reg_t len = cfg.get_size();

  // We don't want new mapped regions *or* tag regions to be put on top of
  // an existing tag region, as it may still be in use.
  if(intersects_tag_region(mapped_base, len) || intersects_tag_region(tag_base, len)) {
    throw tag_region_definition_fault("Attempted to create a memory region which overlaps with an existing tag memory region");
  }

  // We are able to 'move' a mapped region to a different tag region and vice versa
  // (like we might when loading an ELF file on top of an already mapped region
  // for some subregions within it).

  // This is a lambda so we can make adjustments for both the new mapped region,
  // and the new tag region.
  auto remap_regions = [this, remap](reg_t base, reg_t len) {
    reg_t end = base + len; // The end of the new region

    auto regions = intersected_regions(mapped_regions, base, len);
    if(!regions.empty()) {
      if(!remap) {
        std::ostringstream stream;
        stream << "Attempted to create a new tag mapping ("
              << base << ':' << len
              << ") on top of one or more already mapped regions: "; 

        for(auto const& r : regions) {
          stream << r.get_mapped_base() << ':' << r.get_size() << ", ";
        }
        throw tag_region_definition_fault(stream.str());
      }

      // Last region might straddle the new region's end, resize by moving the
      // mapped base up.
      auto last = regions.back();
      if(last.get_mapped_inclusive_end() > end && last.get_mapped_base() < end) {
        assert(end >= last.get_mapped_base());
        reg_t diff = end - last.get_mapped_base();

        tag_region_t new_region(
          last.get_tag_base() + diff, // Shift the tag base up, creates some dead space 
          last.get_size() - diff,
          end // Set the mapped base to the end of the new region
        );
        erase_region(last);
        insert_region(new_region);
      }
      // Last region should be done before first, as first will
      // hold the same base address and might get erased above.

      // First region might straddle the new region's beginning, resize by moving
      // the length back.
      auto first = regions.front();
      if(first.get_mapped_base() < base && first.get_mapped_inclusive_end() > base) {
        tag_region_t new_region(
          first.get_tag_base(), 
          base - first.get_mapped_base(), // msize - ((mbase + msize) - base)
          first.get_mapped_base()
        );
        erase_region(first);
        insert_region(new_region);
      }

      // All other intersected regions are fully contained within the new mapped
      // region, and can be removed.
      for(size_t i = 1; i < regions.size() - 1; i++) {
        erase_region(regions[i]);
      }
    }
  };

  remap_regions(mapped_base, len);
  remap_regions(tag_base, len);

  insert_region(cfg);
}

void tag_regions_t::erase_region(tag_region_t cfg) {
  mapped_regions.erase(cfg.get_mapped_base());
  tag_regions.erase(cfg.get_tag_base());
}

void tag_regions_t::insert_region(tag_region_t cfg) {
  mapped_regions.insert(std::make_pair(cfg.get_mapped_base(), cfg));
  tag_regions.insert(std::make_pair(cfg.get_tag_base(), cfg));
}

reg_t tag_regions_t::get_tag_addr(reg_t mem_addr) const {
  auto it = mapped_regions.upper_bound(mem_addr);
  if(mapped_regions.empty() || it == mapped_regions.begin())
    throw tag_region_fault(mem_addr); // addr is before any mapped regions
  const tag_region_t& m = std::prev(it)->second;

  if(mem_addr >= m.get_mapped_base() + m.get_size())
  	throw tag_region_fault(mem_addr); // Past the mapped region

  return mem_addr + m.get_offset(); 
}

reg_t tag_regions_t::get_mapped_addr(reg_t tag_addr) const {
  auto it = tag_regions.upper_bound(tag_addr);
  if(tag_regions.empty() || it == tag_regions.begin())
    throw tag_region_fault(tag_addr); // addr is before any tag regions
  const tag_region_t& m = std::prev(it)->second;

  if(tag_addr >= m.get_tag_base() + m.get_size())
  	throw tag_region_fault(tag_addr); // Past the tag region

  return tag_addr - m.get_offset(); 
}

bool tag_regions_t::addr_in_tag_region(reg_t addr) const {
  auto it = tag_regions.upper_bound(addr);
  if(tag_regions.empty() || it == tag_regions.begin())
    return false; // addr is before any tag regions
  const tag_region_t& m = std::prev(it)->second;

  return addr <= m.get_tag_inclusive_end();
}

bool tag_regions_t::intersects_tag_region(reg_t base, reg_t len) const {
  return !intersected_regions(tag_regions, base, len).empty();
}

bool tag_regions_t::intersects_mapped_region(reg_t base, reg_t len) const {
  return !intersected_regions(mapped_regions, base, len).empty();
}

std::vector<tag_region_t> 
tag_regions_t::intersected_regions(
  std::map<reg_t, tag_region_t> const& map, 
  reg_t base, 
  reg_t len) const
{
  reg_t end = base + len;
  auto lower = map.lower_bound(base);
  auto upper = map.upper_bound(end);
  if(map.empty()) {
    return {};
  }

  if(lower != upper) {
    std::vector<tag_region_t> result;
    while(lower != upper) {
      result.push_back(lower->second);
      lower++;
    }
    return result; // Range intersects at least one other region
  }
  if(lower == map.begin()) {
    // Also implies upper == map.begin().
    // lower and upper precede any tag regions,
    // return early so we don't run std::prev on a begin() iterator
    return {};
  }

  // The lower and upper bounds are equal, but the upper bound only starts
  // at the next region, we still need to see if the region before us is large
  // enough to intersect our base.
  auto lower_prev = std::prev(lower);
  if(base < lower_prev->second.get_tag_inclusive_end()) {
    return {lower_prev->second}; // Range's base falls inside a tag region
  }

  return {};
}

tag_regions_t tag_regions_t::find_elf_tag_regions(
  std::vector<ElfSection64> const& sections,
  tag_regions_t tag_regions) 
{
  for(ElfSection64 const& section : sections) {
    // If the section is a custom tag region...
    if(section.type == 0x700000000) {
      uint64_t mapped_section_idx = section.link;
      if(mapped_section_idx > sections.size()) {
        throw tag_region_definition_fault("Mapped section referenced in tag section header does not exist");
      }
      uint64_t mapped_virt_addr = sections.at(mapped_section_idx).virt_addr;

      tag_region_t tag_region(section.virt_addr, section.size, mapped_virt_addr);
      tag_regions.map_region(tag_region, true);

      #ifdef TYPE_TAGGING_DEBUG
        printf("[SPIKE DEBUG] Mapping region %#04lx:%#04lx to tag region %#04lx:%#04lx\n", 
          tag_region.get_mapped_base(), tag_region.get_size(),
          tag_region.get_tag_base(), tag_region.get_size()
        );
      #endif
    }
  }

  #ifdef TYPE_TAGGING_DEBUG
  for(auto const& pair : tag_regions.mapped_regions) {
    auto const& tag_region = pair.second;
    printf("[SPIKE DEBUG] mem %#04lx:%#04lx -> tag %#04lx:%#04lx\n", 
      tag_region.get_mapped_base(), tag_region.get_mapped_end(),
      tag_region.get_tag_base(), tag_region.get_tag_end()
    );
  }
  #endif

  return tag_regions;
}
