#ifndef __NS_STD_CONFIG_DB_H__
#define __NS_STD_CONFIG_DB_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmemory.h>
#include <std/nslist.h>
#include <std/nshashlist.h>
#include <std/nsvalue.h>
#include <std/nsstring.h>
#include <std/nsascii.h>
#include <std/nsprint.h>
#include <std/nsfile.h>

NS_DECLS_BEGIN

typedef enum
	{
	NS_CONFIG_DB_VALUE_STRING,
	NS_CONFIG_DB_VALUE_INT,
	NS_CONFIG_DB_VALUE_BOOLEAN,
	NS_CONFIG_DB_VALUE_DOUBLE
	}
	NsConfigDbValueType;


typedef struct _NsConfigDb
	{
	NsHashList  groups;
	NsList      comments;
	}
	NsConfigDb;


typedef nshashlistiter     nsconfigdbiter;
typedef nshashlistreviter  nsconfigdbreviter;


NS_IMPEXP NsError ns_config_db_construct( NsConfigDb *db );
NS_IMPEXP void ns_config_db_destruct( NsConfigDb *db );

NS_IMPEXP void ns_config_db_clear( NsConfigDb *db );

NS_IMPEXP nssize ns_config_db_num_groups( const NsConfigDb *db );
NS_IMPEXP nssize ns_config_db_num_keys( const NsConfigDb *db, const nschar *group );

/* If 'group' is NULL, the comment occurs before any groups. */
NS_IMPEXP NsError ns_config_db_add_comment
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *comment
	);

/* 'lines' is set to the line in the db where an
	error occurred, Can be NULL if not necessary. */
NS_IMPEXP NsError ns_config_db_read( NsConfigDb *db, const nschar *path, nssize *lines );


NS_IMPEXP NsError ns_config_db_read_group
	(
	NsConfigDb    *db,
	const nschar  *path,
	const nschar  *group,
	nsboolean     *found, /* Can be NULL if not needed. */
	nssize        *lines /* Can be NULL if not needed. */
	);


/* Same as above except that all keys are interpreted as strings. i.e. no
	parsing occurs just a copy of the text. */
NS_IMPEXP NsError ns_config_db_read_strings( NsConfigDb *db, const nschar *path, nssize *lines );

/* All the values in the group are read as one long string. An example would be
	a comments section where there are no key-value pairs, just a paragaph e.g.
	describing something. Each group will have one key-value pair whose name
	is simply "key" and whose value is a string of all the text in that group.
	So to access that string call e.g. ns_config_db_get_string( db, "comments", "key" ),
	that is assuming that the group named "comments" exists. Note that the existence
	of this key should still be checked for since the group contain no text.
	Example file:
	[A]
	[B]
	Group A will not have the "key" value present since it has no text. */
NS_IMPEXP NsError ns_config_db_read_groups( NsConfigDb *db, const nschar *path, nssize *lines );

NS_IMPEXP NsError ns_config_db_write( const NsConfigDb *db, const nschar *path );

NS_IMPEXP NsError ns_config_db_add_group( NsConfigDb *db, const nschar *group );
NS_IMPEXP void ns_config_db_remove_group( NsConfigDb *db, const nschar *group );
NS_IMPEXP nsboolean ns_config_db_has_group( const NsConfigDb *db, const nschar *group );

NS_IMPEXP NsError ns_config_db_add_string
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key,
	const nschar  *value
	);

NS_IMPEXP NsError ns_config_db_add_int
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key,
	nsint          value
	);

NS_IMPEXP NsError ns_config_db_add_boolean
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key,
	nsboolean      value
	);

NS_IMPEXP NsError ns_config_db_add_double
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key,
	nsdouble       value
	);

NS_IMPEXP NsError ns_config_db_add_string_if_not_exist
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key,
	const nschar  *value
	);

NS_IMPEXP NsError ns_config_db_add_int_if_not_exist
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key,
	nsint          value
	);

NS_IMPEXP NsError ns_config_db_add_boolean_if_not_exist
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key,
	nsboolean      value
	);

NS_IMPEXP NsError ns_config_db_add_double_if_not_exist
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key,
	nsdouble       value
	);

NS_IMPEXP void ns_config_db_remove_key
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key
	);

NS_IMPEXP nsboolean ns_config_db_has_key
	(
	const NsConfigDb  *db,
	const nschar      *group,
	const nschar      *key
	);

NS_IMPEXP NsConfigDbValueType ns_config_db_value_type
	(
	const NsConfigDb  *db,
	const nschar      *group,
	const nschar      *key
	);

NS_IMPEXP const nschar* ns_config_db_get_string
	(
	const NsConfigDb  *db,
	const nschar      *group,
	const nschar      *key
	);

