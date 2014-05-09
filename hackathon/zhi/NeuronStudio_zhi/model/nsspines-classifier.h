#ifndef __NS_MODEL_SPINES_CLASSIFIER_H__
#define __NS_MODEL_SPINES_CLASSIFIER_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsbits.h>
#include <std/nsprogress.h>
#include <std/nsmemory.h>
#include <std/nsstring.h>
#include <std/nsconfigdb.h>
#include <std/nsutil.h>
#include <std/nsstring.h>
#include <std/nslist.h>
#include <std/nsdir.h>
#include <std/nshashtable.h>
#include <std/nsprogress.h>
#include <std/nsascii.h>
#include <math/nsrandom.h>
#include <fann/floatfann.h>

NS_DECLS_BEGIN

typedef struct _NsSpineClass
	{
	nschar  *name;
	nsint    value;
	}
	NsSpineClass;


#define NS_SPINES_CLASSIFIER_VERBOSE

typedef struct _NsSpinesClassifier
	{
	NsList                    classes;
	struct fann_train_data   *data;
	struct fann              *ann;
	nschar                   *name;
	nschar                   *file;
	NsList                    data_cache;
	nsint                     training_method;
	NsSpineClass            **lookup;
	}
	NsSpinesClassifier;


typedef nslistiter nsspineclass;


NS_IMPEXP void ns_spines_classifier_construct( NsSpinesClassifier *sc );
NS_IMPEXP void ns_spines_classifier_destruct( NsSpinesClassifier *sc );

NS_IMPEXP void ns_spines_classifier_clear( NsSpinesClassifier *sc );
NS_IMPEXP void ns_spines_classifier_clear_data_and_network( NsSpinesClassifier *sc );

NS_IMPEXP const nschar* ns_spines_classifier_get_name( const NsSpinesClassifier *sc );
NS_IMPEXP NsError ns_spines_classifier_set_name( NsSpinesClassifier *sc, const nschar *name );

NS_IMPEXP const nschar* ns_spines_classifier_get_file( const NsSpinesClassifier *sc );
NS_IMPEXP NsError ns_spines_classifier_set_file( NsSpinesClassifier *sc, const nschar *file );


enum{
	NS_SPINES_CLASSIFIER_CHECK_PARSE_OK,
	NS_SPINES_CLASSIFIER_CHECK_PARSE_EMPTY,
	NS_SPINES_CLASSIFIER_CHECK_PARSE_DUPLICATE
	};

/* Returns one of the above enumerated types. */
NS_IMPEXP nsenum ns_spines_classifier_check_parse_classes( const nschar *string );

NS_IMPEXP nsboolean ns_spines_classifier_has_class_name( const NsSpinesClassifier *sc, const nschar *name );
NS_IMPEXP nsboolean ns_spines_classifier_has_class_value( const NsSpinesClassifier *sc, nsint value );
NS_IMPEXP nsboolean ns_spines_classifier_has_class_name_or_value( const NsSpinesClassifier *sc, const nschar *name, nsint value );

NS_IMPEXP NsError ns_spines_classifier_add_class( NsSpinesClassifier *sc, const nschar *name, nsint value );

NS_IMPEXP nssize ns_spines_classifier_max_class_name_length( const NsSpinesClassifier *sc );


#ifdef NS_DEBUG

NS_IMPEXP void ns_spines_classifier_print( const NsSpinesClassifier *sc );

NS_IMPEXP void ns_spines_classifier_print_parse_classes( const nschar *string );

#endif/* NS_DEBUG */


/* The input is a comma delimited string of class names. */
NS_IMPEXP NsError ns_spines_classifier_parse_classes( NsSpinesClassifier *sc, const nschar *string );

NS_IMPEXP nssize ns_spines_classifier_num_classes( const NsSpinesClassifier *sc );

/* Makes a comma delimited string from the class names. */
NS_IMPEXP NsError ns_spines_classifier_classes_string( const NsSpinesClassifier *sc, NsString *string );


