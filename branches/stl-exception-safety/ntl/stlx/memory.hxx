/**\file*********************************************************************
 *                                                                     \brief
 *  N2588 20.6 Memory [memory]
 *
 *  ///\note C++0x move semantics not supported yet
 *
 ****************************************************************************
 */

#ifndef NTL__STLX_MEMORY
#define NTL__STLX_MEMORY

#include <stlx/esafety.hxx>
#include "cstddef.hxx"
#include "exception.hxx"
#include "iterator.hxx"
#include "type_traits.hxx"
#include "utility.hxx"
#include "new.hxx"
#include "stdlib.hxx"
#include "../linked_ptr.hxx"
#include "cassert.hxx"

namespace std {

/**\addtogroup  lib_utilities ********** [20] General utilities library *****
 *@{*/
/**\defgroup  lib_memory *************** [20.6] Memory **********************
 *@{*/

/// 20.6.1 Allocator argument tag [allocator.tag]
struct allocator_arg_t {/**/};
const allocator_arg_t allocator_arg;// = allocator_arg_t();

///\name 20.6.2 Allocator-related traits [allocator.traits]
template<class T, class Alloc> struct uses_allocator
: public integral_constant<
    bool, is_convertible<Alloc, typename T::allocator_type>::value
    > {};
template<class Alloc> struct is_scoped_allocator : false_type {};
template<class T> struct constructible_with_allocator_suffix : false_type {};
template<class T> struct constructible_with_allocator_prefix : false_type {};
///}


///\name 20.6.3 Allocator propagation traits [allocator.propagation]
template<class Alloc> struct allocator_propagate_never
: false_type {};
template<class Alloc> struct allocator_propagate_on_copy_construction
: false_type {};
template<class Alloc> struct allocator_propagate_on_move_assignment
: false_type {};
template<class Alloc> struct allocator_propagate_on_copy_assignment
: false_type {};
///}

/// 20.6.3 20.6.4 Allocator propogation map [allocator.propagation.map]
template<class Alloc> struct allocator_propagation_map
{
  static Alloc select_for_copy_construction(const Alloc& x)
  {
    return allocator_propagate_never<Alloc>::value ? Alloc() : x;
  }
#ifdef NTL__CXX
  static void move_assign(Alloc& to, Alloc&& from)
  {
    if ( allocator_propagate_on_move_assignment<Alloc>::value
      || allocator_propagate_on_copy_assignment<Alloc>::value )
      to = forward(from);
  }
#endif
  static void copy_assign(Alloc& to, Alloc& from)
  {
    if ( allocator_propagate_on_copy_assignment<Alloc>::value )
      to = from;
  }
  static void swap(Alloc& a, Alloc& b)
  {
//    anywhy UB
//    if ( allocator_propagate_on_move_assignment<Alloc>::value
//      || allocator_propagate_on_copy_assignment<Alloc>::value )
    if ( !(a == b ) )
      swap(a, b);
  }
};

/// 20.6.5 The default allocator [default.allocator]
template<class T> class allocator;

/// specialize for void
template<>
class allocator<void>
{
  public:
    typedef void        * pointer;
    typedef const void  * const_pointer;
    typedef void          value_type;
    template<class U> struct rebind { typedef allocator<U> other; };
};

template<class T>
class allocator
{
  ///////////////////////////////////////////////////////////////////////////
  public:

    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;
    typedef       T   * pointer;
    typedef const T   * const_pointer;
    typedef       T   & reference;
    typedef const T   & const_reference;
    typedef       T     value_type;
    template<class U> struct rebind { typedef allocator<U> other; };

    allocator() __ntl_nothrow {}
    allocator(const allocator &) __ntl_nothrow {}
    template<class U> allocator(const allocator<U> &) __ntl_nothrow {}
    ~allocator() __ntl_nothrow {}

    ///\name  20.6.5.1 allocator members [allocator.members]

#ifndef NTL_STLEXT
    pointer address(typename add_reference<typename remove_const<typename remove_reference<reference>::type>::type>::type x) const
    {
      return &x;
    }
    const_pointer address(const_reference x) const { return &x; }
#else
    template<typename value_type>
    value_type * address(value_type& x) const
    {
      const_pointer check_type = &x; (check_type);
      return &x;
    }
#endif

    __declspec(noalias) __declspec(restrict)
    __forceinline
    pointer allocate(size_type n, allocator<void>::const_pointer hint = 0)
      __ntl_throws(bad_alloc)
    {
      (hint);
      const pointer p = reinterpret_cast<T*>(::operator new(sizeof(T) * n));
      __assume(p);
      return p;
    }

    __declspec(noalias)
    __forceinline
    void deallocate(pointer p, size_type /* n */)
    {
      ::operator delete(const_cast<remove_const<T>::type*>(p));//(address(*p));
    }

