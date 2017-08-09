#ifndef IPV_SAMART_POINTER_H
#define IPV_SAMART_POINTER_H

#include <memory>

namespace MediaSDK
{

	template < typename T >
	using SmartPointer = std::shared_ptr < T > ;

	template < typename T >
	using UniquePointer = std::unique_ptr < T > ;

	template < typename T >
	using WeakPointer = std::weak_ptr < T > ;

} //namespace

#endif
