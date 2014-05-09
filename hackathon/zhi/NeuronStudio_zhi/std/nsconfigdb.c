#include "nsconfigdb.h"


#define _NS_CONFIG_DB_GROUP  1
#define _NS_CONFIG_DB_KEY    2


typedef struct _NsConfigDbBase
	{
	const nschar  *name;
	nsint          type;
	}
	NsConfigDbBase;


NS_PRIVATE NsError ns_config_db_base_construct
	(
	NsConfigDbBase  *base,
	const nschar    *name,
	nsint            type
	)
	{
	if( NULL == ( base->name = ns_ascii_strdup( name ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	base->type = type;

	return ns_no_error();
	}


NS_PRIVATE void ns_config_db_base_destruct( NsConfigDbBase *base )
	{  ns_free( ( nspointer )base->name );  }


#define ns_config_db_base_name( base )\
	( ( ( const NsConfigDbBase* )(base) )->name )


#define ns_config_db_base_type( base )\
	( ( ( const NsConfigDbBase* )(base) )->type )


typedef struct ____NsConfigDbKey
	{
	NsConfigDbBase  super; /* Must be first member! */
	NsValue         value;
	}
	_NsConfigDbKey;


NS_PRIVATE NsError ns_config_db_key_new
	(
	_NsConfigDbKey  **ret_key,
	const nschar     *name,
	const nschar     *type
	)
	{
	_NsConfigDbKey  *key;
	NsError         error;


	if( NULL == ( key = ns_new( _NsConfigDbKey ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	if( NS_FAILURE( ns_config_db_base_construct(
							&key->super,
							name,
							_NS_CONFIG_DB_KEY
							),
							error ) )
		{
		ns_delete( key );
		return error;
		}

	ns_value_construct_type( &key->value, type );

	*ret_key = key;
	return ns_no_error();
	}


NS_PRIVATE void ns_config_db_key_delete( _NsConfigDbKey *key )
	{
	ns_value_destruct( &key->value );

	ns_config_db_base_destruct( &key->super );
	ns_delete( key );
	}


typedef struct ____NsConfigDbGroup
	{
	NsConfigDbBase  super; /* Must be first member! */
	NsHashList      keys;
	NsList          comments;
	}
	_NsConfigDbGroup;


NS_PRIVATE NsError ns_config_db_group_new( _NsConfigDbGroup **ret_group, const nschar *name )
	{
	_NsConfigDbGroup  *group;
	NsError            error;


	if( NULL == ( group = ns_new( _NsConfigDbGroup ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	if( NS_FAILURE( ns_config_db_base_construct(
							&group->super,
							name,
							_NS_CONFIG_DB_GROUP
							),
							error ) )
		{
		ns_delete( group );
		return error;
		}

	if( NS_FAILURE( ns_hash_list_construct(
							&group->keys,
							ns_ascii_hash,
							ns_ascii_streq,
							NULL,
							ns_config_db_key_delete
							),
							error ) )
		{
		ns_config_db_base_destruct( &group->super );
		ns_delete( group );

		return error;
		}

	ns_list_construct( &group->comments, ns_free );

	*ret_group = group;
	return ns_no_error();
	}


NS_PRIVATE void ns_config_db_group_delete( _NsConfigDbGroup *group )
	{
	ns_list_destruct( &group->comments );
	ns_hash_list_destruct( &group->keys );

	ns_config_db_base_destruct( &group->super );
	ns_delete( group );
	}


NS_PRIVATE _NsConfigDbGroup* _ns_config_db_lookup_group( const NsConfigDb *db, const nschar *group )
	{
	nshashlistiter I;

	if( ! ns_hash_list_lookup( &db->groups, ( nspointer )group, &I ) )
		return NULL;

	return ns_hash_list_iter_get_object( I );
	}


NS_PRIVATE _NsConfigDbKey* _ns_config_db_lookup_key
	(
	const NsConfigDb  *db,
	const nschar      *group,
	const nschar      *key
	)
	{
	nshashlistiter I;

	if( ! ns_hash_list_lookup(
			&( _ns_config_db_lookup_group( db, group )->keys ),
			( nspointer )key,
			&I ) )
		return NULL;

	return ns_hash_list_iter_get_object( I );
	}


NS_PRIVATE NsValue* ns_config_db_lookup_value
	(
	const NsConfigDb  *db,
	const nschar      *group,
	const nschar      *key
	)
	{
	ns_assert( NULL != db );
	ns_assert( NULL != group );
	ns_assert( ns_config_db_has_group( db, group ) );
	ns_assert( NULL != key );
	ns_assert( ns_config_db_has_key( db, group, key ) );

	return &( _ns_config_db_lookup_key( db, group, key )->value );
	}


NS_PRIVATE NsValue* ns_config_db_lookup_value_type
	(
	const NsConfigDb  *db,
	const nschar      *group,
	const nschar      *key,
	const nschar      *type
	)
	{
	NsValue *value = ns_config_db_lookup_value( db, group, key );

	ns_verify( ns_value_is( value, type ) );
	return value;
	}


NS_PRIVATE NsError ns_config_db_add_key
	(
	const NsConfigDb  *db,
	const nschar      *group,
	const nschar      *name,
	const nschar      *type
	)
	{
	_NsConfigDbKey  *key;
	NsError          error;


	ns_assert( NULL != db );
	ns_assert( NULL != group );
	ns_assert( ns_config_db_has_group( db, group ) );
	ns_assert( NULL != name );
	ns_assert( ! ns_config_db_has_key( db, group, name ) );

	if( NS_FAILURE( ns_config_db_key_new( &key, name, type ), error ) )
		return error;

	if( NS_FAILURE( ns_hash_list_add(
							&( _ns_config_db_lookup_group( db, group )->keys ),
							( nspointer )ns_config_db_base_name( key ),
							key
							),
							error ) )
		{
		ns_config_db_key_delete( key );
		return error;
		}

	return ns_no_error();
	}


NS_PRIVATE NsError _ns_config_db_comment_list_add( NsList *comments, const nschar *src )
	{
	nschar   *dest;
	NsError   error;


	if( NULL == ( dest = ns_ascii_strdup( src ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	if( NS_FAILURE( ns_list_push_back( comments, dest ), error ) )
		{
		ns_free( dest );
		return error;
		}

	return ns_no_error();
	}


NsError ns_config_db_construct( NsConfigDb *db )
	{
	NsError error;

	ns_assert( NULL != db );

	if( NS_FAILURE( ns_hash_list_construct(
							&db->groups,
							ns_ascii_hash,
							ns_ascii_streq,
							NULL,
							ns_config_db_group_delete
							),
							error ) )
		return error;

	ns_list_construct( &db->comments, ns_free );

	return ns_no_error();
	}


void ns_config_db_destruct( NsConfigDb *db )
	{
	ns_assert( NULL != db );

	ns_list_destruct( &db->comments );
	ns_hash_list_destruct( &db->groups );
	}


void ns_config_db_clear( NsConfigDb *db )
	{
	ns_assert( NULL != db );

	ns_list_clear( &db->comments );
	ns_hash_list_clear( &db->groups );
	}


nssize ns_config_db_num_groups( const NsConfigDb *db )
	{
	ns_assert( NULL != db );
	return ns_hash_list_size( &db->groups );
	}


nssize ns_config_db_num_keys( const NsConfigDb *db, const nschar *group )
	{
	ns_assert( NULL != db );
	ns_assert( NULL != group );
	ns_assert( ns_config_db_has_group( db, group ) );

	return ns_hash_list_size( &( _ns_config_db_lookup_group( db, group )->keys ) );
	}


NsError ns_config_db_add_comment
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *comment
	)
	{
	ns_assert( NULL != db );
	ns_assert( NULL != comment );

	if( NULL == group )
		return _ns_config_db_comment_list_add( &db->comments, comment );
	else
		{
		ns_assert( ns_config_db_has_group( db, group ) );

		return _ns_config_db_comment_list_add(
					&( _ns_config_db_lookup_group( db, group )->comments ),
					comment
					);
		}
	}


NsError ns_config_db_add_group( NsConfigDb *db, const nschar *name )
	{
	_NsConfigDbGroup  *group;
	NsError            error;


	ns_assert( NULL != db );
	ns_assert( NULL != name );
	ns_assert( ! ns_config_db_has_group( db, name ) );

	if( NS_FAILURE( ns_config_db_group_new( &group, name ), error ) )
		return error;

	if( NS_FAILURE( ns_hash_list_add(
							&db->groups,
							( nspointer )ns_config_db_base_name( group ),
							group
							),
							error ) )
		{
		ns_config_db_group_delete( group );
		return error;
		}

	return ns_no_error();
	}


void ns_config_db_remove_group( NsConfigDb *db, const nschar *group )
	{
	ns_assert( NULL != db );
	ns_assert( NULL != group );
	ns_assert( ns_config_db_has_group( db, group ) );

	ns_verify( ns_hash_list_remove( &db->groups, ( nspointer )group ) );
	}


nsboolean ns_config_db_has_group( const NsConfigDb *db, const nschar *group )
	{
	ns_assert( NULL != db );
	ns_assert( NULL != group );

	return NULL != _ns_config_db_lookup_group( db, group );
	}


NsError ns_config_db_add_string
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key,
	const nschar  *value
	)
	{
	NsError error;

	if( NS_FAILURE( ns_config_db_add_key( db, group, key, NS_VALUE_STRING ), error ) )
		return error;

	ns_config_db_set_string( db, group, key, value );
	return ns_no_error();
	}


NsError ns_config_db_add_int
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key,
	nsint          value
	)
	{
	NsError error;

	if( NS_FAILURE( ns_config_db_add_key( db, group, key, NS_VALUE_INT ), error ) )
		return error;

	ns_config_db_set_int( db, group, key, value );
	return ns_no_error();
	}


NsError ns_config_db_add_boolean
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key,
	nsboolean      value
	)
	{
	NsError error;

	if( NS_FAILURE( ns_config_db_add_key( db, group, key, NS_VALUE_BOOLEAN ), error ) )
		return error;

	ns_config_db_set_boolean( db, group, key, value );
	return ns_no_error();
	}


NsError ns_config_db_add_double
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key,
	nsdouble       value
	)
	{
	NsError error;

	if( NS_FAILURE( ns_config_db_add_key( db, group, key, NS_VALUE_DOUBLE ), error ) )
		return error;

	ns_config_db_set_double( db, group, key, value );
	return ns_no_error();
	}


NsError ns_config_db_add_string_if_not_exist
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key,
	const nschar  *value
	)
	{
	NsError error;

	if( ! ns_config_db_has_key( db, group, key ) )
		if( NS_FAILURE( ns_config_db_add_string( db, group, key, value ), error ) )
			return error;

	return ns_no_error();
	}


NsError ns_config_db_add_int_if_not_exist
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key,
	nsint          value
	)
	{
	NsError error;

	if( ! ns_config_db_has_key( db, group, key ) )
		if( NS_FAILURE( ns_config_db_add_int( db, group, key, value ), error ) )
			return error;

	return ns_no_error();
	}


NsError ns_config_db_add_boolean_if_not_exist
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key,
	nsboolean      value
	)
	{
	NsError error;

	if( ! ns_config_db_has_key( db, group, key ) )
		if( NS_FAILURE( ns_config_db_add_boolean( db, group, key, value ), error ) )
			return error;

	return ns_no_error();
	}


NsError ns_config_db_add_double_if_not_exist
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key,
	nsdouble       value
	)
	{
	NsError error;

	if( ! ns_config_db_has_key( db, group, key ) )
		if( NS_FAILURE( ns_config_db_add_double( db, group, key, value ), error ) )
			return error;

	return ns_no_error();
	}


void ns_config_db_remove_key
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key
	)
	{
	ns_assert( NULL != db );
	ns_assert( NULL != group );
	ns_assert( ns_config_db_has_group( db, group ) );
	ns_assert( NULL != key );
	ns_assert( ns_config_db_has_key( db, group, key ) );

	ns_verify(
		ns_hash_list_remove(
			&( _ns_config_db_lookup_group( db, group )->keys ),
			( nspointer )key
			)
		);
	}


nsboolean ns_config_db_has_key
	(
	const NsConfigDb  *db,
	const nschar      *group,
	const nschar      *key
	)
	{
	ns_assert( NULL != db );
	ns_assert( NULL != group );
	ns_assert( ns_config_db_has_group( db, group ) );
	ns_assert( NULL != key );

	return NULL != _ns_config_db_lookup_key( db, group, key );
	}


NsConfigDbValueType ns_config_db_value_type
	(
	const NsConfigDb  *db,
	const nschar      *group,
	const nschar      *key
	)
	{
	const NsValue        *value;
	NsConfigDbValueType   type;


	value = ns_config_db_lookup_value( db, group, key );

	if( ns_value_is( value, NS_VALUE_STRING ) )
		type = NS_CONFIG_DB_VALUE_STRING;
	else if( ns_value_is( value, NS_VALUE_INT ) )
		type = NS_CONFIG_DB_VALUE_INT;
	else if( ns_value_is( value, NS_VALUE_BOOLEAN ) )
		type = NS_CONFIG_DB_VALUE_BOOLEAN;
	else
		{
		ns_verify( ns_value_is( value, NS_VALUE_DOUBLE ) );
		type = NS_CONFIG_DB_VALUE_DOUBLE;
		}

	return type;
	}


const nschar* ns_config_db_get_string
	(
	const NsConfigDb  *db,
	const nschar      *group,
	const nschar      *key
	)
	{
	return ns_value_get_string(
				ns_config_db_lookup_value_type( db, group, key, NS_VALUE_STRING )
				); 
	}


void ns_config_db_set_string
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key,
	const nschar  *value
	)
	{
	ns_value_set_string(
		ns_config_db_lookup_value_type( db, group, key, NS_VALUE_STRING ),
		( nspointer )value
		);
	}


nsint ns_config_db_get_int
	(
	const NsConfigDb  *db,
	const nschar      *group,
	const nschar      *key
	)
	{
	return ns_value_get_int(
				ns_config_db_lookup_value_type( db, group, key, NS_VALUE_INT )
				);
	}


void ns_config_db_set_int
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key,
	nsint          value
	)
	{
	ns_value_set_int(
		ns_config_db_lookup_value_type( db, group, key, NS_VALUE_INT ),
		value
		);
	}


