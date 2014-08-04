#ifndef LUAGLUE_PROPERTY_H_GUARD
#define LUAGLUE_PROPERTY_H_GUARD

#include <lua.hpp>
#include <string>

#include "LuaGlue/LuaGlueTypeValue.h"
#include "LuaGlue/LuaGlueApplyTuple.h"
#include "LuaGlue/LuaGluePropertyBase.h"
#include "LuaGlue/LuaGlueBase.h"

template<class _Class>
class LuaGlueClass;

template<typename _Type, typename _Class>
class LuaGlueDirectProperty : public LuaGluePropertyBase
{
	public:
		typedef typename std::remove_const<typename std::remove_reference<_Type>::type>::type Type;
		typedef _Type _Class::*PropType;
		
		LuaGlueDirectProperty(LuaGlueClass<_Class> *luaClass, const std::string &name, PropType prop) : name_(name), prop_(prop), glueClass(luaClass)
		{
			
		}
		
		~LuaGlueDirectProperty() { }
		
		std::string name() { return name_; }
		
		bool glue(LuaGlueBase *luaGlue)
		{
			lua_pushlightuserdata(luaGlue->state(), this);
			lua_pushcclosure(luaGlue->state(), &lua_access, 1);
			lua_setfield(luaGlue->state(), -2, name_.c_str());
			return true;
		}
		
	private:
		std::string name_;
		Type _Class::*prop_;
		LuaGlueClass<_Class> *glueClass;
		
		int access(lua_State *state)
		{
			LG_Debug("access: %s::%s", glueClass->name().c_str(), name_.c_str());
			return accessImpl(state, std::is_class<typename std::remove_pointer<Type>::type>());
		}
		
		void getReturnVal(lua_State *state, _Class *obj, std::true_type)
		{
			LG_Debug("getReturnVal:pointer");
			Type val = (obj->*prop_);
			stack<Type>::put(glueClass->luaGlue(), state, val);
		}
		
		void getReturnVal(lua_State *state, _Class *obj, std::false_type)
		{
			LG_Debug("getReturnVal:!pointer");
			
			//Type val = (obj->*prop_);
			//stack<Type>::put(glueClass->luaGlue(), state, val);


			//Type *val = &(obj->*prop_);
			stack<Type>::put(glueClass->luaGlue(), state, &(obj->*prop_));
		}
		
		void getReturnVal(lua_State *state, _Class *obj)
		{
			//printf("getProp: %s::%s\n", glueClass->name().c_str(), name_.c_str());
			getReturnVal(state, obj, std::is_pointer<Type>());
		}
		
		void setProp(lua_State *state, _Class *obj, std::true_type)
		{
			LG_Debug("SET PTR!");
			(obj->*prop_) = stack<Type>::get(glueClass->luaGlue(), state, 3);
			LG_Debug("p=%p", (obj->*prop_));
		}
		
		void setProp(lua_State *state, _Class *obj, std::false_type)
		{
			LG_Debug("SET STATIC! %p", obj);
			(obj->*prop_) = stack<Type>::get(glueClass->luaGlue(), state, 3);
		}
		
		void setProp(lua_State *state, _Class *obj)
		{
			LG_Debug("set prop %s", name_.c_str());
			//printf("setProp: %s::%s\n", glueClass->name().c_str(), name_.c_str());
			setProp(state, obj, std::is_pointer<Type>());
		}
		
