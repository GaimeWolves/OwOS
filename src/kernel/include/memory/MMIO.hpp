#pragma once

#include <stddef.h>
#include <stdint.h>

#include <libk/kcassert.hpp>

#include <memory/VirtualMemoryManager.hpp>

namespace Kernel::Memory
{
	template <typename T>
	class MMIO
	{
	public:
		MMIO() = default;

		MMIO(uintptr_t address, size_t size)
		    : m_size(size)
		{
			mapping_config_t config;
			config.cacheable = false;
			m_region = VirtualMemoryManager::instance().map_region(address, size, config);
			m_ref = reinterpret_cast<T *>(m_region.virt_address);
			assert(m_ref);
		}

		MMIO(const MMIO &other) = delete;
		MMIO &operator=(const MMIO &other) = delete;

		MMIO(MMIO &&other) noexcept
			: m_ref(other.m_ref)
		    , m_region(other.m_region)
		    , m_size(other.m_size)
		{
			other.m_ref = nullptr;
			other.m_size = 0;
			other.m_region = {};
		}

		MMIO &operator=(MMIO &&other) noexcept
		{
			this->~MMIO();

			m_ref = other.m_ref;
			m_size = other.m_size;
			m_region = other.m_region;

			other.m_ref = nullptr;
			other.m_size = 0;
			other.m_region = {};

			return *this;
		}

		~MMIO()
		{
			if (m_ref)
				VirtualMemoryManager::instance().free(m_region);
		}

		T &operator*() { return *m_ref; }

		T &operator[](int n)
		{
			assert(n >= 0);
			assert(n * sizeof(T) < m_size);
			return m_ref[n];
		}

		T *operator+(int offset)
		{
			assert(offset >= 0);
			assert(offset * sizeof(T) < m_size);
			return &m_ref[offset];
		}

		T *operator()() { return m_ref; }

		T *operator->() const { return m_ref; }

	private:
		T *m_ref{nullptr};
		memory_region_t m_region{};
		size_t m_size{0};
	};
} // namespace Kernel::Memory