nsboolean ns_config_db_get_boolean
	(
	const NsConfigDb  *db,
	const nschar      *group,
	const nschar      *key
	)
	{
	return ns_value_get_boolean(
				ns_config_db_lookup_value_type( db, group, key, NS_VALUE_BOOLEAN )
				);
	}


void ns_config_db_set_boolean
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key,
	nsboolean      value
	)
	{
	ns_value_set_boolean(
		ns_config_db_lookup_value_type( db, group, key, NS_VALUE_BOOLEAN ),
		value
		);
	}


nsdouble ns_config_db_get_double
	(
	const NsConfigDb  *db,
	const nschar      *group,
	const nschar      *key
	)
	{
	return ns_value_get_double(
				ns_config_db_lookup_value_type( db, group, key, NS_VALUE_DOUBLE )
				);
	}


void ns_config_db_set_double
	(
	NsConfigDb    *db,
	const nschar  *group,
	const nschar  *key,
	nsdouble       value
	)
	{
	ns_value_set_double(
		ns_config_db_lookup_value_type( db, group, key, NS_VALUE_DOUBLE ),
		value
		);
	}


nslistiter ns_config_db_begin_comments( const NsConfigDb *db, const nschar *group )
	{
	ns_assert( NULL != db );

	if( NULL == group )
		return ns_list_begin( &db->comments );
	else
		{
		ns_assert( ns_config_db_has_group( db, group ) );
		return ns_list_begin( &( _ns_config_db_lookup_group( db, group )->comments ) );
		}
	}