NS_IMPEXP void ns_config_db_set_string
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key,
	const nschar  *value
	);

NS_IMPEXP nsint ns_config_db_get_int
	(
	const NsConfigDb  *db,
	const nschar      *group,
	const nschar      *key
	);

NS_IMPEXP void ns_config_db_set_int
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key,
	nsint          value
	);

NS_IMPEXP nsboolean ns_config_db_get_boolean
	(
	const NsConfigDb  *db,
	const nschar      *group,
	const nschar      *key
	);

NS_IMPEXP void ns_config_db_set_boolean
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key,
	nsboolean      value
	);

NS_IMPEXP nsdouble ns_config_db_get_double
	(
	const NsConfigDb  *db,
	const nschar      *group,
	const nschar      *key
	);

NS_IMPEXP void ns_config_db_set_double
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key,
	nsdouble       value
	);

/* If 'group' is NULL same as ns_config_db_add_comment(). Cast return
	type of ns_list_iter_get_object() to a const nschar*. */
NS_IMPEXP nslistiter ns_config_db_begin_comments( const NsConfigDb *db, const nschar *group );
NS_IMPEXP nslistiter ns_config_db_end_comments( const NsConfigDb *db, const nschar *group );

/* Returns ns_config_db_end_groups() if 'group' not found. */
NS_IMPEXP nsconfigdbiter ns_config_db_lookup_group( const NsConfigDb *db, const nschar *group );

/* Returns ns_config_db_end_keys() if 'key' not found. */
NS_IMPEXP nsconfigdbiter ns_config_db_lookup_key( const nsconfigdbiter G, const nschar *key );

NS_IMPEXP nsconfigdbiter ns_config_db_begin_groups( const NsConfigDb *db );
NS_IMPEXP nsconfigdbiter ns_config_db_end_groups( const NsConfigDb *db );

NS_IMPEXP nsconfigdbreviter ns_config_db_rev_begin_groups( const NsConfigDb *db );
NS_IMPEXP nsconfigdbreviter ns_config_db_rev_end_groups( const NsConfigDb *db );

NS_IMPEXP nsconfigdbiter ns_config_db_begin_keys( const nsconfigdbiter G );
NS_IMPEXP nsconfigdbiter ns_config_db_end_keys( const nsconfigdbiter G );

NS_IMPEXP const nschar* ns_config_db_iter_string( const nsconfigdbiter I );
NS_IMPEXP const nschar* ns_config_db_rev_iter_string( const nsconfigdbreviter I );

#define ns_config_db_iter_equal( I1, I2 )\
	ns_hash_list_iter_equal( (I1), (I2) )

#define ns_config_db_iter_not_equal( I1, I2 )\
	ns_hash_list_iter_not_equal( (I1), (I2) )

#define ns_config_db_iter_next( I )\
	ns_hash_list_iter_next( (I) )

#define ns_config_db_iter_prev( I )\
	ns_hash_list_iter_prev( (I) )

#define ns_config_db_iter_offset( I, n )\
	ns_hash_list_iter_offset( (I), (n) )

#define ns_config_db_rev_iter_equal( I1, I2 )\
	ns_hash_list_rev_iter_equal( (I1), (I2) )

#define ns_config_db_rev_iter_not_equal( I1, I2 )\
	ns_hash_list_rev_iter_not_equal( (I1), (I2) )

#define ns_config_db_rev_iter_next( I )\
	ns_hash_list_rev_iter_next( (I) )

#define ns_config_db_rev_iter_prev( I )\
	ns_hash_list_rev_iter_prev( (I) )


typedef union _NsConfigDbValue
	{
	const nschar  *as_string;
	nsint          as_int;
	nsboolean      as_boolean;
	nsdouble       as_double;
	}
	NsConfigDbValue;

typedef struct _NsConfigDbKey
	{
	const nschar         *name;
	NsConfigDbValueType   type;
	NsConfigDbValue       value;
	}
	NsConfigDbKey;

typedef struct _NsConfigDbGroup
	{
	const nschar    *name;
	nssize           size;
	NsConfigDbKey   *keys;
	}
	NsConfigDbGroup;


NS_IMPEXP NsError ns_config_db_add_all( NsConfigDb *db, const NsConfigDbGroup *group );
NS_IMPEXP void ns_config_db_get_all( const NsConfigDb *db, NsConfigDbGroup *group );
NS_IMPEXP void ns_config_db_set_all( NsConfigDb *db, const NsConfigDbGroup *group );

NS_DECLS_END

#endif /* __NS_STD_CONFIG_DB_H__ */
