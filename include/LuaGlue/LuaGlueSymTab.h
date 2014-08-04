#ifndef LUAGLUE_SYMTAB_H_GUARD
#define LUAGLUE_SYMTAB_H_GUARD

#include <string.h>
#include <vector>
#include <typeinfo>


#include "LuaGlue/LuaGlueCompat.h"

template<class T>
class LuaGlueSymTab
{
	private:
		
		struct Symbol {
			char *name;
			const char *typeid_name;
			T ptr; int idx; LUA_UNSIGNED lg_typeid;
			
			Symbol(const char *n = nullptr, const char *tn = nullptr, T p = nullptr, int i = -1, LUA_UNSIGNED lg_typeid_ = (LUA_UNSIGNED)-1)
				: name(nullptr), typeid_name(tn), ptr(p), idx(i), lg_typeid(lg_typeid_)
			{
				name = n ? strdup(n) : nullptr;
				//printf("new Symbol(\"%s\", \"%s\", %p, %i)\n", n, tn, p, idx);
			}
			
			Symbol(const Symbol &s)
				: name(s.name ? strdup(s.name) : nullptr), typeid_name(s.typeid_name), ptr(s.ptr), idx(s.idx), lg_typeid(s.lg_typeid)
			{
				
			}
			
			const Symbol &operator=(const Symbol &rhs)
			{
				if(name)
					free(name);
				
				name = strdup(rhs.name);
				typeid_name = rhs.typeid_name;
				ptr = rhs.ptr;
				idx = rhs.ptr;
				lg_typeid = rhs.lg_typeid;
				
				return *this;
			}
			
			~Symbol()
			{
				if(name)
					free((void*)name);
			}
		
			private:
			
		};
		
	public:
		LuaGlueSymTab()
		{
		
		}
		
		~LuaGlueSymTab()
		{
			for(auto &i: items)
				delete i.ptr;
		}
		
		template<class C>
		const Symbol &addSymbol(const char *name, C *ptr)
		{
			const Symbol &sym = findSym(name);
			if(sym.name)
				return sym;
			
			items.push_back(Symbol(name, typeid(C).name(), ptr, items.size()));
			return items.back();
		}
		
		template<class C>
		const Symbol &addSymbol(const char *name, C *ptr, LUA_UNSIGNED tid)
		{
			const Symbol &sym = findSym(name);
			if(sym.name)
				return sym;
			
			items.push_back(Symbol(name, typeid(C).name(), ptr, items.size(), tid));
			return items.back();
		}
		
		/*
		T &operator[]( const char *key )
		{
			Symbol &sym = findSym(key);
			if(!sym.name)
			{
				items.push_back(Symbol(key, nullptr, items.size()));
				Symbol &new_sym = items.back();
				new_sym.name = strdup(key);
				return new_sym.ptr;
			}
			
			return sym.ptr;
		}
		
		T &operator[]( const std::string &key )
		{
			const char *ckey = key.c_str();
			Symbol &sym = findSym(ckey);
			if(!sym.name)
			{
				items.push_back(Symbol(ckey, nullptr, items.size()));
				Symbol &new_sym = items.back();
				new_sym.name = strdup(ckey);
				return new_sym.ptr;
			}
			
			return sym.ptr;
		}
		*/
		
		typename std::vector<Symbol>::iterator begin()
		{
			return items.begin();
		}

		typename std::vector<Symbol>::iterator end()
		{
			return items.end();
		}
		
		bool exists(const char *key, bool internal_name = false)
		{
			if(internal_name)
				return findSym_int(key).name != nullptr;
			
			return findSym(key).name != nullptr;
		}
		
		T lookup(const char *key, bool internal_name = false)
		{
			if(internal_name)
				return findSym_int(key).ptr;
			
			return findSym(key).ptr;
		}
		
		T lookup(uint32_t idx)
		{
			return findSym(idx).ptr;
		}
		
		T lookupByLGTypeID(LUA_UNSIGNED id)
		{
			return findSym_lgtypeid(id).ptr;
		}
		
		const Symbol &findSym(const char *name)
		{
			for(auto &sym: items)
			{
				//printf("findSym: %s <=> %s\n", sym.name, name);
				if(strcmp(sym.name, name) == 0)
					return sym;
			}
			
			return nullSymbol;
		}
		
	private:
		
		static const Symbol nullSymbol;
		std::vector<Symbol> items;
		
		const Symbol &findSym_int(const char *name)
		{
			for(auto &sym: items)
			{
				//printf("findSym_int: %s <=> %s\n", sym.typeid_name, name);
				if(strcmp(sym.typeid_name, name) == 0)
					return sym;
			}
			
			return nullSymbol;
		}
		
		const Symbol &findSym_lgtypeid(LUA_UNSIGNED id)
		{
			for(auto &sym: items)
			{
				if(sym.lg_typeid == id)
					return sym;
			}
			
			return nullSymbol;
		}
		
		const Symbol &findSym(uint32_t idx)
		{
			if(idx > items.size())
			{
				//printf("findSym(%i): not found\n", idx);
				return nullSymbol;
			}
			
			//printf("findSym(%i): %s\n", idx, items[idx].name);
			return items[idx];
		}
		
		
};

template<typename T>
const typename LuaGlueSymTab<T>::Symbol LuaGlueSymTab<T>::nullSymbol;

#endif /* LUAGLUE_SYMTAB_H_GUARD */