nslistiter ns_config_db_end_comments( const NsConfigDb *db, const nschar *group )
	{
	ns_assert( NULL != db );

	if( NULL == group )
		return ns_list_end( &db->comments );
	else
		{
		ns_assert( ns_config_db_has_group( db, group ) );
		return ns_list_end( &( _ns_config_db_lookup_group( db, group )->comments ) );
		}
	}


nsconfigdbiter ns_config_db_lookup_group( const NsConfigDb *db, const nschar *group )
	{
	nsconfigdbiter G;

	ns_assert( NULL != db );
	ns_assert( NULL != group );

	if( ! ns_hash_list_lookup( &db->groups, ( nspointer )group, &G ) )
		G = ns_config_db_end_groups( db );

	return G;
	}


nsconfigdbiter ns_config_db_lookup_key( const nsconfigdbiter G, const nschar *key )
	{
	const _NsConfigDbGroup  *group;
	nsconfigdbiter           K;


	ns_assert( NULL != G );
	ns_assert( NULL != key );

	group = ns_hash_list_iter_get_object( G );

	if( ! ns_hash_list_lookup( &group->keys, ( nspointer )key, &K ) )
		K = ns_config_db_end_keys( G );

	return K;
	}


nsconfigdbiter ns_config_db_begin_groups( const NsConfigDb *db )
	{
	ns_assert( NULL != db );
	return ns_hash_list_begin( &db->groups );
	}


