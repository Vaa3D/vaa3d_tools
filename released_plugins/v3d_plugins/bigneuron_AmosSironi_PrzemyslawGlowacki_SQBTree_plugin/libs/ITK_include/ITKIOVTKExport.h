
#ifndef ITKIOVTK_EXPORT_H
#define ITKIOVTK_EXPORT_H

#ifdef ITK_STATIC
#  define ITKIOVTK_EXPORT
#  define ITKIOVTK_HIDDEN
#else
#  ifndef ITKIOVTK_EXPORT
#    ifdef ITKIOVTK_EXPORTS
        /* We are building this library */
#      define ITKIOVTK_EXPORT 
#    else
        /* We are using this library */
#      define ITKIOVTK_EXPORT 
#    endif
#  endif

#  ifndef ITKIOVTK_HIDDEN
#    define ITKIOVTK_HIDDEN 
#  endif
#endif

#ifndef ITKIOVTK_DEPRECATED
#  define ITKIOVTK_DEPRECATED __attribute__ ((__deprecated__))
#  define ITKIOVTK_DEPRECATED_EXPORT ITKIOVTK_EXPORT __attribute__ ((__deprecated__))
#  define ITKIOVTK_DEPRECATED_NO_EXPORT ITKIOVTK_HIDDEN __attribute__ ((__deprecated__))
#endif

#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
# define ITKIOVTK_NO_DEPRECATED
#endif

#endif
