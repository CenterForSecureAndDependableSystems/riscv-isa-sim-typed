// See LICENSE for license details.
#ifndef _RISCV_CFG_H
#define _RISCV_CFG_H

#include <optional>
#include <vector>
#include "decode.h"
#include <cassert>

typedef enum {
  endianness_little,
  endianness_big
} endianness_t;

template <typename T>
class cfg_arg_t {
public:
  cfg_arg_t(T default_val)
    : value(default_val), was_set(false) {}

  bool overridden() const { return was_set; }

  T operator()() const { return value; }

  T operator=(const T v) {
    value = v;
    was_set = true;
    return value;
  }

private:
  T value;
  bool was_set;
};

// Configuration that describes a memory region
class mem_cfg_t
{
public:
  static bool check_if_supported(reg_t base, reg_t size);

  mem_cfg_t(reg_t base, reg_t size);

  reg_t get_base() const {
    return base;
  }

  reg_t get_size() const {
    return size;
  }

  reg_t get_inclusive_end() const {
    return base + size - 1;
  }

private:
  reg_t base;
  reg_t size;
};

class tag_region_t
{
public:
  /// Create a tag memory region at `base:size`, representing the 
  /// memory tags for an existing region at `mapped_base:size`. 
  tag_region_t(reg_t tag_base, reg_t size, reg_t mapped_base);

  reg_t get_tag_base() const {
    return tag_base;
  }

  reg_t get_mapped_base() const {
    return mapped_base;
  }

  reg_t get_size() const {
    return size;
  }

  reg_t get_tag_end() const {
    return tag_base + size;
  }

  reg_t get_mapped_end() const {
    return mapped_base + size;
  }

  reg_t get_tag_inclusive_end() const {
    return tag_base + size - 1;
  }

  reg_t get_mapped_inclusive_end() const {
    return mapped_base + size - 1;
  }

  /// Return the offset from mapped_base to tag_base
  long int get_offset() const {
    return (long int)tag_base - (long int)mapped_base;
  }

private:
  reg_t tag_base;
  reg_t mapped_base;
  reg_t size;
};

class cfg_t
{
public:
  cfg_t();

  std::pair<reg_t, reg_t> initrd_bounds;
  const char *            bootargs;
  const char *            isa;
  const char *            priv;
  bool                    misaligned;
  endianness_t            endianness;
  reg_t                   pmpregions;
  reg_t                   pmpgranularity;
  std::vector<mem_cfg_t>  mem_layout;
  std::vector<tag_region_t>  tag_mem_mappings;
  std::optional<reg_t>    start_pc;
  std::vector<size_t>     hartids;
  bool                    explicit_hartids;
  bool                    real_time_clint;
  reg_t                   trigger_count;

  size_t nprocs() const { return hartids.size(); }
  size_t max_hartid() const { return hartids.back(); }
};

#endif