nsconfigdbiter ns_config_db_end_groups( const NsConfigDb *db )
	{
	ns_assert( NULL != db );
	return ns_hash_list_end( &db->groups );
	}


nsconfigdbreviter ns_config_db_rev_begin_groups( const NsConfigDb *db )
	{
	ns_assert( NULL != db );
	return ns_hash_list_rev_begin( &db->groups );
	}


nsconfigdbreviter ns_config_db_rev_end_groups( const NsConfigDb *db )
	{
	ns_assert( NULL != db );
	return ns_hash_list_rev_end( &db->groups );
	}


const nschar* ns_config_db_iter_string( const nsconfigdbiter I )
	{
	ns_assert( NULL != I );
	return ns_config_db_base_name( ns_hash_list_iter_get_object( I ) );
	}


const nschar* ns_config_db_rev_iter_string( const nsconfigdbreviter I )
	{
	ns_assert( NULL != I );
	return ns_config_db_base_name( ns_hash_list_rev_iter_get_object( I ) );
	}


nsconfigdbiter ns_config_db_begin_keys( const nsconfigdbiter G )
	{
	const _NsConfigDbGroup *group;

	ns_assert( NULL != G );

	group = ns_hash_list_iter_get_object( G );
	ns_assert( _NS_CONFIG_DB_GROUP == ns_config_db_base_type( group ) );

	return ns_hash_list_begin( &group->keys );
	}


nsconfigdbiter ns_config_db_end_keys( const nsconfigdbiter G )
	{
	const _NsConfigDbGroup *group;

	ns_assert( NULL != G );

	group = ns_hash_list_iter_get_object( G );
	ns_assert( _NS_CONFIG_DB_GROUP == ns_config_db_base_type( group ) );

	return ns_hash_list_end( &group->keys );
	}


NsError ns_config_db_write( const NsConfigDb *db, const nschar *path )
	{
	NsFile                file;
	nslistiter            curr_comment;
	nslistiter            end_comments;
	nsconfigdbiter        curr_group;
	nsconfigdbiter        end_groups;
	nsconfigdbiter        curr_key;
	nsconfigdbiter        end_keys;
	const nschar         *group;
	const nschar         *key;
	NsConfigDbValueType   value_type;
	NsError               error;


	ns_assert( NULL != db );
	ns_assert( NULL != path );

	ns_file_construct( &file );

	if( NS_FAILURE( ns_file_open( &file, path, NS_FILE_MODE_WRITE ), error ) )
		{
		ns_file_destruct( &file );
		return error;
		}

	curr_comment = ns_config_db_begin_comments( db, NULL );
	end_comments = ns_config_db_end_comments( db, NULL );

	for( ; ns_list_iter_not_equal( curr_comment, end_comments );
			 curr_comment = ns_list_iter_next( curr_comment ) )
		if( NS_FAILURE( ns_file_print(
								&file,
								"# " NS_FMT_STRING NS_STRING_NEWLINE,
								ns_list_iter_get_object( curr_comment )
								),
								error ) )
			{
			ns_file_destruct( &file );
			return error;
			}

	curr_group = ns_config_db_begin_groups( db );
	end_groups = ns_config_db_end_groups( db );

	for( ; ns_config_db_iter_not_equal( curr_group, end_groups );
			 curr_group = ns_config_db_iter_next( curr_group ) )
		{
		group = ns_config_db_iter_string( curr_group );

		if( NS_FAILURE( ns_file_print(
								&file,
								NS_STRING_NEWLINE "[" NS_FMT_STRING "]" NS_STRING_NEWLINE,
								group
								),
								error ) )
			{
			ns_file_destruct( &file );
			return error;
			}

		curr_comment = ns_config_db_begin_comments( db, group );
		end_comments = ns_config_db_end_comments( db, group );

		for( ; ns_list_iter_not_equal( curr_comment, end_comments );
				 curr_comment = ns_list_iter_next( curr_comment ) )
			if( NS_FAILURE( ns_file_print(
									&file,
									"# " NS_FMT_STRING NS_STRING_NEWLINE,
									ns_list_iter_get_object( curr_comment )
									),
									error ) )
				{
				ns_file_destruct( &file );
				return error;
				}

		curr_key = ns_config_db_begin_keys( curr_group );
		end_keys = ns_config_db_end_keys( curr_group );

		for( ; ns_config_db_iter_not_equal( curr_key, end_keys );
				 curr_key = ns_config_db_iter_next( curr_key ) )
			{
			key = ns_config_db_iter_string( curr_key );

			if( NS_FAILURE( ns_file_print(
									&file,
									NS_FMT_STRING "=",
									ns_config_db_iter_string( curr_key )
									),
									error ) )
				{
				ns_file_destruct( &file );
				return error;
				}

			value_type = ns_config_db_value_type( db, group, key );

			switch( value_type )
				{
				case NS_CONFIG_DB_VALUE_STRING:
					if( NS_FAILURE( ns_file_print(
											&file,
											NS_FMT_STRING NS_STRING_NEWLINE,
											ns_config_db_get_string( db, group, key )
											),
											error ) )
						{
						ns_file_destruct( &file );
						return error;
						}
					break;

				case NS_CONFIG_DB_VALUE_INT:
					if( NS_FAILURE( ns_file_print(
											&file,
											NS_FMT_INT NS_STRING_NEWLINE,
											ns_config_db_get_int( db, group, key )
											),
											error ) )
						{
						ns_file_destruct( &file );
						return error;
						}
					break;

				case NS_CONFIG_DB_VALUE_BOOLEAN:
					if( NS_FAILURE( ns_file_print(
											&file,
											NS_FMT_STRING NS_STRING_NEWLINE,
											ns_config_db_get_boolean( db, group, key ) ? "true" : "false"
											),
											error ) )
						{
						ns_file_destruct( &file );
						return error;
						}
					break;

				case NS_CONFIG_DB_VALUE_DOUBLE:
					if( NS_FAILURE( ns_file_print(
											&file,
											NS_FMT_DOUBLE NS_STRING_NEWLINE,
											ns_config_db_get_double( db, group, key )
											),
											error ) )
						{
						ns_file_destruct( &file );
						return error;
						}
					break;

				default:
					ns_assert_not_reached();
				}
			}
		}

	ns_file_destruct( &file );
	return ns_no_error();
	}


