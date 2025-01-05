#ifndef _RISCV_TYPE_TAG_H
#define _RISCV_TYPE_TAG_H

#include <cmath>
#include <cstdint>

enum ObjType : uint8_t {
	OBJ_SPECIAL = 0,
	OBJ_CODE,
	OBJ_COLLECTION,
	OBJ_RAW,
	OBJ_UINT,
	OBJ_INT,
	OBJ_FLOAT,
	OBJ_REF
};

enum RefType : uint8_t {
	REF_NONE = 0,
	REF_BASIC,
	REF_COLLECTION,
	REF_COLLECTION_REF
};

typedef uint8_t raw_tag_t;
class TypeTag {
public:
	TypeTag(raw_tag_t tag) {
		this->_tag = tag;
	}

	raw_tag_t raw_tag() const {
		return _tag;
	}

	/// Size of this object in bytes {1, 2, 4, 8}
	int obj_size() const {
		// Size in bytes is 2^{bits}
		return (1 << (int)get_bits(0, 2));
	}

	ObjType type() const {
		return (ObjType)get_bits(2, 3);
	}

	RefType ref_type() const {
		return (RefType)get_bits(5, 2);
	}

	bool multibyte() const {
		return get_bits(7, 1);
	}

private:
	raw_tag_t _tag; 

	raw_tag_t get_bits(short pos, short len) const {
		return (this->_tag >> pos) & ((raw_tag_t(1) << len) - 1);
	}
};

#endif