		int accessImpl(lua_State *state, std::true_type)
		{
			//printf("accessImpl: class\n");
			LG_Debug("accessImpl: class");
			
			int nargs = lua_gettop(state);

			_Class *ptr = nullptr;
			auto base = GetLuaUdata(state, 1, glueClass->name().c_str());
			
			// FIXME: I don't think this code is quite right
			//  the 'auto obj' object should really persist till after
			//  we're done with the internal ptr() object.
			if(base->isSharedPtr())
			{
				auto obj = CastLuaGlueTypeValueShared(_Class, base);
				ptr = obj->ptr();
			}
			else
			{
				auto obj = CastLuaGlueTypeValue(_Class, base);
				ptr = obj->ptr();
			}
			
			LG_Debug("accessImpl: ptr=%p", ptr);
			if(nargs == 2)
			{
				// get
				getReturnVal(state, ptr);
				return 1;
			}
			else if(nargs == 3)
			{
				// set
				setProp(state, ptr);
				return 0;
			}
			
			return 0;
		}
		
		int accessImpl(lua_State *state, std::false_type)
		{
			LG_Debug("accessImpl: basic type");
			
			int nargs = lua_gettop(state);
			
			_Class *ptr = nullptr;
			auto base = GetLuaUdata(state, 1, glueClass->name().c_str());
			if(base->isSharedPtr())
			{
				auto obj = CastLuaGlueTypeValueShared(_Class, base);
				ptr = obj->ptr();
			}
			else
			{
				auto obj = CastLuaGlueTypeValue(_Class, base);
				ptr = obj->ptr();
			}
			
			//printf("accessImpl: %p pod nargs:%i '%s'\n", obj, nargs, lua_tostring(state, -1));
			
			if(nargs == 2)
			{
				// get
				LG_Debug("type: %s/%s %p", typeid(decltype((ptr->*prop_))).name(), typeid(Type).name(), (ptr->*prop_));
				//Type val = (ptr->*prop_);
				stack<Type>::put(glueClass->luaGlue(), state, (ptr->*prop_));
				return 1;
			}
			else if(nargs == 3)
			{
				// set
				Type val = stack<Type>::get(glueClass->luaGlue(), state, 3);
				(ptr->*prop_) = val;
				//setProp(state, ptr);
				LG_Debug("set (%p) prop to %d", ptr, (ptr->*prop_));
				return 0;
			}
			
			return 0;
		}
		
		static int lua_access(lua_State *state)
		{
			auto pimp = (LuaGlueDirectProperty<_Type, _Class> *)lua_touserdata(state, lua_upvalueindex(1));
			return pimp->access(state);
		}
};

template<int _N, typename _Type, typename _Class>
class LuaGlueDirectPropertyArray : public LuaGluePropertyBase
{
	public:
		typedef typename std::remove_const<typename std::remove_reference<_Type>::type>::type Type;
		typedef _Type (_Class::*PropType)[_N];
		typedef _Type (ArrayType)[_N];
		
		char LUAGLUE_CLASS_NAME[256];
		
		LuaGlueDirectPropertyArray(LuaGlueClass<_Class> *luaClass, const std::string &name, PropType prop) : name_(name), prop_(prop), glueClass(luaClass)
		{
			snprintf(LUAGLUE_CLASS_NAME, 255, "LuaGlueStaticArrayType<%i, %s>", _N, typeid(_Type).name());
		}
		
		~LuaGlueDirectPropertyArray() { }
		
		std::string name() { return name_; }
		
		bool glue(LuaGlueBase *luaGlue)
		{
			lua_pushlightuserdata(luaGlue->state(), this);
			lua_pushcclosure(luaGlue->state(), &lua_access, 1);
			lua_setfield(luaGlue->state(), -2, name_.c_str());
			return true;
		}
		
	private:
		std::string name_;
		Type (_Class::*prop_)[_N];
		
		LuaGlueClass<_Class> *glueClass;
		
