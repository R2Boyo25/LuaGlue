#ifndef LUAGLUE_TYPE_BASE_H
#define LUAGLUE_TYPE_BASE_H

#include <random>
#include <chrono>

#include <lua5.4/lua.hpp>

#if LUAI_BITSINT > 32
#	define chrono_period std::chrono::nanoseconds
#elseif LUAI_BITSINT < 32
#	error unsupported LUA_UNSIGNED type
#else
#	define chrono_period std::chrono::milliseconds
#endif

#include <LuaGlue/LuaGlueDebug.h>

class LuaGlueBase;

class LuaGlueTypeBase
{
	template<typename _T>
	friend class LuaGlueType;
	
	template<typename _T>
	friend class LuaGlueTypeValueImpl;
	
	public:
		constexpr static const char *METATABLE_TYPENAME_FIELD = "TypeName";
		constexpr static const char *METATABLE_TYPENAMEINT_FIELD = "InternalTypeName";
		constexpr static const char *METATABLE_TYPEIDINT_FIELD = "InternalTypeId";
		constexpr static const char *METATABLE_TYPEID_FIELD = "TypeId";
		
		LuaGlueTypeBase() { }
		
		virtual ~LuaGlueTypeBase() { }
	
		virtual const std::string &name() = 0;
		virtual LUA_UNSIGNED typeId() = 0;
		
		virtual bool glue(LuaGlueBase *g) = 0;
	
		virtual std::string toString() = 0;
		virtual lua_Integer toInteger() = 0;
		virtual lua_Number toNumber() = 0;
		
	protected:
		
		// sub types can implement these to add instance methods and properties
		virtual bool glue_type_methods(LuaGlueBase *) { return true; }
		virtual bool glue_type_properties(LuaGlueBase *) { return true; }
		virtual bool glue_instance_methods(LuaGlueBase *) { return true; }
		virtual bool glue_instance_properties(LuaGlueBase *) { return true; }
		virtual bool glue_meta_methods(LuaGlueBase *) { return true; }
		
		virtual LuaGlueBase *luaGlue() = 0;
		
		virtual int mm_index(lua_State *state) = 0;
		virtual int mm_newindex(lua_State *state) = 0;
		virtual int mm_gc(lua_State *state) = 0;
		virtual int mm_concat(lua_State *state) = 0;
		virtual int mm_call(lua_State *state) = 0;
		virtual int mm_eq(lua_State *state) = 0;
		virtual int lg_typeid(lua_State *state) = 0;
		
		// FIXME: this smells... how else to hide LuaGlueType from places that only really
		// need to see LuaGlueTypeBase???
		virtual void _impl_dtor(void *p) = 0;
		
		template <typename _Type>
		void impl_dtor(_Type *p)
		{
			this->_impl_dtor((void*)p);
		}
};

#endif /* LUAGLUE_TYPE_BASE_H */