NS_PRIVATE NsError _ns_config_db_parse_comment( NsConfigDb *db, nschar *comment )
	{
	const nschar *group = NULL;

	ns_assert( '#' == *comment );
	
	++comment;

	while( ns_ascii_isspace( *comment ) )
		++comment;

	if( 0 < ns_config_db_num_groups( db ) )
		{
		group = ns_config_db_rev_iter_string( ns_config_db_rev_begin_groups( db ) );

		/* Comments should only come before any keys in a group */
		if( 0 < ns_config_db_num_keys( db, group ) )
			return ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
		}

	return ns_config_db_add_comment( db, group, comment );
	}


NS_PRIVATE NsError _ns_config_db_parse_group( NsConfigDb *db, nschar *group )
	{
	nschar   *end;
	NsError   error;


	ns_assert( '[' == *group );

	/* Find the terminating ']' character. */
	end = group;

	while( *end && ']' != *end )
		++end;

	/* Did we find it? */
	if( ']' != *end )
		return ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	/* Skip the opening '[' character. */
	++group;

	/* The group name cant be empty! */
	if( group >= end )
		return ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	*end = NS_ASCII_NULL;

	if( ns_config_db_has_group( db, group ) )
		return ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	if( NS_FAILURE( ns_config_db_add_group( db, group ), error ) )
		return error;

	return ns_no_error();
	}


NS_PRIVATE nschar* _ns_config_db_parse_double( nschar *p, NsConfigDbValueType *type )
	{
	if( '-' == *p || '+' == *p )
		++p;

	/* Digits before decimal pt. */
	while( ns_ascii_isdigit( *p ) )
		++p;

	if( '.' == *p )
		{
		++p;

		/* Digits after decimal pt. */
		while( ns_ascii_isdigit( *p ) )
			++p;
		}

	if( 'd' == *p || 'D' == *p || 'e' == *p || 'E' == *p )
		{
		++p;

		if( '-' == *p || '+' == *p )
			++p;

		/* Exponent digits. */
		while( ns_ascii_isdigit( *p ) )
			++p;
		}

	*type = NS_CONFIG_DB_VALUE_DOUBLE;
	return p;
	}


NS_PRIVATE nschar* _ns_config_db_parse_int( nschar *p, NsConfigDbValueType *type )
	{
	nschar  *s;
	nsint    hex;


	s   = p;
	hex = 0;

	if( '-' == *p || '+' == *p )
		++p;

	/* Skip hexadecimal prefix if its present. */
	if( '0' == *p && ( 'x' == p[1] || 'X' == p[1] ) )
		{
		p  += 2;
		hex = 1;
		}

	if( hex )
		{
		while( ns_ascii_isxdigit( *p ) )
			++p;
		}
	else
		{
		while( ns_ascii_isdigit( *p ) )
			++p;
		}

	/* If floating point number... */
	if( '.' == *p || 'd' == *p || 'D' == *p || 'e' == *p || 'E' == *p )
		return _ns_config_db_parse_double( s, type );

	*type = NS_CONFIG_DB_VALUE_INT;
	return p;
	}


NS_PRIVATE nschar* _ns_config_db_parse_numeric( nschar *p, NsConfigDbValueType *type )
	{  return _ns_config_db_parse_int( p, type );  }