		int access(lua_State *state)
		{
			LG_Debug("access: %s::%s", glueClass->name().c_str(), name_.c_str());
			
			int nargs = lua_gettop(state);
			
			_Class *ptr = nullptr;
			auto base = GetLuaUdata(state, 1, glueClass->name().c_str());
			if(base->isSharedPtr())
			{
				auto obj = CastLuaGlueTypeValueShared(_Class, base);
				ptr = obj->ptr();
			}
			else
			{
				auto obj = CastLuaGlueTypeValue(_Class, base);
				ptr = obj->ptr();
			}
			
			if(nargs == 2)
			{
				// get
				LG_Debug("type: %s", typeid(ArrayType).name());
				stack<ArrayType>::put(glueClass->luaGlue(), state, (ptr->*prop_));
				return 1;
			}
			else if(nargs == 3)
			{
				// set
				stack<ArrayType>::getInPlace(glueClass->luaGlue(), state, 3, (ptr->*prop_));

				LG_Debug("set prop to <array>");
				return 0;
			}
			
			return 0;
		}
		
		
		static int lua_access(lua_State *state)
		{
			auto pimp = (LuaGlueDirectPropertyArray<_N, _Type, _Class> *)lua_touserdata(state, lua_upvalueindex(1));
			return pimp->access(state);
		}
};

template<typename _Type, typename _Class>
class LuaGlueDirectProperty<std::shared_ptr<_Type>, _Class> : public LuaGluePropertyBase
{
	public:
		typedef std::shared_ptr<typename std::remove_reference<_Type>::type> SharedType;
		
		typedef typename std::remove_const<typename std::remove_reference<_Type>::type>::type Type;
		typedef SharedType _Class::*PropType;
		
		LuaGlueDirectProperty(LuaGlueClass<_Class> *luaClass, const std::string &name, PropType prop) : name_(name), prop_(prop), glueClass(luaClass)
		{
			
		}
		
		~LuaGlueDirectProperty() { }
		
		std::string name() { return name_; }
		
		bool glue(LuaGlueBase *luaGlue)
		{
			lua_pushlightuserdata(luaGlue->state(), this);
			lua_pushcclosure(luaGlue->state(), &lua_access, 1);
			lua_setfield(luaGlue->state(), -2, name_.c_str());
			return true;
		}
		
	private:
		std::string name_;
		SharedType _Class::*prop_;
		LuaGlueClass<_Class> *glueClass;
		
		int access(lua_State *state)
		{
			LG_Debug("access: %s::%s", glueClass->name().c_str(), name_.c_str());
			return accessImpl(state, std::is_class<typename std::remove_pointer<Type>::type>());
		}
		
		void getReturnVal(lua_State *state, _Class *obj)
		{
			//printf("getProp: %s::%s\n", glueClass->name().c_str(), name_.c_str());
			SharedType val = (obj->*prop_);
			stack<SharedType>::put(glueClass->luaGlue(), state, val);
		}
		
		void setProp(lua_State *state, _Class *obj)
		{
			LG_Debug("set prop %s", name_.c_str());
			(obj->*prop_) = stack<SharedType>::get(glueClass->luaGlue(), state, 3);
		}
		
		int accessImpl(lua_State *state, std::true_type)
		{
			//printf("accessImpl: class\n");
			LG_Debug("accessImpl: class");
			
			int nargs = lua_gettop(state);

			_Class *ptr = nullptr;
			auto base = GetLuaUdata(state, 1, glueClass->name().c_str());
			
			// FIXME: I don't think this code is quite right
			//  the 'auto obj' object should really persist till after
			//  we're done with the internal ptr() object.
			if(base->isSharedPtr())
			{
				auto obj = CastLuaGlueTypeValueShared(_Class, base);
				ptr = obj->ptr();
			}
			else
			{
				auto obj = CastLuaGlueTypeValue(_Class, base);
				ptr = obj->ptr();
			}
			
			if(nargs == 2)
			{
				// get
				getReturnVal(state, ptr);
				return 1;
			}
			else if(nargs == 3)
			{
				// set
				setProp(state, ptr);
				return 0;
			}
			
			return 0;
		}
		