    size_type max_size() const __ntl_nothrow { return size_t(-1) / sizeof(T); }

    __forceinline
    void construct(pointer p, const T & val)
    {
      __assume(p);
      ///\todo ::new((void *)p ) T(std::forward<U>(val))
      ::new((void *)p) T(val);
    }

//    __forceinline
    void destroy(const pointer p)
    {
      p->T::/*< workaround MSVC's weird `scalar deleting destructor'*/ ~T();
    }

    ///\}

};//class allocator


template<class T> struct allocator_propagate_never<allocator<T> >
: true_type {};


///\name  20.6.5.2 allocator globals [allocator.globals]

template<class T, class U>
inline
bool
  operator==(const allocator<T>&, const allocator<U>&) __ntl_nothrow
{
  return true;
}

template<class T, class U>
inline
bool
  operator!=(const allocator<T>&, const allocator<U>&) __ntl_nothrow
{
  return false;
}

///\}

///\ 20.6.6 Scoped allocator adaptor [allocator.adaptor]
template<typename OuterA, typename InnerA = void>
class scoped_allocator_adaptor;

/// 20.6.7 Raw storage iterator [storage.iterator]
template<class OutputIterator, class T>
class raw_storage_iterator
: public iterator<output_iterator_tag, void, void, void, void>
{
    typedef raw_storage_iterator<OutputIterator, T> this_type;

  public:
    explicit raw_storage_iterator(OutputIterator x) : i(x) {}
    this_type& operator* ()     { return *this; }
    this_type& operator++()     { ++i; return *this; }
    this_type  operator++(int)  { this_type old( i ); ++i; return old; }
    this_type& operator= (const T& element)
    {
      allocator<T>().construct(&*i, element);
      return *this;
    }

  private:
    OutputIterator  i;
};


///\name  20.6.8 Temporary buffers [temporary.buffer]

template <class T>
__forceinline
pair<T*,ptrdiff_t>
  get_temporary_buffer(ptrdiff_t n)
{
  T* p = allocator<T>().allocate(static_cast<allocator<T>::size_type>(n));
  return make_pair(p, p ? n : 0);
}

template <class T>
__forceinline
void
  return_temporary_buffer(T* p)
{
  // 20.6.1.1/7  n shall equal the value passed as the first argument
  //             to the invocation of allocate which returned p.
  // but allocator::deallocate() does not use n
  allocator<T>::size_type const n = 0;
  allocator<T>().deallocate(p, n);
}

/// 20.6.9 construct_element [construct.element]
#ifdef NTL__CXX
template <class Alloc, class T, class... Args>
void construct_element(Alloc& alloc, T& r, Args&&... args);
#endif


///\name 20.7.10 Specialized algorithms [specialized.algorithms]
/// 1 All the iterators that are used as formal template parameters in the following
///   algorithms are required to have their operator* return an object for which
///   operator& is defined and returns a pointer to T. In the algorithm uninitialized_
///   copy, the formal template parameter InputIterator is required to satisfy
///   the requirements of an input iterator (24.1.1).
///   In all of the following algorithms, the formal template parameter ForwardIterator
///   is required to satisfy the requirements of a forward iterator (24.1.3)
///   and also to satisfy the requirements of a mutable iterator (24.1), and is
///   required to have the property that no exceptions are thrown from increment,
///   assignment, comparison, or dereference of valid iterators.
///   In the following algorithms, if an exception is thrown there are no effects.

///\name  20.7.10.1 uninitialized_copy [uninitialized.copy]

template <class InputIterator, class ForwardIterator>
__forceinline
ForwardIterator
  uninitialized_copy(InputIterator    first,
                     InputIterator    last,
                     ForwardIterator  result)
{
  detail::guarded_range_constructor<ForwardIterator> guarded_ctor(result);
  for ( ; first != last; ++result, ++first )
  {
    guarded_ctor(*first);
  }
  return guarded_ctor.dismiss();
  ///@todo separate function for scalar types ?
}

template <class InputIterator, class Size, class ForwardIterator>
__forceinline
ForwardIterator
  uninitialized_copy_n(InputIterator first, Size n, ForwardIterator result)
{
  detail::guarded_range_constructor<ForwardIterator> guarded_ctor(result);
  for ( ; n; ++result, ++first )
  {
    guarded_ctor(*first);
  }
  return guarded_ctor.dismiss();
  ///@todo separate function for scalar types ?
}

///@}

/// 20.7.10.2 uninitialized_fill [uninitialized.fill]
template <class ForwardIterator, class T>
__forceinline
void
  uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& x)
{
  detail::guarded_range_constructor<ForwardIterator> guarded_ctor(first);
  for ( ; first != last; ++first )
  {
    guarded_ctor(x);
  }
  guarded_ctor.dismiss();
  ///@todo separate function for scalar types ?
}

