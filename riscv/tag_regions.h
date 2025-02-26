#ifndef _RISCV_TAG_REGIONS_H
#define _RISCV_TAG_REGIONS_H

#include "cfg.h"
#include "memtracer.h"
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

/// A bidirectional map for translating memory addresses to their
/// corresponding tag addresses based on `tag_mappings`.
class tag_regions_t {
public:
  tag_regions_t(const std::vector<tag_mapping_cfg_t>& tag_mappings);

  reg_t get_tag_addr(reg_t mem_addr) const;
  reg_t get_base_addr(reg_t tag_addr) const;
  bool addr_in_tag_region(reg_t addr) const;
  bool intersects_tag_region(reg_t addr, reg_t len) const;

private:
  std::map<reg_t, tag_mapping_cfg_t> mem_to_tag;
  std::map<reg_t, tag_mapping_cfg_t> tag_to_mem; 
};


#endif // _RISCV_TAG_REGIONS_H