NS_PRIVATE NsError _ns_config_db_parse_key( NsConfigDb *db, nschar *key, nsboolean strings )
	{
	const nschar         *group;
	nschar               *end;
	nschar               *value;
	NsConfigDbValueType   type;
	NsError               error;


	if( 0 == ns_config_db_num_groups( db ) )
		return ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	/* Find the '=' character. */
	end = key;

	while( *end && '=' != *end )
		++end;

	/* Did we find it? */
	if( '=' != *end )
		return ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	value = end;

	--end;
	++value;

	/* The key name cant be empty! */
	if( key > end )
		return ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	/* Skip any whitespace between the key name and the '='. */
	while( ns_ascii_isspace( *end ) )
		--end;

	ns_assert( key <= end );

	++end;
	*end = NS_ASCII_NULL;

	group = ns_config_db_rev_iter_string( ns_config_db_rev_begin_groups( db ) );

	if( ns_config_db_has_key( db, group, key ) )
		return ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	/* Skip whitespace between the '=' and the value. */
	while( ns_ascii_isspace( *value ) )
		++value;

	/* The value cant be empty. */
	if( NS_ASCII_NULL == *value )
		return ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

	/* What type of value is this? */
	if( ! strings )
		{
		if( ns_ascii_streq( value, "true" ) )
			{
			if( NS_FAILURE( ns_config_db_add_boolean( db, group, key, NS_TRUE ), error ) )
				return error;
			}
		else if( ns_ascii_streq( value, "false" ) )
			{
			if( NS_FAILURE( ns_config_db_add_boolean( db, group, key, NS_FALSE ), error ) )
				return error;
			}
		else if( '+' == *value || '-' == *value || '.' == *value || ns_ascii_isdigit( *value ) )
			{
			end  = _ns_config_db_parse_numeric( value, &type );
			*end = NS_ASCII_NULL;

			switch( type )
				{
				case NS_CONFIG_DB_VALUE_INT:
					if( NS_FAILURE( ns_config_db_add_int( db, group, key, ns_atoi( value ) ), error ) )
						return error;
					break;

				case NS_CONFIG_DB_VALUE_DOUBLE:
					if( NS_FAILURE( ns_config_db_add_double( db, group, key, ns_atod( value ) ), error ) )
						return error;
					break;

				default:
					ns_assert_not_reached();
				}
			}
		else
			strings = NS_TRUE;
		}

	if( strings )
		{
		if( NS_FAILURE( ns_config_db_add_string( db, group, key, value ), error ) )
			return error;
		}

	return ns_no_error();
	}


#define _NS_CONFIG_DB_READ_BUFFER_SIZE  1024

NS_PRIVATE NsError _ns_config_db_do_read( NsConfigDb *db, const nschar *path, nssize *lines, nsboolean strings )
	{
	NsFile      file;
	nsboolean   eof;
	nschar     *ptr;
	NsError     error;
	nschar      buffer[ _NS_CONFIG_DB_READ_BUFFER_SIZE ];


	ns_assert( NULL != db );
	ns_assert( NULL != path );

	ns_file_construct( &file );

	if( NS_FAILURE( ns_file_open( &file, path, NS_FILE_MODE_READ ), error ) )
		{
		ns_file_destruct( &file );
		return error;
		}

	ns_config_db_clear( db );

	eof = NS_FALSE;

	if( NULL != lines )
		*lines = 0;

	while( ! eof )
		{
		*buffer = NS_ASCII_NULL;

		if( NS_FAILURE( ns_file_readline(
								&file,
								buffer,
								_NS_CONFIG_DB_READ_BUFFER_SIZE,
								&eof
								),
								error ) )
			{
			ns_file_destruct( &file );
			return error;
			}

		if( NULL != lines )
			*lines = *lines + 1;

		ptr = buffer;

		/* Skip leading white-space. */
		while( ns_ascii_isspace( *ptr ) )
			++ptr;

		/* Skip empty lines. */
		if( NS_ASCII_NULL == *ptr )
			continue;

		if( '#' == *ptr )/* Its a comment. */
			{
			if( NS_FAILURE( _ns_config_db_parse_comment( db, ptr ), error ) )
				{
				ns_file_destruct( &file );
				return error;
				}
			}
		else if( '[' == *ptr )/* Its a group. */
			{
			if( NS_FAILURE( _ns_config_db_parse_group( db, ptr ), error ) )
				{
				ns_file_destruct( &file );
				return error;
				}
			}
		else/* It should be a key-value pair. */
			{
			if( NS_FAILURE( _ns_config_db_parse_key( db, ptr, strings ), error ) )
				{
				ns_file_destruct( &file );
				return error;
				}
			}
		}


	ns_file_destruct( &file );
	return ns_no_error();
	}


NsError ns_config_db_read( NsConfigDb *db, const nschar *path, nssize *lines )
	{
	ns_assert( NULL != db );
	ns_assert( NULL != path );

	return _ns_config_db_do_read( db, path, lines, NS_FALSE );
	}


NS_PRIVATE nsboolean _ns_config_db_check_read_group
	(
	NsConfigDb     *db,
	const nschar   *read_group,
	nsboolean      *found,
	const nschar  **prev_group,
	const nschar  **curr_group
	)
	{
	if( ns_hash_list_is_empty( &db->groups ) )
		return NS_FALSE;

	*curr_group =
		ns_config_db_base_name(
			ns_hash_list_rev_iter_get_object( ns_hash_list_rev_begin( &db->groups ) )
			);

	/* If the previous group is the one we're looking for... Note that the keys
		for the previous group will have been read by the time we get to the current
		group. */
	if( NULL != *prev_group )
		{
		if( ns_ascii_streq( *prev_group, read_group ) )
			{
			/* If the 'read_group' is the last group in the file then
				curr_group will be equal to prev_group. */
			if( ! ns_ascii_streq( *curr_group, *prev_group ) )
				ns_config_db_remove_group( db, *curr_group );

			if( NULL != found )
				*found = NS_TRUE;

			return NS_TRUE;
			}
		else
			ns_config_db_remove_group( db, *prev_group );
		}

	*prev_group = *curr_group;

	return NS_FALSE;
	}