/// 20.7.10.3 uninitialized_fill_n [uninitialized.fill.n]
template <class ForwardIterator, class Size, class T>
__forceinline
void
  uninitialized_fill_n(ForwardIterator first, Size n, const T& x)
{
  detail::guarded_range_constructor<ForwardIterator> guarded_ctor(first);
  for ( ; n; --n)
  {
    guarded_ctor(x);
  }
  guarded_ctor.dismiss();
  ///@todo separate function for scalar types ?
}


/// 20.6.11 Class template unique_ptr [unique.ptr]

///\name 20.6.11.1 Default deleters [unique.ptr.dltr]

/// 20.6.11.1.1 default_delete [unique.ptr.dltr.dflt]
template <class T> struct default_delete
{
    default_delete() {}
    template <class U> default_delete(const default_delete<U>&) {}
    void operator()(T* ptr) const { ::delete ptr; }
  private:
    // forbid incompatible ::delete[]
    template <class U> default_delete(const default_delete<U[]>&);
    template <class U, size_t S> default_delete(const default_delete<U[S]>&);
};

/// 20.6.11.1.2 default_delete<T[]> [unique.ptr.dltr.dflt1]
template <class T> struct default_delete<T[]>
{
  void operator()(T* ptr) const { ::delete[] ptr; }
};

/// 20.6.11.1.3 default_delete<T[N]> [unique.ptr.dltr.dflt2]
template <class T, size_t N> struct default_delete<T[N]>
{
  void operator()(T* ptr, size_t) const { ::delete[] ptr; }
};

///\}

template <class T, class D = default_delete<T> >
class unique_ptr;

/// 20.6.11.2 unique_ptr for single objects [unique.ptr.single]
///\warning not Standard as no && support yet
template <class T>
class unique_ptr<T, default_delete<T> >
{
    typedef typename const unique_ptr::T** unspecified_pointer_type;
    typedef typename const unique_ptr::T*  unspecified_bool_type;

  ///////////////////////////////////////////////////////////////////////////
  public:

    typedef T element_type;
    typedef default_delete<T> deleter_type;

    ///\name 20.6.11.2.1 unique_ptr constructors [unique.ptr.single.ctor]
    unique_ptr() __ntl_nothrow : ptr(0) {}
    explicit unique_ptr(T* p) __ntl_nothrow : ptr(p) {}
    unique_ptr(T* p, const deleter_type &) __ntl_nothrow : ptr(p) {}
    unique_ptr(const unique_ptr& u) __ntl_nothrow : ptr(u.get())
    {
      u.release(); // it's here to help MSVC optimizing container operations
    }

    template <class U/*, class E*/>
    unique_ptr(const unique_ptr<U, deleter_type>& u) __ntl_nothrow : ptr(u.get())
    {
      u.release();
    }

    ///\name 20.6.11.2.2 unique_ptr destructor [unique.ptr.single.dtor]
    ~unique_ptr() __ntl_nothrow { if ( get() ) get_deleter()(get()); }

    ///\name 20.6.11.2.3 unique_ptr assignment [unique.ptr.single.asgn]
    __forceinline
    const unique_ptr& operator=(const unique_ptr& u) __ntl_nothrow
    {
      reset(u.release());
      return *this;
    }

    template <class U/*, class E*/>
    __forceinline
    const unique_ptr& operator=(const unique_ptr<U, deleter_type>& u) __ntl_nothrow
    {
      reset(u.release());
      return *this;
    }

    const unique_ptr& operator=(unspecified_pointer_type *)
    {
      reset();
      return *this;
    }

    ///\name 20.6.11.2.4 unique_ptr observers [unique.ptr.single.observers]
    T& operator*() const __ntl_nothrow { return *get(); }
    T* operator->() const __ntl_nothrow { return get(); }
    T* get() const __ntl_nothrow { return ptr; }

    // local statics produce code bloat, shoud we replace the UD with a global static?
    deleter_type& get_deleter() __ntl_nothrow { return *(deleter_type*)0; }
    const deleter_type& get_deleter() const __ntl_nothrow { return *(deleter_type*)0; }

    operator unspecified_bool_type() const __ntl_nothrow { return ptr; }

    ///\name 20.6.11.2.5 unique_ptr modifiers [unique.ptr.single.modifiers]
    T* release() const { T * const tmp = get(); set(0); return tmp; }

    __forceinline
    void reset(T* p = 0) const __ntl_nothrow
    {
      if ( get() && get() != p ) get_deleter()(get());
      set(p);
    }

    void swap(const unique_ptr& u) const __ntl_nothrow { std::swap(ptr, u.ptr); }

    ///\}

  ///////////////////////////////////////////////////////////////////////////
  private:

    mutable element_type *  ptr;
    void set(T * p) const { ptr = p; }

};//template class unique_ptr

