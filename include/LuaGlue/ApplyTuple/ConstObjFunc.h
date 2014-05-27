#ifndef LUAGLUE_APPLYTUPLE_CONSTOBJFUNC_H_GUARD
#define LUAGLUE_APPLYTUPLE_CONSTOBJFUNC_H_GUARD

/**
 * Object Function Tuple Argument Unpacking
 *
 * This recursive template unpacks the tuple parameters into
 * variadic template arguments until we reach the count of 0 where the function
 * is called with the correct parameters
 *
 * @tparam N Number of tuple arguments to unroll
 *
 * @ingroup g_util_tuple
 */
template < int N >
struct apply_obj_constfunc
{
  template < typename T, typename R, typename... ArgsF, typename... ArgsT, typename... Args >
  static R applyTuple(LuaGlueBase *g, lua_State *s, T* pObj,
                          R (T::*f)( ArgsF... ) const,
                          const std::tuple<ArgsT...> &t,
                          Args... args )
	{
		const static int argCount = sizeof...(ArgsT);
		typedef typename std::remove_reference<decltype(std::get<N-1>(t))>::type ltype_const;
		typedef typename std::remove_const<ltype_const>::type ltype;
		return apply_obj_func<N-1>::applyTuple(g, s, pObj, f, std::forward<decltype(t)>(t), stack<ltype>::get(g, s, -(argCount-N+1)), args... );
	}
};

//-----------------------------------------------------------------------------

/**
 * Object Function Tuple Argument Unpacking End Point
 *
 * This recursive template unpacks the tuple parameters into
 * variadic template arguments until we reach the count of 0 where the function
 * is called with the correct parameters
 *
 * @ingroup g_util_tuple
 */
template <>
struct apply_obj_constfunc<0>
{
  template < typename T, typename R, typename... ArgsF, typename... ArgsT, typename... Args >
  static R applyTuple(LuaGlueBase *, lua_State *, T* pObj,
                          R (T::*f)( ArgsF... ) const,
                          const std::tuple<ArgsT...> &/* t */,
                          Args... args )
	{
		return (pObj->*f)( args... );
	}
};

//-----------------------------------------------------------------------------

/**
 * Object Function Call Forwarding Using Tuple Pack Parameters
 */
// Actual apply function
template < typename T, typename R, typename... ArgsF, typename... ArgsT >
R applyTuple(LuaGlueBase *g, lua_State *s, T* pObj,
                 R (T::*f)( ArgsF... ) const,
                 const std::tuple<ArgsT...> &t )
{
	return apply_obj_constfunc<sizeof...(ArgsT)>::applyTuple(g, s, pObj, f, std::forward<decltype(t)>(t) );
}

#endif /* LUAGLUE_APPLYTUPLE_CONSTOBJFUNC_H_GUARD */