NS_PRIVATE NsError _ns_config_db_do_read_group
	(
	NsConfigDb    *db,
	const nschar  *path,
	const nschar  *read_group,
	nsboolean     *found,
	nssize        *lines
	)
	{
	NsFile         file;
	nsboolean      eof;
	nschar        *ptr;
	const nschar  *prev_group, *curr_group;
	NsError        error;
	nschar         buffer[ _NS_CONFIG_DB_READ_BUFFER_SIZE ];


	prev_group = NULL;

	if( NULL != found )
		*found = NS_FALSE;

	ns_file_construct( &file );

	if( NS_FAILURE( ns_file_open( &file, path, NS_FILE_MODE_READ ), error ) )
		{
		ns_file_destruct( &file );
		return error;
		}

	ns_config_db_clear( db );

	eof = NS_FALSE;

	if( NULL != lines )
		*lines = 0;

	while( ! eof )
		{
		*buffer = NS_ASCII_NULL;

		if( NS_FAILURE( ns_file_readline(
								&file,
								buffer,
								_NS_CONFIG_DB_READ_BUFFER_SIZE,
								&eof
								),
								error ) )
			{
			ns_file_destruct( &file );
			return error;
			}

		if( NULL != lines )
			*lines = *lines + 1;

		ptr = buffer;

		/* Skip leading white-space. */
		while( ns_ascii_isspace( *ptr ) )
			++ptr;

		/* Skip empty lines. */
		if( NS_ASCII_NULL == *ptr )
			continue;

		if( '#' == *ptr )/* Its a comment. */
			{
			if( NS_FAILURE( _ns_config_db_parse_comment( db, ptr ), error ) )
				{
				ns_file_destruct( &file );
				return error;
				}
			}
		else if( '[' == *ptr )/* Its a group. */
			{
			if( NS_FAILURE( _ns_config_db_parse_group( db, ptr ), error ) )
				{
				ns_file_destruct( &file );
				return error;
				}

			if( _ns_config_db_check_read_group( db, read_group, found, &prev_group, &curr_group ) )
				break;
			}
		else/* It should be a key-value pair. */
			{
			if( NS_FAILURE( _ns_config_db_parse_key( db, ptr, NS_FALSE ), error ) )
				{
				ns_file_destruct( &file );
				return error;
				}
			}
		}

	ns_file_destruct( &file );

	_ns_config_db_check_read_group( db, read_group, found, &prev_group, &curr_group );

	return ns_no_error();
	}


NsError ns_config_db_read_group
	(
	NsConfigDb    *db,
	const nschar  *path,
	const nschar  *group,
	nsboolean     *found,
	nssize        *lines
	)
	{
	ns_assert( NULL != db );
	ns_assert( NULL != path );
	ns_assert( NULL != group );

	return _ns_config_db_do_read_group( db, path, group, found, lines );
	}


NsError ns_config_db_read_strings( NsConfigDb *db, const nschar *path, nssize *lines )
	{
	ns_assert( NULL != db );
	ns_assert( NULL != path );

	return _ns_config_db_do_read( db, path, lines, NS_TRUE );
	}


NsError ns_config_db_read_groups( NsConfigDb *db, const nschar *path, nssize *lines )
	{
	NsFile         file;
	nsboolean      eof;
	nschar        *ptr;
	const nschar  *group;
	NsString       string;
	NsError        error;
	nschar         buffer[ _NS_CONFIG_DB_READ_BUFFER_SIZE ];


	ns_assert( NULL != db );
	ns_assert( NULL != path );

	error = ns_no_error();

	ns_string_construct( &string );
	ns_file_construct( &file );

	if( NS_FAILURE( ns_file_open( &file, path, NS_FILE_MODE_READ ), error ) )
		goto _NS_CONFIG_DB_READ_GROUPS_EXIT;

	ns_config_db_clear( db );

	eof = NS_FALSE;

	if( NULL != lines )
		*lines = 0;

	while( ! eof )
		{
		*buffer = NS_ASCII_NULL;

		if( NS_FAILURE( ns_file_readline(
								&file,
								buffer,
								_NS_CONFIG_DB_READ_BUFFER_SIZE,
								&eof
								),
								error ) )
			goto _NS_CONFIG_DB_READ_GROUPS_EXIT;

		if( NULL != lines )
			*lines = *lines + 1;

		ptr = buffer;

		/* Skip leading white-space. */
		while( ns_ascii_isspace( *ptr ) )
			++ptr;

		if( '[' == *ptr )/* Its a group. */
			{
			/* Add string to current group before creating this new one. */
			if( 0 < ns_config_db_num_groups( db ) && 0 < ns_string_length( &string ) )
				{
				group = ns_config_db_rev_iter_string( ns_config_db_rev_begin_groups( db ) );

				if( NS_FAILURE( ns_config_db_add_string( db, group, "key", ns_string_get( &string ) ), error ) )
					goto _NS_CONFIG_DB_READ_GROUPS_EXIT;
				}

			if( NS_FAILURE( ns_string_empty( &string ), error ) )
				goto _NS_CONFIG_DB_READ_GROUPS_EXIT;

			if( NS_FAILURE( _ns_config_db_parse_group( db, ptr ), error ) )
				goto _NS_CONFIG_DB_READ_GROUPS_EXIT;
			}
		else
			{
			/* NOTE: Dont add any text outside of a group, e.g. comments in the
				beginning of the file. */
			if( 0 < ns_config_db_num_groups( db ) )
				{
				if( NS_FAILURE( ns_string_append( &string, buffer ), error ) )
					goto _NS_CONFIG_DB_READ_GROUPS_EXIT;

				if( NS_FAILURE( ns_string_append_c( &string, NS_UNICHAR_NEWLINE ), error ) )
					goto _NS_CONFIG_DB_READ_GROUPS_EXIT;
				}
			}
		}

	_NS_CONFIG_DB_READ_GROUPS_EXIT:

	/* Add string to last group. */
	if( ! ns_is_error( error ) )
		if( 0 < ns_config_db_num_groups( db ) && 0 < ns_string_length( &string ) )
			{
			group = ns_config_db_rev_iter_string( ns_config_db_rev_begin_groups( db ) );

			if( NS_FAILURE( ns_config_db_add_string( db, group, "key", ns_string_get( &string ) ), error ) )
				goto _NS_CONFIG_DB_READ_GROUPS_EXIT;
			}

	ns_string_destruct( &string );
	ns_file_destruct( &file );

	return error;
	}