		int accessImpl(lua_State *state, std::false_type)
		{
			LG_Debug("accessImpl: basic type");
			
			int nargs = lua_gettop(state);
			
			_Class *ptr = nullptr;
			auto base = GetLuaUdata(state, 1, glueClass->name().c_str());
			if(base->isSharedPtr())
			{
				auto obj = CastLuaGlueTypeValueShared(_Class, base);
				ptr = obj->ptr();
			}
			else
			{
				auto obj = CastLuaGlueTypeValue(_Class, base);
				ptr = obj->ptr();
			}
			
			//printf("accessImpl: %p pod nargs:%i '%s'\n", obj, nargs, lua_tostring(state, -1));
			
			if(nargs == 2)
			{
				// get
				LG_Debug("type: %s", typeid(decltype((ptr->*prop_))).name());
				SharedType val = (ptr->*prop_);
				stack<SharedType>::put(glueClass->luaGlue(), state, val);
				return 1;
			}
			else if(nargs == 3)
			{
				// set
				SharedType val = stack<SharedType>::get(glueClass->luaGlue(), state, 3);
				(ptr->*prop_) = val;
				//setProp(state, ptr);
				LG_Debug("set prop to %d", (ptr->*prop_));
				return 0;
			}
			
			return 0;
		}
		
		static int lua_access(lua_State *state)
		{
			auto pimp = (LuaGlueDirectProperty<SharedType, _Class> *)lua_touserdata(state, lua_upvalueindex(1));
			return pimp->access(state);
		}
};

template<typename _Type, typename _Class>
class LuaGlueProperty : public LuaGluePropertyBase
{
	public:
		typedef typename std::remove_const<typename std::remove_reference<_Type>::type>::type Type;
		typedef _Type (_Class::*GetterType)();
		typedef _Type (_Class::*ConstGetterType)() const;
		typedef void (_Class::*SetterType)(_Type);
		
		LuaGlueProperty(LuaGlueClass<_Class> *luaClass, const std::string &name, GetterType getter, SetterType setter) : name_(name), getter(getter), setter(setter), glueClass(luaClass)
		{
			
		}
		
		LuaGlueProperty(LuaGlueClass<_Class> *luaClass, const std::string &name, ConstGetterType getter, SetterType setter) : name_(name), getter((GetterType)getter), setter(setter), glueClass(luaClass)
		{
			
		}
		
		~LuaGlueProperty() { }
		
		std::string name() { return name_; }
		
		bool glue(LuaGlueBase *luaGlue)
		{
			lua_pushlightuserdata(luaGlue->state(), this);
			lua_pushcclosure(luaGlue->state(), &lua_access, 1);
			lua_setfield(luaGlue->state(), -2, name_.c_str());
			return true;
		}
		
	private:
		std::string name_;
		GetterType getter;
		SetterType setter;
		LuaGlueClass<_Class> *glueClass;
		
		int access(lua_State *state)
		{
			int nargs = lua_gettop(state);

			_Class *ptr = nullptr;
			auto base = GetLuaUdata(state, 1, glueClass->name().c_str());
			if(base->isSharedPtr())
			{
				auto obj = CastLuaGlueTypeValueShared(_Class, base);
				ptr = obj->ptr();
			}
			else
			{
				auto obj = CastLuaGlueTypeValue(_Class, base);
				ptr = obj->ptr();
			}
			
			if(nargs == 2)
			{
				// get
				Type ret = (ptr->*getter)();
				stack<Type>::put(glueClass->luaGlue(), state, ret);
				return 1;
			}
			else if(nargs == 3)
			{
				// set
				Type arg = stack<Type>::get(glueClass->luaGlue(), state, 3);
				(ptr->*setter)(arg);
				return 0;
			}
			
			return 0;
		}
		
		static int lua_access(lua_State *state)
		{
			auto pimp = (LuaGlueProperty<_Type, _Class> *)lua_touserdata(state, lua_upvalueindex(1));
			return pimp->access(state);
		}
};

#endif /* LUAGLUE_PROPERTY_H_GUARD */