/// 20.6.11.3 unique_ptr for array objects with a runtime length [unique.ptr.runtime]
template <class T>
class unique_ptr<T[], default_delete<T[]> >
{
    typedef typename const unique_ptr::T** unspecified_pointer_type;
    typedef typename const unique_ptr::T*  unspecified_bool_type;

  ///////////////////////////////////////////////////////////////////////////
  public:

    typedef T element_type;
    typedef default_delete<T[]> deleter_type;

    ///\name 20.6.11.3.1 unique_ptr constructors [unique.ptr.runtime.ctor]
    unique_ptr() __ntl_nothrow : ptr(0) {}
    explicit unique_ptr(T* p) __ntl_nothrow : ptr(p) {}
    unique_ptr(T* p, const deleter_type &) __ntl_nothrow : ptr(p) {}
    unique_ptr(const unique_ptr& u) __ntl_nothrow : ptr(u.get())
    {
      u.release();
    }

    ///\name 20.6.11.2.2 unique_ptr destructor [unique.ptr.single.dtor]
    ~unique_ptr() __ntl_nothrow { if ( get() ) get_deleter()(get()); }

    ///\name 20.6.11.2.3 unique_ptr assignment [unique.ptr.single.asgn]
    __forceinline
    const unique_ptr& operator=(const unique_ptr& u) __ntl_nothrow
    {
      reset(u.release());
      return *this;
    }

    const unique_ptr& operator=(unspecified_pointer_type *)
    {
      reset();
      return *this;
    }

    ///\name 20.6.11.3.2 unique_ptr observers [unique.ptr.runtime.observers]
    T& operator[](size_t i) const __ntl_nothrow { return get()[i]; }
    T* get() const __ntl_nothrow { return ptr; }

    deleter_type& get_deleter() __ntl_nothrow
    {
      static deleter_type deleter;
      return deleter;
    }

    const deleter_type& get_deleter() const __ntl_nothrow
    {
      static const deleter_type deleter;
      return deleter;
    }

    operator unspecified_bool_type() const __ntl_nothrow { return ptr; }

    ///\name 20.6.11.2.5 unique_ptr modifiers [unique.ptr.single.modifiers]
    T* release() const { T * const tmp = get(); set(0); return tmp; }

    __forceinline
    void reset(T* p = 0) const __ntl_nothrow
    {
      if ( get() && get() != p ) get_deleter()(get());
      set(p);
    }

    void swap(const unique_ptr& u) const __ntl_nothrow { std::swap(ptr, u.ptr); }

    ///\}

  ///////////////////////////////////////////////////////////////////////////
  private:

    mutable element_type *  ptr;
    void set(T * p) const { ptr = p; }

    template<class Other> unique_ptr(Other*);
    template<class Other> void reset(Other*) const;

};//template class unique_ptr

/// 20.6.11.4 unique_ptr for array objects with a compile time length [unique.ptr.compiletime]
template <class T, size_t N>
class unique_ptr<T[N], default_delete<T[N]> >
{
    typedef typename const unique_ptr::T** unspecified_pointer_type;
    typedef typename const unique_ptr::T*  unspecified_bool_type;

  ///////////////////////////////////////////////////////////////////////////
  public:

    typedef T element_type;
    typedef default_delete<T[N]> deleter_type;

    static const size_t size = N;

    ///\name 20.6.11.3.1 unique_ptr constructors [unique.ptr.runtime.ctor]
    unique_ptr() __ntl_nothrow : ptr(0) {}
    explicit unique_ptr(T* p) __ntl_nothrow : ptr(p) {}
    unique_ptr(T* p, const deleter_type &) __ntl_nothrow : ptr(p) {}
    unique_ptr(const unique_ptr& u) __ntl_nothrow : ptr(u.get())
    {
      u.release();
    }

    ///\name 20.6.11.4.1 unique_ptr destructor [unique.ptr.compiletime.dtor]
    ~unique_ptr() __ntl_nothrow { if ( get() ) get_deleter()(get(), N); }

    ///\name 20.6.11.2.3 unique_ptr assignment [unique.ptr.single.asgn]
    __forceinline
    const unique_ptr& operator=(const unique_ptr& u) __ntl_nothrow
    {
      reset(u.release());
      return *this;
    }

    const unique_ptr& operator=(unspecified_pointer_type *)
    {
      reset();
      return *this;
    }

    ///\name 20.6.11.4.1 unique_ptr destructor [unique.ptr.compiletime.dtor]
    T& operator[](size_t i) const __ntl_nothrow { return get()[i]; }
    T* get() const __ntl_nothrow { return ptr; }

    deleter_type& get_deleter() __ntl_nothrow
    {
      static deleter_type deleter;
      return deleter;
    }

    const deleter_type& get_deleter() const __ntl_nothrow
    {
      static const deleter_type deleter;
      return deleter;
    }

    operator unspecified_bool_type() const __ntl_nothrow { return ptr; }