NsError ns_config_db_add_all( NsConfigDb *db, const NsConfigDbGroup *group )
	{
	nssize   i;
	NsError  error;


	ns_assert( NULL != db );
	ns_assert( NULL != group );
	ns_assert( NULL != group->name );
	ns_assert( ! ns_config_db_has_group( db, group->name ) );
	ns_assert( NULL != group->keys );
	ns_assert( 0 < group->size );

	if( NS_FAILURE( ns_config_db_add_group( db, group->name ), error ) )
		return error;

	for( i = 0; i < group->size; ++i )
		{
		ns_assert( NULL != group->keys[i].name );
		ns_assert( ! ns_config_db_has_key( db, group->name, group->keys[i].name ) );

		switch( group->keys[i].type )
			{
			case NS_CONFIG_DB_VALUE_STRING:
				if( NS_FAILURE( ns_config_db_add_string(
										db,
										group->name,
										group->keys[i].name,
										group->keys[i].value.as_string
										),
										error ) )
					return error;
				break;

			case NS_CONFIG_DB_VALUE_INT:
				if( NS_FAILURE( ns_config_db_add_int(
										db,
										group->name,
										group->keys[i].name,
										group->keys[i].value.as_int
										),
										error ) )
					return error;
				break;

			case NS_CONFIG_DB_VALUE_BOOLEAN:
				if( NS_FAILURE( ns_config_db_add_boolean(
										db,
										group->name,
										group->keys[i].name,
										group->keys[i].value.as_boolean
										),
										error ) )
					return error;
				break;

			case NS_CONFIG_DB_VALUE_DOUBLE:
				if( NS_FAILURE( ns_config_db_add_double(
										db,
										group->name,
										group->keys[i].name,
										group->keys[i].value.as_double
										),
										error ) )
					return error;
				break;

			default:
				ns_assert_not_reached();
			}
		}

	return ns_no_error();
	}


void ns_config_db_get_all( const NsConfigDb *db, NsConfigDbGroup *group )
	{
	nssize i;

	ns_assert( NULL != db );
	ns_assert( NULL != group );
	ns_assert( NULL != group->name );
	ns_assert( ns_config_db_has_group( db, group->name ) );
	ns_assert( NULL != group->keys );
	ns_assert( 0 < group->size );

	for( i = 0; i < group->size; ++i )
		{
		ns_assert( NULL != group->keys[i].name );
		ns_assert( ns_config_db_has_key( db, group->name, group->keys[i].name ) );

		switch( group->keys[i].type )
			{
			case NS_CONFIG_DB_VALUE_STRING:
				group->keys[i].value.as_string =
					ns_config_db_get_string(
						db,
						group->name,
						group->keys[i].name
						);
				break;

			case NS_CONFIG_DB_VALUE_INT:
				group->keys[i].value.as_int =
					ns_config_db_get_int(
						db,
						group->name,
						group->keys[i].name
						);
				break;

			case NS_CONFIG_DB_VALUE_BOOLEAN:
				group->keys[i].value.as_boolean =
					ns_config_db_get_boolean(
						db,
						group->name,
						group->keys[i].name
						);
				break;

			case NS_CONFIG_DB_VALUE_DOUBLE:
				group->keys[i].value.as_double =
					ns_config_db_get_double(
						db,
						group->name,
						group->keys[i].name
						);
				break;

			default:
				ns_assert_not_reached();
			}
		}
	}


void ns_config_db_set_all( NsConfigDb *db, const NsConfigDbGroup *group )
	{
	nssize i;

	ns_assert( NULL != db );
	ns_assert( NULL != group );
	ns_assert( NULL != group->name );
	ns_assert( ns_config_db_has_group( db, group->name ) );
	ns_assert( NULL != group->keys );
	ns_assert( 0 < group->size );

	for( i = 0; i < group->size; ++i )
		{
		ns_assert( NULL != group->keys[i].name );
		ns_assert( ns_config_db_has_key( db, group->name, group->keys[i].name ) );

		switch( group->keys[i].type )
			{
			case NS_CONFIG_DB_VALUE_STRING:
				ns_config_db_set_string(
					db,
					group->name,
					group->keys[i].name,
					group->keys[i].value.as_string
					);
				break;

			case NS_CONFIG_DB_VALUE_INT:
				ns_config_db_set_int(
					db,
					group->name,
					group->keys[i].name,
					group->keys[i].value.as_int
					);
				break;

			case NS_CONFIG_DB_VALUE_BOOLEAN:
				ns_config_db_set_boolean(
					db,
					group->name,
					group->keys[i].name,
					group->keys[i].value.as_boolean
					);
				break;

			case NS_CONFIG_DB_VALUE_DOUBLE:
				ns_config_db_set_double(
					db,
					group->name,
					group->keys[i].name,
					group->keys[i].value.as_double
					);
				break;

			default:
				ns_assert_not_reached();
			}
		}
	}