/* Adds an example to the training set. */
NS_IMPEXP NsError ns_spines_classifier_add_datum
	(
	NsSpinesClassifier  *sc,
	const fann_type     *input,
	nssize               num_input,
	const fann_type     *output,
	nssize               num_output
	);


NS_IMPEXP nssize ns_spines_classifier_num_datums( const NsSpinesClassifier *sc );


NS_IMPEXP NsError ns_spines_classifier_get_datum
	(
	NsSpinesClassifier  *sc,
	nssize               at,
	fann_type           *input,
	nssize               num_input,
	fann_type           *output,
	nssize               num_output
	);

NS_IMPEXP NsError ns_spines_classifier_set_datum
	(
	NsSpinesClassifier  *sc,
	nssize               at,
	const fann_type     *input,
	nssize               num_input,
	const fann_type     *output,
	nssize               num_output
	);

NS_IMPEXP NsError ns_spines_classifier_type_datum
	(
	NsSpinesClassifier  *sc,
	nssize               at,
	nsspineclass         klass,
	fann_type           *output,
	nssize               num_output
	);

NS_IMPEXP NsError ns_spines_classifier_remove_datum
	(
	NsSpinesClassifier  *sc,
	nssize               at
	);


/* Returns a class based on the maximum output value. */
NS_IMPEXP nsint ns_spines_classifier_type
	(
	const NsSpinesClassifier  *sc,
	const fann_type           *output,
	nssize                     num_output
	);


/* Runs a classification on an input example. Will run training first
	if necessary. 'valid' is set to false only if there is no training
	data added to the classifier yet. */
NS_IMPEXP NsError ns_spines_classifier_run
	(
	NsSpinesClassifier  *sc,
	const fann_type     *input,
	nssize               num_input,
	fann_type           *output,
	nssize               num_output,
	nsboolean           *valid,
	NsProgress          *progress
	);


NS_IMPEXP NsError ns_spines_classifier_read( NsSpinesClassifier *sc, const nschar *file_name );
NS_IMPEXP NsError ns_spines_classifier_write( const NsSpinesClassifier *sc, const nschar *file_name );


NS_IMPEXP nsspineclass ns_spines_classifier_begin( const NsSpinesClassifier *sc );
NS_IMPEXP nsspineclass ns_spines_classifier_end( const NsSpinesClassifier *sc );

#define ns_spine_class_not_equal( klass1, klass2 )\
	ns_list_iter_not_equal( (klass1), (klass2) )

#define ns_spine_class_next( klass )\
	ns_list_iter_next( (klass) )

NS_IMPEXP const nschar* ns_spine_class_name( const nsspineclass klass );
NS_IMPEXP nsint ns_spine_class_value( const nsspineclass klass );


/* The find functions return ns_spines_classifier_end() if no match. */
NS_IMPEXP nsspineclass ns_spines_classifier_find_by_name( const NsSpinesClassifier *sc, const nschar *name );
NS_IMPEXP nsspineclass ns_spines_classifier_find_by_value( const NsSpinesClassifier *sc, nsint value );


/* Fast way to map a class value to a class name. */
NS_IMPEXP NsError ns_spines_classifier_init_lookup( NsSpinesClassifier *sc );
NS_IMPEXP void ns_spines_classifier_finalize_lookup( NsSpinesClassifier *sc );

#define NS_SPINES_CLASSIFIER_NA  "N/A"
/* Returns NS_SPINES_CLASSIFIER_NA if the value isnt found. */
NS_IMPEXP const nschar* ns_spines_classifier_lookup( const NsSpinesClassifier *sc, nsint value );


/* Global classifier object. */
NS_IMPEXP NsError ns_spines_classifier_new_by_classes( const nschar *name, const nschar *classes );
NS_IMPEXP NsError ns_spines_classifier_new_by_file( const nschar *file_name );

NS_IMPEXP void ns_spines_classifier_delete( void );

NS_IMPEXP NsSpinesClassifier* ns_spines_classifier_get( void );
NS_IMPEXP void ns_spines_classifier_set( NsSpinesClassifier *sc );