    ///\name 20.6.11.2.5 unique_ptr modifiers [unique.ptr.single.modifiers]
    T* release() const { T * const tmp = get(); set(0); return tmp; }

    __forceinline
    void reset(T* p = 0) const __ntl_nothrow
    {
      if ( get() && get() != p ) get_deleter()(get());
      set(p);
    }

    void swap(const unique_ptr& u) const __ntl_nothrow { std::swap(ptr, u.ptr); }

    ///\}

  ///////////////////////////////////////////////////////////////////////////
  private:

    mutable element_type *  ptr;
    void set(T * p) const { ptr = p; }

    template<class Other> unique_ptr(Other*);
    template<class Other> void reset(Other*) const;

};//template class unique_ptr<T[N], default_delete<T[N]> >

///\name 20.6.11.5 unique_ptr specialized algorithms [unique.ptr.special]
template <class T, class D> void swap(unique_ptr<T, D>& x, unique_ptr<T, D>& y)
{
  x.swap(y);
}
#ifdef NTL__CXX
template <class T, class D> void swap(unique_ptr<T, D>&& x, unique_ptr<T, D>& y)
{
  x.swap(y);
}
template <class T, class D> void swap(unique_ptr<T, D>& x, unique_ptr<T, D>&& y)
{
  x.swap(y);
}
#endif

template <class T1, class D1, class T2, class D2>
bool operator==(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y)
{
  return x.get() == y.get();
}

template <class T1, class D1, class T2, class D2>
bool operator!=(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y)
{
  return x.get() != y.get();
}

template <class T1, class D1, class T2, class D2>
bool operator<(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y)
{
  return x.get() < y.get();
}

template <class T1, class D1, class T2, class D2>
bool operator<=(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y)
{
  return x.get() <= y.get();
}

template <class T1, class D1, class T2, class D2>
bool operator>(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y)
{
  return x.get() > y.get();
}

template <class T1, class D1, class T2, class D2>
bool operator>=(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y)
{
  return x.get() >= y.get();
}

///\}

///\name  20.6.12 Smart pointers [util.smartptr]

template<class X> class auto_ptr;

/// 20.6.12.1 Class bad_weak_ptr [util.smartptr.weakptr]
class bad_weak_ptr : public std::exception
{
  public:
    virtual const char* what() const __ntl_nothrow { return "bad_weak_ptr"; }
};

template<class T> class weak_ptr;

/// 20.6.12.2 Class template shared_ptr [util.smartptr.shared]
///
/// 1 The shared_ptr class template stores a pointer, usually obtained via new.
///   shared_ptr implements semantics of shared ownership; the last remaining
///   owner of the pointer is responsible for destroying the object, or
///   otherwise releasing the resources associated with the stored pointer.
///\note no constructor throws bad_alloc, since this needs no heap memory
template<class T>
class shared_ptr : ntl::linked_ptr<T>
{
    typedef ntl::linked_ptr<T> base_type;

  ///////////////////////////////////////////////////////////////////////////
  public:

    typedef T element_type;

    ///\name  20.6.12.2.1 shared_ptr constructors [util.smartptr.shared.const]

    /// 1 Effects: Constructs an empty shared_ptr object.
    /// 2 Postconditions: use_count() == 0 && get() == 0.
    /// 3 Throws: nothing.
    shared_ptr() __ntl_nothrow : base_type() {/**/}

    /// 4 Requires: p shall be convertible to T*. Y shall be a complete type.
    ///   The expression delete p shall be well-formed,
    ///   shall have well defined behavior, and shall not throw exceptions.
    /// 5 Effects: Constructs a shared_ptr object that owns the pointer p.
    /// 6 Postconditions: use_count() == 1 && get() == p
    template<class Y> explicit shared_ptr(Y* p) : base_type(p) {/**/}

    /// Well, should not D & A be moved to the class template? ...
    template<class Y, class D> shared_ptr(Y* p, D d);
    /// this needs no allolcator at all
    template<class Y, class D, class A> shared_ptr(Y* p, D d, A a);

    /// 14 Effects: Constructs a shared_ptr instance that stores p and shares
    ///    ownership with r.
    /// 15 Postconditions: get() == p && use_count() == r.use_count()
    /// 16 Throws: nothing.
    template<class Y> shared_ptr(const shared_ptr<Y>& r, T *p);

    /// 20 Effects: If r is empty, constructs an empty shared_ptr object;
    ///   otherwise, constructs a shared_ptr object that shares ownership with r.
    /// 21 Postconditions: get() == r.get() && use_count() == r.use_count().
    /// 22 Throws: nothing.
    shared_ptr(const shared_ptr& r) : base_type(r)
    {
      set(r.get());
    }

