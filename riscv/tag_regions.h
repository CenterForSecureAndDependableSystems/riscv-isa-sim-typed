#ifndef _RISCV_TAG_REGIONS_H
#define _RISCV_TAG_REGIONS_H

#include "cfg.h"
#include "decode.h"
#include "elfloader.h"
#include <exception>
#include <map>

class tag_region_fault : public std::exception {
public:
	tag_region_fault(reg_t addr)
		: addr(addr) {}

    const char* what() const throw() {
        return "No tag region associated with address";
    }

private:
	reg_t addr;
};

class tag_region_definition_fault : public std::exception {
public:
	tag_region_definition_fault(std::string msg)
		: msg(msg) {}

    const char* what() const throw() {
        return msg.c_str();
    }

private:
  std::string msg;
};

/// A bidirectional map for translating memory addresses to their
/// corresponding tag addresses based on `tag_mappings`.
class tag_regions_t {
public:
  tag_regions_t() {};
  tag_regions_t(std::vector<tag_region_t> const& tag_mappings);

  void map_region(tag_region_t const& cfg, bool remap = false);

  reg_t get_tag_addr(reg_t mem_addr) const;
  reg_t get_mapped_addr(reg_t tag_addr) const;
  bool addr_in_tag_region(reg_t addr) const;
  bool intersects_tag_region(reg_t base, reg_t len) const;
  bool intersects_mapped_region(reg_t base, reg_t len) const;

  static tag_regions_t find_elf_tag_regions(
    std::vector<ElfSection64> const& sections,
    tag_regions_t existing_regions = tag_regions_t());

private:
  typedef std::map<reg_t, tag_region_t>::iterator RegionIter;
  typedef std::map<reg_t, tag_region_t>::const_iterator RegionConstIter;

  std::map<reg_t, tag_region_t> mapped_regions;
  std::map<reg_t, tag_region_t> tag_regions; 

  void insert_region(tag_region_t cfg);
  void erase_region(tag_region_t cfg);

  std::vector<tag_region_t> intersected_regions(
    std::map<reg_t, tag_region_t> const& map, 
    reg_t base, 
    reg_t len) const;
};


#endif // _RISCV_TAG_REGIONS_H