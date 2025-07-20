// *************************************************************************
// [Author]		xiong.qiang
// [Date]		2023-11-04
// [Describe]	Memory define of nimble framwork 
// *************************************************************************
#pragma once

#include "x/x.hpp"
#include <shared_mutex>

namespace nb {

struct InfoMemory {
	x::u32 size = 0;
	std::string info;

	InfoMemory() {}

	InfoMemory(const x::u32 &sz,const std::string & info) noexcept{
		this->size = sz;
		this->info = info;
	}
};

class Memory {

private:
	std::unordered_map<x::u64,InfoMemory>  records;
	mutable std::shared_mutex			   mtx;

	Memory() {}

public:

	static Memory * instance() {
		static Memory memory;
		return &memory;
	}
	
	void add(const x::u64 & addr, const InfoMemory &info) noexcept {
		std::lock_guard<std::shared_mutex> lk(mtx);
		records[addr] = info;
	}

	void remove(const x::u64 & addr) noexcept {
		std::lock_guard<std::shared_mutex> lk(mtx);
		auto it = records.find(addr);
		if (it != records.end())
			records.erase(it);
	}

	x::u64 size() const noexcept{
		std::shared_lock<std::shared_mutex> lk(mtx);
		x::u64 len = 0;
		for (const auto &it : records)
			len += it.second.size;
		return len;
	}

	std::string dump() const noexcept {
		std::string info = "\n----------------- Memeory -----------------\n";
		std::shared_lock<std::shared_mutex> lk(mtx);
		if (records.size() <= 0){
			info += "^_^ Nice,memory all released. ^_^ \n";
			info += "-----------------!Memeory!-----------------\n";
			return info;
		}
		x::u64 totalBytes = 0;
		x::u64 index = 0;
		for (const auto &it : records) {
			totalBytes += it.second.size;
			info += std::format("<{:05}>   [Addr] = {:<#16X}   [Size] = {:10}   [Info] = {}\n",
							index++,it.first,it.second.size,it.second.info);
		}
		info += std::format("<Total> = {} bytes\n", totalBytes);
		info += "-----------------!Memeory!-----------------\n\n";
		return info;
	}
};

extern Memory * g_memory;

template <class T>
struct Delete{
	void operator()(T *p) {
		if (g_memory != nullptr)
			g_memory->remove((x::u64)p);
		delete[] p;
	}
};

template <class T>
inline std::unique_ptr<T,Delete<T>> New(const x::u32 & size,const std::string& info) {
	if (g_memory == nullptr)
		throw std::runtime_error(("NB::New() not set g_memory.  --->  " + info).c_str());
	std::unique_ptr<T, Delete<T>> p(new T[size], Delete<T>());
	if(p)
		g_memory->add((x::u64)p.get(), InfoMemory(size * sizeof(T), info));
	return p;
}

} // namespace nb