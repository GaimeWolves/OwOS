#include <panic.hpp>

#include <atomic>

#include <logging/logger.hpp>
#include <stdint.h>

#define is_aligned(value, alignment) (value & (alignment - 1))

class SourceLocation
{
public:
	SourceLocation() = default;

	SourceLocation(const char *filename, uint32_t line, uint32_t column)
	    : filename(filename), line(line), column(column)
	{
	}

	[[nodiscard]] bool isValid() const { return filename; }

	SourceLocation acquire()
	{
		uint32_t old_column = __atomic_exchange_n(&column, UINT32_MAX, __ATOMIC_RELAXED);
		return {filename, line, old_column};
	}

	[[nodiscard]] bool is_disabled() const { return column == UINT32_MAX; }

	[[nodiscard]] const char *get_filename() const { return filename; }
	[[nodiscard]] uint32_t get_line() const { return line; }
	[[nodiscard]] uint32_t get_column() const { return column; }

private:
	const char *filename;
	uint32_t line;
	uint32_t column;
};

class TypeDescriptor
{
public:
	[[nodiscard]] const char *get_name() const { return name; }

private:
	uint16_t kind;
	uint16_t info;
	char name[1];
};

typedef struct __type_mismatch_info_t
{
	SourceLocation location;
	const TypeDescriptor &type;
	uint8_t alignment;
	uint8_t type_check_kind;
} type_mismatch_info_t;

const char *const type_check_kinds[] = {
    "load of",
    "store to",
    "reference binding to",
    "member access within",
    "member call on",
    "constructor call on",
    "downcast of",
    "downcast of",
    "upcast of",
    "cast to virtual base of",
    "_Nonnull binding to",
    "dynamic operation on",
};

static void log_location(SourceLocation &location)
{
	if (!location.isValid() || location.is_disabled())
	{
		Kernel::log("UBSAN", "No source information available");
		return;
	}

	SourceLocation loc = location.acquire();
	Kernel::log("UBSAN", "At %s:%d:%d", loc.get_filename(), loc.get_line(), loc.get_column());
}

extern "C" __noreturn __used void __ubsan_handle_type_mismatch_v1(type_mismatch_info_t *type_mismatch, uintptr_t address)
{
	const char *const type = type_mismatch->type.get_name();
	const char *const kind = type_check_kinds[type_mismatch->type_check_kind];

	log_location(type_mismatch->location);

	if (!address)
	{
		Kernel::log("UBSAN", "%s null pointer access of type %s", kind, type);
	}
	else if (type_mismatch->alignment != 0 && !is_aligned(address, type_mismatch->alignment))
	{
		Kernel::log("UBSAN", "%s misaligned address %p for type %s which requires %d byte alignment", kind, address, type, type_mismatch->alignment);
	}
	else
	{
		Kernel::log("UBSAN", "%s address %p with insufficient space for an object of type %s", kind, address, type);
	}

	Kernel::panic("UBSAN");
}

extern "C" __noreturn __used void __ubsan_handle_load_invalid_value(void *, uintptr_t)
{
	Kernel::panic("UBSAN");
}

extern "C" __noreturn __used void __ubsan_handle_nonnull_arg(void *)
{
	Kernel::panic("UBSAN");
}

extern "C" __noreturn __used void __ubsan_handle_nullability_arg(void *)
{
	Kernel::panic("UBSAN");
}

extern "C" __noreturn __used void __ubsan_handle_nonnull_return_v1(void *, void *)
{
	Kernel::panic("UBSAN");
}

extern "C" __noreturn __used void __ubsan_handle_nullability_return_v1(void *, void *)
{
	Kernel::panic("UBSAN");
}

extern "C" __noreturn __used void __ubsan_handle_vla_bound_not_positive(void *, uintptr_t)
{
	Kernel::panic("UBSAN");
}

extern "C" __noreturn __used void __ubsan_handle_add_overflow(void *, uintptr_t, uintptr_t)
{
	Kernel::panic("UBSAN");
}

extern "C" __noreturn __used void __ubsan_handle_sub_overflow(void *, uintptr_t, uintptr_t)
{
	Kernel::panic("UBSAN");
}

extern "C" __noreturn __used void __ubsan_handle_negate_overflow(void *, uintptr_t)
{
	Kernel::panic("UBSAN");
}

extern "C" __noreturn __used void __ubsan_handle_mul_overflow(void *, uintptr_t, uintptr_t)
{
	Kernel::panic("UBSAN");
}

extern "C" __noreturn __used void __ubsan_handle_shift_out_of_bounds(void *, uintptr_t, uintptr_t)
{
	Kernel::panic("UBSAN");
}

extern "C" __noreturn __used void __ubsan_handle_divrem_overflow(void *, uintptr_t, uintptr_t)
{
	Kernel::panic("UBSAN");
}

extern "C" __noreturn __used void __ubsan_handle_out_of_bounds(void *, uintptr_t)
{
	Kernel::panic("UBSAN");
}

extern "C" __noreturn __used void __ubsan_handle_alignment_assumption(void *, uintptr_t, uintptr_t, uintptr_t)
{
	Kernel::panic("UBSAN");
}

extern "C" __noreturn __used void __ubsan_handle_builtin_unreachable(void *)
{
	Kernel::panic("UBSAN");
}

extern "C" __noreturn __used void __ubsan_handle_missing_return(void *)
{
	Kernel::panic("UBSAN");
}

extern "C" __noreturn __used void __ubsan_handle_implicit_conversion(void *, uintptr_t, uintptr_t)
{
	Kernel::panic("UBSAN");
}

extern "C" __noreturn __used void __ubsan_handle_invalid_builtin(void *)
{
	Kernel::panic("UBSAN");
}

extern "C" __noreturn __used void __ubsan_handle_pointer_overflow(void *, uintptr_t, uintptr_t)
{
	Kernel::panic("UBSAN");
}