NS_IMPEXP NsError ns_spines_classifier_new_ex( NsSpinesClassifier **sc, const nschar *name );
NS_IMPEXP NsError ns_spines_classifier_new_by_classes_ex( NsSpinesClassifier **sc, const nschar *name, const nschar *classes );
NS_IMPEXP NsError ns_spines_classifier_new_by_file_ex( NsSpinesClassifier **sc, const nschar *file_name );

NS_IMPEXP void ns_spines_classifier_delete_ex( NsSpinesClassifier **sc );


#define NS_SPINES_PLUGIN_FILE_EXTENSION  ".nssc"

typedef struct _NsSpinesPlugins
	{
	NsList   entries;
	nschar  *dir;
	nschar   path[ NS_PATH_SIZE ];
	}
	NsSpinesPlugins;

typedef nslistiter nsspinesplugin;


NS_IMPEXP void ns_spines_plugins_construct( NsSpinesPlugins *scp );
NS_IMPEXP void ns_spines_plugins_destruct( NsSpinesPlugins *scp );

NS_IMPEXP void ns_spines_plugins_clear( NsSpinesPlugins *scp );

/* Removes all plugins marked as temporary from the list and deletes their files as well. */
NS_IMPEXP void ns_spines_plugins_remove_and_delete_temps( NsSpinesPlugins *scp );

NS_IMPEXP nssize ns_spines_plugins_size( const NsSpinesPlugins *scp );

NS_IMPEXP NsError ns_spines_plugins_check_name( NsSpinesPlugins *scp, const nschar *name, nsboolean *ok );

/* Call check_name() first to make sure this one doesnt overwrite one that exists
	with the same name already. */
NS_IMPEXP NsError ns_spines_plugins_add
	(
	NsSpinesPlugins     *scp,
	NsSpinesClassifier  *sc,
	nsboolean            is_temp,
	nsspinesplugin      *sp
	);

NS_IMPEXP void ns_spines_plugins_remove_and_delete( NsSpinesPlugins *scp, nsspinesplugin sp );

#define ns_spines_plugins_is_empty( scp )\
	( 0 == ns_spines_plugins_size( (scp) ) )

NS_IMPEXP NsError ns_spines_plugins_read( NsSpinesPlugins *scp, const nschar *dir );

NS_IMPEXP nsboolean ns_spines_plugins_exists( const NsSpinesPlugins *scp, const nschar *name );

/* Returns ns_spines_plugins_end() if not found. */
NS_IMPEXP nsspinesplugin ns_spines_plugins_find( const NsSpinesPlugins *scp, const nschar *name );

NS_IMPEXP const nschar* ns_spines_plugins_file( const NsSpinesPlugins *scp, const nschar *name );

NS_IMPEXP const nschar* ns_spines_plugins_make_path( NsSpinesPlugins *scp, const nsspinesplugin sp );

/* Returns ns_spines_plugins_end() if 'at' is not a valid index. */
NS_IMPEXP nsspinesplugin ns_spines_plugins_at( const NsSpinesPlugins *scp, nssize at );

NS_IMPEXP nsspinesplugin ns_spines_plugins_begin( const NsSpinesPlugins *scp );
NS_IMPEXP nsspinesplugin ns_spines_plugins_end( const NsSpinesPlugins *scp );

#define ns_spines_plugin_not_equal( sp1, sp2 )\
	ns_list_iter_not_equal( (sp1), (sp2) )

#define ns_spines_plugin_next( sp )\
	ns_list_iter_next( (sp) )

NS_IMPEXP const nschar* ns_spines_plugin_name( const nsspinesplugin sp );
NS_IMPEXP const nschar* ns_spines_plugin_file( const nsspinesplugin sp );


#ifdef NS_DEBUG
NS_IMPEXP void ns_spines_plugins_print( const NsSpinesPlugins *scp );
#endif

NS_DECLS_END

#endif/* __NS_MODEL_SPINES_CLASSIFIER_H__ */