    /// see above
    ///\todo:
    /// 19 Requires: The constructor shall not participate in
    ///    the overload resolution unless Y* is implicitly convertible to T*.
    template<class Y> shared_ptr(const shared_ptr<Y>& r) : base_type(r)
    {
      set(r.get());
    }

#ifdef NTL__CXX
    /// 23 Requires: For the second constructor Y* shall be convertible to T*.
    /// 24 Effects: Move-constructs a shared_ptr instance from r.
    /// 25 Postconditions: *this shall contain the old value of r. r shall be empty.
    /// 26 Throws: nothing.
    shared_ptr(shared_ptr&& r);
    template<class Y> shared_ptr(shared_ptr<Y>&& r);
#endif

    /// 27 Requires: Y* shall be convertible to T*.
    /// 28 Effects: Constructs a shared_ptr object that shares ownership with r
    ///    and stores a copy of the pointer stored in r.
    /// 29 Postconditions: use_count() == r.use_count().
    /// 30 Throws: bad_weak_ptr when r.expired().
    /// 31 Exception safety: If an exception is thrown, the constructor has no effect.
    template<class Y> explicit shared_ptr(const weak_ptr<Y>& r) : base_type(r)
    {
      if ( r.get() ) set(r.get());
      else throw bad_alloc();
    }

#ifdef NTL__CXX
    /// 32 Requires: r.release() shall be convertible to T*. Y shall be a complete
    ///    type. The expression delete r.release() shall be well-formed,
    ///    shall have well defined behavior, and shall not throw exceptions.
    /// 33 Effects: Constructs a shared_ptr object that stores and owns r.release().
    /// 34 Postconditions: use_count() == 1 && r.get() == 0.
    /// 35 Throws: bad_alloc, or an implementation-defined exception when
    ///    a resource other than memory could not be obtained.
    /// 36 Exception safety: If an exception is thrown, the constructor has no effect.
    template<class Y> explicit shared_ptr(auto_ptr<Y>&& r);
#else
    template<class Y> explicit shared_ptr(const auto_ptr<Y>& r)
    : base_type(r.release()) {/**/}
#endif

#ifdef NTL__CXX
    /// 37 Effects: Equivalent to shared_ptr(r.release(), r.get_deleter())
    ///    when D is not a reference type,
    ///    otherwise shared_ptr(r.release(), ref(r.get_deleter())).
    /// 38 Exception safety: If an exception is thrown, the constructor has no effect.
    template <class Y, class D> explicit shared_ptr(const unique_ptr<Y, D>& r) = delete;
    template <class Y, class D> explicit shared_ptr(unique_ptr<Y, D>&& r);
#else
    template <class Y, class D> explicit shared_ptr(const unique_ptr<Y, D>& r)
    : base_type(r.release()) {/**/}
#endif

    ///\name  20.6.12.2.2 shared_ptr destructor [util.smartptr.shared.dest]
    ///
    /// 1  If *this is empty or shares ownership with another shared_ptr instance
    ///   (use_count() > 1), there are no side effects.
    ///\todo: (?)
    ///    Otherwise, if *this owns a pointer p and a deleter d, d(p) is called.
    ///    Otherwise, *this owns a pointer p, and delete p is called.
    /// 2 Throws: nothing.
    /// 3 [ Note: Since the destruction of *this decreases the number of instances
    ///   that share ownership with *this by one, after *this has been destroyed
    ///   all shared_ptr instances that shared ownership with *this will report
    ///   a use_count() that is one less than its previous value. end note ]
    __forceinline
    ~shared_ptr()
    {
      if ( unique() )
        free();
    }

    ///\name  20.6.12.2.3 shared_ptr assignment [util.smartptr.shared.assign]

#ifdef NTL__CXX
    shared_ptr& operator=(shared_ptr&& r);
    template<class Y> shared_ptr& operator=(shared_ptr<Y>&& r);
    template<class Y> shared_ptr& operator=(auto_ptr<Y>&& r);
    template <class Y, class D> shared_ptr& operator=(const unique_ptr<Y, D>& r) = delete;
    template <class Y, class D> shared_ptr& operator=(unique_ptr<Y, D>&& r);
#else
    shared_ptr& operator=(const shared_ptr& r) { r.swap(this); }
    template<class Y> shared_ptr& operator=(const shared_ptr<Y>& r) { r.swap(this); }
#endif

    ///\name  20.6.12.2.4 shared_ptr modifiers [util.smartptr.shared.mod]

    /// 1 Effects: Exchanges the contents of *this and r.
    /// 2 Throws: nothing.
    void swap(shared_ptr& r)
    {
      base_type::swap(r);
    }

    /// 3 Effects: Equivalent to shared_ptr().swap(*this).
    __forceinline
    void reset() { shared_ptr().swap(*this); }
    template<class Y> void reset(Y* p) { shared_ptr(p).swap(*this); }
    template<class Y, class D> void reset(Y* p, D d);

