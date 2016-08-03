
#ifndef ITKIOGIPL_EXPORT_H
#define ITKIOGIPL_EXPORT_H

#ifdef ITK_STATIC
#  define ITKIOGIPL_EXPORT
#  define ITKIOGIPL_HIDDEN
#else
#  ifndef ITKIOGIPL_EXPORT
#    ifdef ITKIOGIPL_EXPORTS
        /* We are building this library */
#      define ITKIOGIPL_EXPORT 
#    else
        /* We are using this library */
#      define ITKIOGIPL_EXPORT 
#    endif
#  endif

#  ifndef ITKIOGIPL_HIDDEN
#    define ITKIOGIPL_HIDDEN 
#  endif
#endif

#ifndef ITKIOGIPL_DEPRECATED
#  define ITKIOGIPL_DEPRECATED __attribute__ ((__deprecated__))
#  define ITKIOGIPL_DEPRECATED_EXPORT ITKIOGIPL_EXPORT __attribute__ ((__deprecated__))
#  define ITKIOGIPL_DEPRECATED_NO_EXPORT ITKIOGIPL_HIDDEN __attribute__ ((__deprecated__))
#endif

#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
# define ITKIOGIPL_NO_DEPRECATED
#endif

#endif