    ///\name  20.6.6.2.5 shared_ptr observers [util.smartptr.shared.obs]

    T & operator* ()  const __ntl_nothrow { return *base_type::get(); }
    T * operator->()  const __ntl_nothrow { return base_type::get(); }
    T * get()         const __ntl_nothrow { return base_type::get(); }

    //long use_count() const __ntl_nothrow { return base_type::use_count(); }
    using base_type::use_count;

    //bool unique()    const __ntl_nothrow { return base_type::unique(); }
    using base_type::unique;

    //operator base_type::unspecified_bool_type() const __ntl_nothrow
    //{
    //  return base_type::operator base_type::unspecified_bool_type();
    //}
    using base_type::operator base_type::unspecified_bool_type;

    ///@}

  ///////////////////////////////////////////////////////////////////////////
  private:

    void set(T * p) { base_type::set(p); }
    void free() { delete base_type::get(); }

};

///\name  20.6.12.2.6 shared_ptr creation [util.smartptr.shared.create]
#ifdef NTL__CXX
/// 1 Requires: The expression new (pv) T(std::forward<Args>(args)...), where
///   pv has type void* and points to storage suitable to hold an object of type
///   T, shall be well-formed. A shall be an allocator (20.1.2).
///   The copy constructor and destructor of A shall not throw exceptions.
/// 2 Effects: Allocates memory suitable for an object of type T and constructs
///   an object in that memory via the placement new expression new (pv) T()
///   or new (pv) T(std::forward<Args>(args)...). The template allocate_shared
///   uses a copy of a to allocate memory.
///   If an exception is thrown, the functions have no effect.
/// 3 Returns: A shared_ptr instance that stores and owns the address of
///   the newly constructed object of type T.
/// 4 Postconditions: get() != 0 && use_count() == 1
/// 5 Throws: bad_alloc, or an exception thrown from A::allocate
///   or from the constructor of T.
template<class T, class... Args> shared_ptr<T> make_shared(Args&&... args);
template<class T, class A, class... Args>shared_ptr<T> allocate_shared(const A& a, Args&&... args);
#else

#endif

///\name  20.6.12.2.7 shared_ptr comparison [util.smartptr.shared.cmp]

template<class T, class U>
inline
bool
  operator==(shared_ptr<T> const& a, shared_ptr<U> const& b)
{
  return a.get() == b.get();
}

template<class T, class U>
inline
bool
  operator!=(shared_ptr<T> const& a, shared_ptr<U> const& b)
{
  return a.get() != b.get().
}

/// 5 Returns: an unspecified value such that
///    operator< is a strict weak ordering as described in 25.3;
///    under the equivalence relation defined by operator<, !(a < b) && !(b < a),
///     two shared_ptr instances are equivalent if and only if they share
///     ownership or are both empty.
/// 6 Throws: nothing.
/// 7 [ Note: Defining a comparison operator allows shared_ptr objects
///   to be used as keys in associative containers. end note ]
template<class T, class U>
inline
bool
  operator<(shared_ptr<T> const& a, shared_ptr<U> const& b)
{
  return reinterpret_cast<uintptr_t>(a.get())
       < reinterpret_cast<uintptr_t>(b.get());
}

///\name  20.6.12.2.8 shared_ptr I/O [util.smartptr.shared.io]
template<class E, class T, class Y>
inline
basic_ostream<E, T>&
  operator<< (basic_ostream<E, T>& os, shared_ptr<Y> const& p)
{
  os << p.get();
}

///\name  20.6.12.2.9 shared_ptr specialized algorithms [util.smartptr.shared.spec]
template<class T>
inline
void
  swap(shared_ptr<T>& a, shared_ptr<T>& b) __ntl_nothrow
{
  a.swap(b);
}

///\name  20.6.12.2.10 shared_ptr casts [util.smartptr.shared.cast]

template<class T, class U>
inline
shared_ptr<T>
  static_pointer_cast(shared_ptr<U> const& r);

template<class T, class U>
inline
shared_ptr<T>
  dynamic_pointer_cast(shared_ptr<U> const& r);

template<class T, class U>
inline
shared_ptr<T>
  const_pointer_cast(shared_ptr<U> const& r);

///@}

/// 20.6.12.2.11 get_deleter [util.smartptr.getdeleter]
template<class D, class T> D* get_deleter(shared_ptr<T> const& p);


/// 20.6.6.3 Class template weak_ptr [util.smartptr.weak]
template<class T>
class weak_ptr
{
  ///////////////////////////////////////////////////////////////////////////
  public:

    typedef T element_type;

    ///\name  20.6.6.3.1 weak_ptr constructors [util.smartptr.weak.const]

    weak_ptr();
    template<class Y> weak_ptr(shared_ptr<Y> const& r);
    weak_ptr(weak_ptr const& r);
    template<class Y> weak_ptr(weak_ptr<Y> const& r);

    ///\name  20.6.6.3.2 weak_ptr destructor [util.smartptr.weak.dest]
    ~weak_ptr();

    ///\name  20.6.6.3.3 weak_ptr assignment [util.smartptr.weak.assign]

    weak_ptr& operator=(weak_ptr const& r);
    template<class Y> weak_ptr& operator=(weak_ptr<Y> const& r);
    template<class Y> weak_ptr& operator=(shared_ptr<Y> const& r);

    ///\name  20.6.6.3.4 weak_ptr modifiers [util.smartptr.weak.mod]

    void swap(weak_ptr& r);
    void reset();

    ///\name  20.6.6.3.5 weak_ptr observers [util.smartptr.weak.obs]

    long use_count() const;
    bool expired() const;
    shared_ptr<T> lock() const;

    ///@}

  ///////////////////////////////////////////////////////////////////////////
  private:

};

///\name  20.6.6.3.6 weak_ptr comparison [util.smartptr.weak.cmp]
template<class T, class U>
bool
  operator<(weak_ptr<T> const& a, weak_ptr<U> const& b);

///\name  20.6.6.3.7 weak_ptr specialized algorithms [util.smartptr.weak.spec]
template<class T>
void
  swap(weak_ptr<T>& a, weak_ptr<T>& b)
{
  a.swap(b);
}

///@}

/// 20.6.6.4 Class template enable_shared_from_this [util.smartptr.enab]
template<class T> class enable_shared_from_this;

/// 20.6.13 Align [ptr.align]
inline void * align(size_t alignment, size_t size, void* &ptr, size_t& space)
{
  uintptr_t & uptr = reinterpret_cast<uintptr_t&>(ptr);
  uintptr_t const aligned_ptr = ntl::align_up(uptr, alignment);
  if ( aligned_ptr + size <= uptr + space )
  {
    space -= aligned_ptr - uptr;
    return reinterpret_cast<void*>(uptr = aligned_ptr);
  }
  return 0;
}

///\name  D.9 auto_ptr [depr.auto.ptr]

template<class Y> struct auto_ptr_ref;

/// D.9.1 Class template auto_ptr [auto.ptr]
template<class X>
class auto_ptr
{
  ///////////////////////////////////////////////////////////////////////////
  public:

    typedef X element_type;

    ///\name  D.9.1.1 auto_ptr constructors [auto.ptr.cons]

    explicit auto_ptr(X * p = 0)  __ntl_nothrow : ptr(p) {}

    auto_ptr(auto_ptr & a)        __ntl_nothrow : ptr(a.release()) {}

    template<class Y>
    auto_ptr(auto_ptr<Y> & a)     __ntl_nothrow : ptr(a.release()) {}

    __forceinline
    auto_ptr & operator=(auto_ptr & a) __ntl_nothrow
    {
      reset(a.release());
      return *this;
    }

    template<class Y>
    auto_ptr & operator=(auto_ptr<Y> & a) __ntl_nothrow
    {
      reset(a.release());
      return *this;
    }

    __forceinline
    ~auto_ptr() __ntl_nothrow { if ( get() ) delete get(); }

    ///\name  D.9.1.2 auto_ptr members [auto.ptr.members]

    X & operator* ()  const __ntl_nothrow { return *get(); }
    X * operator->()  const __ntl_nothrow { return get(); }
    X * get()         const __ntl_nothrow { return ptr; }
    X * release()           __ntl_nothrow { X * tmp = get(); set(0); return tmp; }

    __forceinline
    void reset(X * p = 0)   __ntl_nothrow
    {
      if ( get() && get() != p ) delete get();
      set(p);
    }

    ///\name  D.9.1.3 auto_ptr conversions [auto.ptr.conv]

    auto_ptr(auto_ptr_ref<X> r) __ntl_nothrow : ptr(r.ptr.release()) {}

    __forceinline
    auto_ptr & operator=(auto_ptr_ref<X> r) __ntl_nothrow { return *this = r.ptr; }

    template<class Y>
    operator auto_ptr_ref<Y>()  __ntl_nothrow { return auto_ptr_ref<Y>(*this); }

    template<class Y>
    operator auto_ptr<Y>()      __ntl_nothrow { return auto_ptr<Y>(release()); }

    ///@}

  ///////////////////////////////////////////////////////////////////////////
  private:

    X * ptr;
    void set(X * p) { ptr = p; }
};

template<class Y>
struct auto_ptr_ref
{
    auto_ptr_ref(auto_ptr<Y> & a) __ntl_nothrow : ptr(a) {}
    auto_ptr<Y> & ptr;
  private:
    auto_ptr_ref<Y> & operator=(const auto_ptr_ref<Y> &);
};


/**@} lib_memory */
/**@} lib_utilities */

}//namespace std

#endif//#ifndef NTL__STLX_MEMORY
