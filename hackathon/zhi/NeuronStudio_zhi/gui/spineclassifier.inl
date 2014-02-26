
#include "tabcontrol.h"


static HWND    ____spine_classifier_dialog_owner;
NsTabControl   ____spine_classifier_dialog_tc;


NsSettings  ____spine_classifier_default_current;
NsSettings  ____spine_classifier_default_last_applied;


NsSpinesClassifier*  ____spine_classifier_ann_current;
nsboolean            ____spine_classifier_ann_enabled;
nschar               ____spine_classifier_ann_name[ 256 ];
nschar               ____spine_classifier_ann_class_names[ 512 ];
NsSpinesPlugins      ____spine_classifier_ann_plugins;
nssize               ____spine_classifier_ann_curr_datum;
nssize               ____spine_classifier_ann_num_datums;
nssize               ____spine_classifier_ann_num_output;
fann_type           *____spine_classifier_ann_output;
MemoryGraphics       ____spine_classifier_ann_graphics;
nsfloat              ____spine_classifier_ann_field_size;


NS_PRIVATE nsboolean _spine_classifier_default_confirm_retyping( NsSettings *current, NsSettings *last_applied )
	{
	nsboolean retype = NS_FALSE;

	if( ! NS_FLOAT_EQUAL(
				ns_settings_get_classify_neck_ratio( current ),
			   ns_settings_get_classify_neck_ratio( last_applied ) ) )
		retype = NS_TRUE;

	if( ! NS_FLOAT_EQUAL(
				ns_settings_get_classify_thin_ratio( current ),
			   ns_settings_get_classify_thin_ratio( last_applied ) ) )
		retype = NS_TRUE;

	if( ! NS_FLOAT_EQUAL(
				ns_settings_get_classify_mushroom_size( current ),
			   ns_settings_get_classify_mushroom_size( last_applied ) ) )
		retype = NS_TRUE;

	if( retype )
		_write_settings_ex( current );

	return retype;
	}


NS_PRIVATE void _spine_classifier_default_show( HWND dlg )
	{
	nschar number[64];

	ns_snprint( number, NS_ARRAY_LENGTH( number ), "%.3f", ns_settings_get_classify_neck_ratio( &____spine_classifier_default_current ) );
	SetDlgItemText( dlg, IDE_ADV_SET_NECK_RATIO, number );

	ns_snprint( number, NS_ARRAY_LENGTH( number ), "%.3f", ns_settings_get_classify_thin_ratio( &____spine_classifier_default_current ) );
	SetDlgItemText( dlg, IDE_ADV_SET_THIN_RATIO, number );

	ns_snprint( number, NS_ARRAY_LENGTH( number ), "%.3f", ns_settings_get_classify_mushroom_size( &____spine_classifier_default_current ) );
	SetDlgItemText( dlg, IDE_ADV_SET_MUSHROOM_SIZE, number );
	}


NS_PRIVATE void _spine_classifier_default_obtain( HWND dlg )
	{
	nsdouble  value;
	nschar    number[64];

	GetDlgItemText( dlg, IDE_ADV_SET_NECK_RATIO, number, sizeof( number ) - 1 );
	value = ns_atod( number );
	ns_settings_set_classify_neck_ratio( &____spine_classifier_default_current, value );

	GetDlgItemText( dlg, IDE_ADV_SET_THIN_RATIO, number, sizeof( number ) - 1 );
	value = ns_atod( number );
	ns_settings_set_classify_thin_ratio( &____spine_classifier_default_current, value );

	GetDlgItemText( dlg, IDE_ADV_SET_MUSHROOM_SIZE, number, sizeof( number ) - 1 );
	value = ns_atod( number );
	ns_settings_set_classify_mushroom_size( &____spine_classifier_default_current, value );
	}


NS_PRIVATE void _spine_classifier_default_enable( HWND dlg, nsint b )
	{
	EnableWindow( GetDlgItem( dlg, IDE_ADV_SET_NECK_RATIO ), b );
	EnableWindow( GetDlgItem( dlg, IDE_ADV_SET_THIN_RATIO ), b );
	EnableWindow( GetDlgItem( dlg, IDE_ADV_SET_MUSHROOM_SIZE ), b );
	EnableWindow( GetDlgItem( dlg, IDB_ADV_SET_APPLY ), b );
	EnableWindow( GetDlgItem( dlg, IDB_ADV_SET_RESET_DEFAULTS ), b );
	}


INT_PTR CALLBACK _spine_classifier_default_dialog_proc( HWND dlg, UINT msg, WPARAM wparam, LPARAM lparam )
   {
   switch( msg )
      {
		case WM_COMMAND:
			switch( LOWORD( wparam ) )
				{
				case IDB_ADV_SET_APPLY:
					_spine_classifier_default_obtain( dlg );

					if( _spine_classifier_default_confirm_retyping( &____spine_classifier_default_current, &____spine_classifier_default_last_applied ) )
						{
						ns_model_spines_retype_by_default_classifier(
							GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ),
							&____spine_classifier_default_current
							);

						____spine_classifier_default_last_applied = ____spine_classifier_default_current;

						____redraw_all();
						}
					break;

				case IDB_ADV_SET_RESET_DEFAULTS:
					ns_settings_set_classify_neck_ratio( &____spine_classifier_default_current, NS_CLASSIFY_NECK_RATIO_DEFAULT );
					ns_settings_set_classify_thin_ratio( &____spine_classifier_default_current, NS_CLASSIFY_THIN_RATIO_DEFAULT );
					ns_settings_set_classify_mushroom_size( &____spine_classifier_default_current, NS_CLASSIFY_MUSHROOM_SIZE_DEFAULT );

					_spine_classifier_default_show( dlg );
					break;
				}
			return TRUE;

		case WM_INITDIALOG:
			SetFocus( GetDlgItem( dlg, IDE_ADV_SET_NECK_RATIO ) );

			if( ____spine_classifier_ann_enabled )
				_spine_classifier_default_enable( dlg, ! ____spine_classifier_ann_enabled );

			_spine_classifier_default_show( dlg );
			return FALSE;

		case WM_DESTROY:
			_spine_classifier_default_obtain( dlg );
			return TRUE;
		}

	return FALSE;
	}


NS_PRIVATE void _spine_classifier_ann_enable( HWND dlg, nsint b )
	{
	EnableWindow( GetDlgItem( dlg, IDC_SC_ANN_PLUGINS ), b );
	EnableWindow( GetDlgItem( dlg, IDB_SC_ANN_NEW ), b );
	EnableWindow( GetDlgItem( dlg, IDB_SC_ANN_DELETE ), b );
	EnableWindow( GetDlgItem( dlg, IDB_SC_ANN_EDIT ), b );
	EnableWindow( GetDlgItem( dlg, IDG_SC_ANN_CURRENT ), b );
	EnableWindow( GetDlgItem( dlg, IDS_SC_ANN_NAME ), b );
	EnableWindow( GetDlgItem( dlg, IDS_SC_ANN_CLASSES ), b );
	EnableWindow( GetDlgItem( dlg, IDE_SC_ANN_NAME ), b );
	EnableWindow( GetDlgItem( dlg, IDE_SC_ANN_CLASSES ), b );
	EnableWindow( GetDlgItem( dlg, IDS_SC_ANN_EXAMPLES ), b );
	EnableWindow( GetDlgItem( dlg, IDE_SC_ANN_EXAMPLES ), b );
	}


INT_PTR CALLBACK _spine_classifier_ann_new_dlg_proc( HWND dlg, UINT msg, WPARAM wparam, LPARAM lparam )
   {
	switch( msg )
		{
		case WM_COMMAND:
			switch( LOWORD( wparam ) )
				{
				case IDOK:
					GetDlgItemText(
						dlg,
						IDE_SPINE_CLASSIFIER_NAME,
						____spine_classifier_ann_name,
						NS_ARRAY_LENGTH( ____spine_classifier_ann_name )
						);

					GetDlgItemText(
						dlg,
						IDE_SPINE_CLASS_NAMES,
						____spine_classifier_ann_class_names,
						NS_ARRAY_LENGTH( ____spine_classifier_ann_class_names )
						);

					{
					nsenum ret = ns_spines_classifier_check_parse_classes( ____spine_classifier_ann_class_names );

					if( NS_SPINES_CLASSIFIER_CHECK_PARSE_EMPTY == ret )
						MessageBox( dlg, "Please specify at least 1 valid class name.", "NeuronStudio", MB_OK | MB_ICONERROR );
					else if( NS_SPINES_CLASSIFIER_CHECK_PARSE_DUPLICATE == ret )
						MessageBox( dlg, "Duplicate class names found. Please make sure\neach class name is unique.", "NeuronStudio", MB_OK | MB_ICONERROR );
					else
						{
						nsboolean  ok;
						NsError    error;

						if( NS_FAILURE(
								ns_spines_plugins_check_name( &____spine_classifier_ann_plugins, ____spine_classifier_ann_name, &ok ),
								error ) )
							MessageBox( dlg, ns_error_code_to_string( error ), "NeuronStudio", MB_OK );
	
						if( ! ok )
							{
							nschar msg[256];

							ns_snprint(
								msg,
								NS_ARRAY_LENGTH( msg ),
								"A classifier named " NS_FMT_STRING_DOUBLE_QUOTED " already exists.\nPlease try a different name.",
								____spine_classifier_ann_name
								);

							MessageBox( dlg, msg, "NeuronStudio", MB_OK );
							}
						else
							EndDialog( dlg, 1 );
						}
					}
					return TRUE;

				case IDCANCEL:
					EndDialog( dlg, 0 );
					return TRUE;
				}
			return FALSE;

		case WM_INITDIALOG:
			CenterWindow( dlg, s_MainWindow.hWnd );
			SetFocus( GetDlgItem( dlg, IDE_SPINE_CLASSIFIER_NAME ) );
			return FALSE;

      case WM_CLOSE:
         EndDialog( dlg, 0 );
         return TRUE;
		}

	return FALSE;
	}


NS_PRIVATE void _spine_classifier_ann_show( HWND dlg )
	{
	if( NULL != ____spine_classifier_ann_current )
		{
		NsString  string;
		nssize    num_datums;


		SetDlgItemText( dlg, IDE_SC_ANN_NAME, ns_spines_classifier_get_name( ____spine_classifier_ann_current ) );

		ns_string_construct( &string );
		/*error*/ns_spines_classifier_classes_string( ____spine_classifier_ann_current, &string );

		SetDlgItemText( dlg, IDE_SC_ANN_CLASSES, ns_string_get( &string ) );

		num_datums = ns_spines_classifier_num_datums( ____spine_classifier_ann_current );
		//ns_string_format( &string, NS_FMT_ULONG " training spine" NS_FMT_STRING, num_datums, 1 != num_datums ? "s" : "" );
		ns_string_format( &string, NS_FMT_ULONG, num_datums );

		SetDlgItemText( dlg, IDE_SC_ANN_EXAMPLES, ns_string_get( &string ) );
		
		ns_string_destruct( &string );
		}
	else
		{
		SetDlgItemText( dlg, IDE_SC_ANN_NAME, "" );
		SetDlgItemText( dlg, IDE_SC_ANN_CLASSES, "" );
		SetDlgItemText( dlg, IDE_SC_ANN_EXAMPLES, "" );
		}
	}


NS_PRIVATE void _spine_classifier_ann_on_new( HWND dlg )
	{
	nsspinesplugin  sp;
	HWND            list;
	NsError         error;


	if( ( nsint )DialogBox( g_Instance, "NEW_SPINE_CLASSIFIER", dlg, _spine_classifier_ann_new_dlg_proc ) )
		{
		//ns_println( "Name    : " NS_FMT_STRING, ____spine_classifier_ann_name );
		//ns_spines_classifier_print_parse_classes( ____spine_classifier_ann_class_names );

		if( ____spine_classifier_ann_current != ns_spines_classifier_get() )
			ns_spines_classifier_delete_ex( &____spine_classifier_ann_current );

		if( NS_FAILURE(
				ns_spines_classifier_new_by_classes_ex(
					&____spine_classifier_ann_current,
					____spine_classifier_ann_name,
					____spine_classifier_ann_class_names
					),
				error ) )
			MessageBox( dlg, ns_error_code_to_string( error ), "NeuronStudio", MB_OK );

		if( NS_FAILURE(
				ns_spines_plugins_add( &____spine_classifier_ann_plugins, ____spine_classifier_ann_current, NS_TRUE, &sp ),
				error ) )
			MessageBox( dlg, ns_error_code_to_string( error ), "NeuronStudio", MB_OK );

		if( NS_FAILURE(
				ns_spines_classifier_set_file(
					____spine_classifier_ann_current,
					ns_spines_plugins_make_path( &____spine_classifier_ann_plugins, sp )
					),
				error ) )
			MessageBox( dlg, ns_error_code_to_string( error ), "NeuronStudio", MB_OK );	

		list = GetDlgItem( dlg, IDC_SC_ANN_PLUGINS );

      SendMessage( list, CB_ADDSTRING, 0, ( LPARAM )ns_spines_classifier_get_name( ____spine_classifier_ann_current ) );
		SendMessage( list, CB_SELECTSTRING, -1, ( LPARAM )ns_spines_classifier_get_name( ____spine_classifier_ann_current ) );

		_spine_classifier_ann_show( dlg );
		}
	}


NS_PRIVATE void _spine_classifier_ann_on_delete( HWND dlg )
	{
	nsspinesplugin   sp;
	nsint            index;
	const nschar    *name;
	HWND             list;
	nschar           msg[256];


	if( NULL == ____spine_classifier_ann_current )
		return;

	name = ns_spines_classifier_get_name( ____spine_classifier_ann_current );

	if( ____spine_classifier_ann_current == ns_spines_classifier_get() )
		{
		ns_snprint(
			msg,
			sizeof( msg ),
			"Can't delete classifier " NS_FMT_STRING_DOUBLE_QUOTED " since it is currently active.",
			name
			);

		MessageBox( dlg, msg, "NeuronStudio", MB_OK | MB_ICONEXCLAMATION );

		return;
		}

	ns_snprint(
		msg,
		sizeof( msg ),
		"Are you sure you want to delete classifier " NS_FMT_STRING_DOUBLE_QUOTED "?",
		name
		);

	if( IDNO == MessageBox( dlg, msg, "NeuronStudio", MB_YESNO | MB_ICONQUESTION ) )
		return;

	sp = ns_spines_plugins_find( &____spine_classifier_ann_plugins, name );
	ns_assert( ns_spines_plugin_not_equal( sp, ns_spines_plugins_end( &____spine_classifier_ann_plugins ) ) );

	list  = GetDlgItem( dlg, IDC_SC_ANN_PLUGINS );
	index = ( nsint )SendMessage( list, CB_FINDSTRING, -1, ( LPARAM )name );

	if( CB_ERR != index )
		SendMessage( list, CB_DELETESTRING, ( WPARAM )index, 0 );

	ns_spines_classifier_delete_ex( &____spine_classifier_ann_current );

	/* NOTE: Have to delete the plug-in entry AFTER the classifier is deleted since the
		classifier saves itself during the delete function. */
	ns_spines_plugins_remove_and_delete( &____spine_classifier_ann_plugins, sp );

	/* Select new classifier. */
	_spine_classifier_ann_show( dlg );
	}


/*
NS_PRIVATE void _spine_classifier_ann_on_load( HWND dlg )
	{
   OPENFILENAME  ofn;
	nschar        path[ NS_PATH_SIZE ] = { '\0' };
	NsError       error;


   ns_memzero( &ofn, sizeof( OPENFILENAME ) );

   ofn.lStructSize = sizeof( OPENFILENAME );
   ofn.hwndOwner   = dlg;
   ofn.lpstrFile   = path;
	ofn.lpstrTitle  = "Load Spine Classifier";
   ofn.nMaxFile    = NS_PATH_SIZE;
   ofn.lpstrFilter = "NeuronStudio Spine Classifier (*.nssc)\0*.nssc\0\0\0";
   ofn.lpstrDefExt = ".nssc";
   ofn.Flags       = OFN_READONLY | OFN_PATHMUSTEXIST | OFN_EXPLORER;

   if( GetOpenFileName( &ofn ) )
		{
		if( ____spine_classifier_ann_current != ns_spines_classifier_get() )
			ns_spines_classifier_delete_ex( &____spine_classifier_ann_current );

		if( NS_FAILURE( ns_spines_classifier_new_by_file_ex( &____spine_classifier_ann_current, path ), error ) )
			MessageBox( dlg, ns_error_code_to_string( error ), "NeuronStudio", MB_OK );

		_spine_classifier_ann_show( dlg );
		}
	}
*/


void _spine_classifier_ann_init_plugins( HWND dlg )
	{
	nsspinesplugin  curr, end;
	HWND            list;
	NsError         error;


   list = GetDlgItem( dlg, IDC_SC_ANN_PLUGINS );

	/* Read the plugin (names) from file. */
	if( NS_FAILURE( ns_spines_plugins_read( &____spine_classifier_ann_plugins, ____classifiers_dir ), error ) )
		MessageBox( dlg, ns_error_code_to_string( error ), "NeuronStudio", MB_OK );

	/* If there is not a current classifier, select the first plugin as the current
		one, that is if there is one. */
	if( NULL == ____spine_classifier_ann_current )
		if( ! ns_spines_plugins_is_empty( &____spine_classifier_ann_plugins ) )
			if( NS_FAILURE(
					ns_spines_classifier_new_by_file_ex(
						&____spine_classifier_ann_current,
						ns_spines_plugins_make_path( &____spine_classifier_ann_plugins, ns_spines_plugins_begin( &____spine_classifier_ann_plugins ) )
						),
					error ) )
				MessageBox( dlg, ns_error_code_to_string( error ), "NeuronStudio", MB_OK );

	/* Add all the plugins to the list. */

	curr = ns_spines_plugins_begin( &____spine_classifier_ann_plugins );
	end  = ns_spines_plugins_end( &____spine_classifier_ann_plugins );

	for( ; ns_spines_plugin_not_equal( curr, end ); curr = ns_spines_plugin_next( curr ) )
      SendMessage( list, CB_ADDSTRING, 0, ( LPARAM )ns_spines_plugin_name( curr ) );

	/* Select the current one (if there is one) in the plugin list. It should be there. */

	if( NULL != ____spine_classifier_ann_current )
		SendMessage( list, CB_SELECTSTRING, -1, ( LPARAM )ns_spines_classifier_get_name( ____spine_classifier_ann_current ) );
	}


void _spine_classifier_ann_on_select_plugin( HWND dlg, nssize at )
	{
	nsspinesplugin  sp;
	NsError         error;


	/* Try to find the one the user selected in the plugin list. */
	sp = ns_spines_plugins_at( &____spine_classifier_ann_plugins, at );

	if( ns_spines_plugin_not_equal( sp, ns_spines_plugins_end( &____spine_classifier_ann_plugins ) ) )
		{
		/* If the user selects the current one, then do nothing. */
		if( NULL != ____spine_classifier_ann_current )
			if( ns_ascii_streq(
					ns_spines_classifier_get_name( ____spine_classifier_ann_current ),
					ns_spines_plugin_name( sp )
					) )
				return;

		if( ____spine_classifier_ann_current != ns_spines_classifier_get() )
			ns_spines_classifier_delete_ex( &____spine_classifier_ann_current );

		if( NS_FAILURE(
				ns_spines_classifier_new_by_file_ex(
					&____spine_classifier_ann_current,
					ns_spines_plugins_make_path( &____spine_classifier_ann_plugins, sp )
					),
				error ) )
			MessageBox( dlg, ns_error_code_to_string( error ), "NeuronStudio", MB_OK );

		_spine_classifier_ann_show( dlg );
		}
	}


#include <ext/drawspine.inl>

void _spine_classifier_ann_edit_on_paint( HWND dlg )
	{
	HWND          canvas;
   HDC           hdc;
   RECT          client;
	nsint         type;
	nsspineclass  klass;
	fann_type     input[ NS_SPINES_CLASSIFIER_NUM_INPUT ];
	NsError       error;


	if( NULL == ____spine_classifier_ann_graphics.hDC )
		return;

	canvas = GetDlgItem( dlg, IDS_SC_EDITOR_CANVAS );

   GetClientRect( canvas, &client );

	if( NS_FAILURE(
			ns_spines_classifier_get_datum(
				____spine_classifier_ann_current,
				____spine_classifier_ann_curr_datum - 1,
				input,
				NS_SPINES_CLASSIFIER_NUM_INPUT,
				____spine_classifier_ann_output,
				____spine_classifier_ann_num_output
				),
			error ) )
		return;

	type = ns_spines_classifier_type(
				____spine_classifier_ann_current,
				____spine_classifier_ann_output,
				____spine_classifier_ann_num_output
				);

	klass = ns_spines_classifier_find_by_value( ____spine_classifier_ann_current, type );

	ns_assert(
		ns_spine_class_not_equal(
			klass,
			ns_spines_classifier_end( ____spine_classifier_ann_current )
			)
		);

	DrawSpine(
		____spine_classifier_ann_graphics.hDC,
		ns_spine_class_name( klass ),
		( nsfloat )input[ NS_SPINES_CLASSIFIER_HEAD_DIAMETER ],
		( nsfloat )input[ NS_SPINES_CLASSIFIER_NECK_DIAMETER ],
		( nsfloat )input[ NS_SPINES_CLASSIFIER_BASE_WIDTH ],
		( nsfloat )input[ NS_SPINES_CLASSIFIER_HEIGHT ],
		( nsfloat )input[ NS_SPINES_CLASSIFIER_XYPLANE_ANGLE ],
		client.right - client.left,
		client.bottom - client.top,
		____spine_classifier_ann_field_size
		);

	hdc = GetDC( canvas );

	BitBlt(
		hdc,
		0, 0, 
		client.right - client.left,
		client.bottom - client.top,
		____spine_classifier_ann_graphics.hDC,
		0, 0,
		SRCCOPY 
		);

	ReleaseDC( canvas, hdc );
	}


void _spine_classifier_ann_edit_on_select( HWND dlg )
	{
	HWND          ctrl;
	fann_type     input[ NS_SPINES_CLASSIFIER_NUM_INPUT ];
	nschar        text[64];
	nsint         type;
	nsspineclass  klass;
	nsint         index;
	NsError       error;


	if( NS_FAILURE(
			ns_spines_classifier_get_datum(
				____spine_classifier_ann_current,
				____spine_classifier_ann_curr_datum - 1,
				input,
				NS_SPINES_CLASSIFIER_NUM_INPUT,
				____spine_classifier_ann_output,
				____spine_classifier_ann_num_output
				),
			error ) )
		return;

	type = ns_spines_classifier_type(
				____spine_classifier_ann_current,
				____spine_classifier_ann_output,
				____spine_classifier_ann_num_output
				);

	ns_snprint(
		text,
		sizeof( text ),
		NS_FMT_UINT " of " NS_FMT_UINT,
		( nsuint )____spine_classifier_ann_curr_datum,
		( nsuint )____spine_classifier_ann_num_datums
		);

	SetDlgItemText( dlg, IDE_SC_EDITOR_ID, text );

	ns_snprint( text, sizeof( text ), NS_FMT_DOUBLE " \xb5m", ( nsfloat )input[ NS_SPINES_CLASSIFIER_HEAD_DIAMETER ] );
	SetDlgItemText( dlg, IDE_SC_EDITOR_HEAD_DIAM, text );

	ns_snprint( text, sizeof( text ), NS_FMT_DOUBLE " \xb5m", ( nsfloat )input[ NS_SPINES_CLASSIFIER_NECK_DIAMETER ] );
	SetDlgItemText( dlg, IDE_SC_EDITOR_NECK_DIAM, text );

	ns_snprint( text, sizeof( text ), NS_FMT_DOUBLE " \xb5m", ( nsfloat )input[ NS_SPINES_CLASSIFIER_BASE_WIDTH ] );
	SetDlgItemText( dlg, IDE_SC_EDITOR_WIDTH, text );

	ns_snprint( text, sizeof( text ), NS_FMT_DOUBLE " \xb5m", ( nsfloat )input[ NS_SPINES_CLASSIFIER_HEIGHT ] );
	SetDlgItemText( dlg, IDE_SC_EDITOR_HEIGHT, text );

	ns_snprint( text, sizeof( text ), NS_FMT_DOUBLE " \xb0", ( nsfloat )input[ NS_SPINES_CLASSIFIER_XYPLANE_ANGLE ] );
	SetDlgItemText( dlg, IDE_SC_EDITOR_ANGLE, text );

	ctrl  = GetDlgItem( dlg, IDC_SC_EDITOR_CLASSES );
	klass = ns_spines_classifier_find_by_value( ____spine_classifier_ann_current, type );

	if( ns_spine_class_not_equal( klass, ns_spines_classifier_end( ____spine_classifier_ann_current ) ) )
		{
		index = ( nsint )SendMessage( ctrl, CB_FINDSTRING, -1, ( LPARAM )ns_spine_class_name( klass ) );

		if( CB_ERR != index )
			SendMessage( ctrl, CB_SETCURSEL, ( WPARAM )index, 0 );
		}
	}


void _spine_classifier_ann_edit_create_graphics( HWND dlg )
	{
	HWND  canvas;
   HDC   hdc;
   RECT  client;


	canvas = GetDlgItem( dlg, IDS_SC_EDITOR_CANVAS );
	hdc    = GetDC( canvas );

   GetClientRect( canvas, &client );

	CreateMemoryGraphics(
		&____spine_classifier_ann_graphics,
		hdc,
		client.right - client.left,
		client.bottom - client.top
		);

	ReleaseDC( canvas, hdc );
	}


void _spine_classifier_ann_edit_destroy_graphics( HWND dlg )
	{
	NS_USE_VARIABLE( dlg );
	DestroyMemoryGraphics( &____spine_classifier_ann_graphics );
	}


#define _NS_SPINE_CLASSIFIER_ANN_MIN_FIELD_SIZE      1.0f
#define _NS_SPINE_CLASSIFIER_ANN_MAX_FIELD_SIZE      6.0f
#define _NS_SPINE_CLASSIFIER_ANN_DEFAULT_FIELD_SIZE  3.0f
#define _NS_SPINE_CLASSIFIER_ANN_FIELD_SIZE_OFFSET   1.0f


void _spine_classifier_ann_edit_on_next( HWND dlg )
	{
	if( ____spine_classifier_ann_curr_datum < ____spine_classifier_ann_num_datums )
		{
		++____spine_classifier_ann_curr_datum;

		_spine_classifier_ann_edit_on_select( dlg );
		_spine_classifier_ann_edit_on_paint( dlg );
		}
	}


void _spine_classifier_ann_edit_on_prev( HWND dlg )
	{
	if( 1 < ____spine_classifier_ann_curr_datum )
		{
		--____spine_classifier_ann_curr_datum;

		_spine_classifier_ann_edit_on_select( dlg );
		_spine_classifier_ann_edit_on_paint( dlg );
		}
	}


void _spine_classifier_ann_edit_init_classes( HWND dlg )
	{
	HWND          ctrl;
	nsspineclass  curr, end;


	ctrl = GetDlgItem( dlg, IDC_SC_EDITOR_CLASSES );
	curr = ns_spines_classifier_begin( ____spine_classifier_ann_current );
	end  = ns_spines_classifier_end( ____spine_classifier_ann_current );

	for( ; ns_spine_class_not_equal( curr, end ); curr = ns_spine_class_next( curr ) )
		SendMessage( ctrl, CB_ADDSTRING, 0, ( LPARAM )ns_spine_class_name( curr ) );
	}


void _spine_classifier_ann_edit_on_change_class( HWND dlg )
	{
	HWND          ctrl;
	nsint         index;
	nssize        length;
	nsspineclass  klass;
	nschar        name[ 128 ];
	fann_type     input[ NS_SPINES_CLASSIFIER_NUM_INPUT ];
	NsError       error;


	ctrl   = GetDlgItem( dlg, IDC_SC_EDITOR_CLASSES );
	index  = ( nsint )SendMessage( ctrl, CB_GETCURSEL, 0, 0 );
	length = ( nssize )SendMessage( ctrl, CB_GETLBTEXTLEN, ( WPARAM )index, 0 );

	if( length < sizeof( name ) )
		{
		SendMessage( ctrl, CB_GETLBTEXT, ( WPARAM )index, ( LPARAM )name );

		klass = ns_spines_classifier_find_by_name( ____spine_classifier_ann_current, name );

		if( ns_spine_class_not_equal( klass, ns_spines_classifier_end( ____spine_classifier_ann_current ) ) )
			{
			if( NS_FAILURE(
					ns_spines_classifier_get_datum(
						____spine_classifier_ann_current,
						____spine_classifier_ann_curr_datum - 1,
						input,
						NS_SPINES_CLASSIFIER_NUM_INPUT,
						____spine_classifier_ann_output,
						____spine_classifier_ann_num_output
						),
					error ) )
				return;

			if( NS_FAILURE(
					ns_spines_classifier_type_datum(
						____spine_classifier_ann_current,
						____spine_classifier_ann_curr_datum - 1,
						klass,
						____spine_classifier_ann_output,
						____spine_classifier_ann_num_output
						),
					error ) )
				return;

			if( NS_FAILURE(
					ns_spines_classifier_set_datum(
						____spine_classifier_ann_current,
						____spine_classifier_ann_curr_datum - 1,
						input,
						NS_SPINES_CLASSIFIER_NUM_INPUT,
						____spine_classifier_ann_output,
						____spine_classifier_ann_num_output
						),
					error ) )
				return;

			_spine_classifier_ann_edit_on_paint( dlg );
			}
		}
	}


void _spine_classifier_ann_edit_on_zoom( HWND dlg )
	{
	nschar text[64];

	ns_snprint(
		text,
		sizeof( text ),
		"View: " NS_FMT_INT ".0 \xb5m x " NS_FMT_INT ".0 \xb5m",
		( nsint )____spine_classifier_ann_field_size,
		( nsint )____spine_classifier_ann_field_size
		);

	SetDlgItemText( dlg, IDS_SC_EDITOR_ZOOM_LEVEL, text );
	}


void _spine_classifier_ann_edit_on_delete( HWND dlg )
	{
	NsError error;

	if( NS_FAILURE(
			ns_spines_classifier_remove_datum(
				____spine_classifier_ann_current,
				____spine_classifier_ann_curr_datum - 1
				),
			error ) )
		return;

	--____spine_classifier_ann_num_datums;

	if( 0 == ____spine_classifier_ann_num_datums )
		EndDialog( dlg, 0 );
	else
		{
		if( ____spine_classifier_ann_num_datums < ____spine_classifier_ann_curr_datum )
			____spine_classifier_ann_curr_datum = ____spine_classifier_ann_num_datums;
	
		_spine_classifier_ann_edit_on_select( dlg );
		_spine_classifier_ann_edit_on_paint( dlg );
		}
	}


INT_PTR CALLBACK _spine_classifier_ann_edit_dlg_proc( HWND dlg, UINT msg, WPARAM wparam, LPARAM lparam )
   {
	switch( msg )
		{
		case WM_PAINT:
			_spine_classifier_ann_edit_on_paint( dlg );
			return FALSE;

		/*
		case WM_KEYDOWN:
			switch( ( nsint )wparam )
				{
				case VK_LEFT:
					_spine_classifier_ann_edit_on_prev( dlg );
					break;

				case VK_RIGHT:
					_spine_classifier_ann_edit_on_next( dlg );
					break;
				}
			return TRUE;
		*/

		case WM_COMMAND:
			switch( LOWORD( wparam ) )
				{
				case IDB_SC_EDITOR_NEXT:
					_spine_classifier_ann_edit_on_next( dlg );
					break;

				case IDB_SC_EDITOR_PREV:
					_spine_classifier_ann_edit_on_prev( dlg );
					break;

				case IDB_SC_EDITOR_FIRST:
					if( 1 != ____spine_classifier_ann_curr_datum )
						{
						____spine_classifier_ann_curr_datum = 1;

						_spine_classifier_ann_edit_on_select( dlg );
						_spine_classifier_ann_edit_on_paint( dlg );
						}
					break;

				case IDB_SC_EDITOR_LAST:
					if( ____spine_classifier_ann_curr_datum != ____spine_classifier_ann_num_datums )
						{
						____spine_classifier_ann_curr_datum = ____spine_classifier_ann_num_datums;

						_spine_classifier_ann_edit_on_select( dlg );
						_spine_classifier_ann_edit_on_paint( dlg );
						}
					break;

				case IDB_SC_EDITOR_ZOOM_IN:
					if( _NS_SPINE_CLASSIFIER_ANN_MIN_FIELD_SIZE < ____spine_classifier_ann_field_size )
						{
						____spine_classifier_ann_field_size -= _NS_SPINE_CLASSIFIER_ANN_FIELD_SIZE_OFFSET;

						/* Clip just to be sure. Probably not necessary. */
						if( ____spine_classifier_ann_field_size < _NS_SPINE_CLASSIFIER_ANN_MIN_FIELD_SIZE )
							____spine_classifier_ann_field_size = _NS_SPINE_CLASSIFIER_ANN_MIN_FIELD_SIZE;

						_spine_classifier_ann_edit_on_zoom( dlg );
						_spine_classifier_ann_edit_on_paint( dlg );
						}
					break;

				case IDB_SC_EDITOR_ZOOM_OUT:
					if( ____spine_classifier_ann_field_size < _NS_SPINE_CLASSIFIER_ANN_MAX_FIELD_SIZE )
						{
						____spine_classifier_ann_field_size += _NS_SPINE_CLASSIFIER_ANN_FIELD_SIZE_OFFSET;
						
						/* Clip just to be sure. Probably not necessary. */
						if( _NS_SPINE_CLASSIFIER_ANN_MAX_FIELD_SIZE < ____spine_classifier_ann_field_size )
							____spine_classifier_ann_field_size = _NS_SPINE_CLASSIFIER_ANN_MAX_FIELD_SIZE;

						_spine_classifier_ann_edit_on_zoom( dlg );
						_spine_classifier_ann_edit_on_paint( dlg );
						}
					break;


				case IDC_SC_EDITOR_CLASSES:
					if( CBN_SELCHANGE == HIWORD( wparam ) )
						_spine_classifier_ann_edit_on_change_class( dlg );
					break;

				case IDB_SC_EDITOR_DELETE:
					_spine_classifier_ann_edit_on_delete( dlg );
					break;
				}
			return TRUE;

		case WM_INITDIALOG:
			_spine_classifier_ann_edit_init_classes( dlg );
			_spine_classifier_ann_edit_create_graphics( dlg );
			_spine_classifier_ann_edit_on_select( dlg );
			_spine_classifier_ann_edit_on_zoom( dlg );
		
			CenterWindow( dlg, s_MainWindow.hWnd );
			return FALSE;

      case WM_CLOSE:
			_spine_classifier_ann_edit_destroy_graphics( dlg );
         EndDialog( dlg, 0 );
         return TRUE;
		}

	return FALSE;
	}


NS_PRIVATE void _spine_classifier_ann_on_edit( HWND dlg )
	{
	if( NULL != ____spine_classifier_ann_current )
		{
		____spine_classifier_ann_num_datums = ns_spines_classifier_num_datums( ____spine_classifier_ann_current );

		if( 0 < ____spine_classifier_ann_num_datums )
			{
			____spine_classifier_ann_field_size = _NS_SPINE_CLASSIFIER_ANN_DEFAULT_FIELD_SIZE;
			____spine_classifier_ann_curr_datum = 1;
			____spine_classifier_ann_num_output = ns_spines_classifier_num_classes( ____spine_classifier_ann_current );

			if( NULL != ( ____spine_classifier_ann_output =
					ns_new_array( fann_type, ____spine_classifier_ann_num_output ) ) )
				DialogBox( g_Instance, "SPINE_CLASSIFIER_EDITOR", dlg, _spine_classifier_ann_edit_dlg_proc );

			ns_free( ____spine_classifier_ann_output );
			}
		}
	}


INT_PTR CALLBACK _spine_classifier_ann_dialog_proc( HWND dlg, UINT msg, WPARAM wparam, LPARAM lparam )
   {
   switch( msg )
      {
		case WM_COMMAND:
			switch( LOWORD( wparam ) )
				{
				case IDC_SC_ANN_ENABLE:
					____spine_classifier_ann_enabled = ! ____spine_classifier_ann_enabled;
					_spine_classifier_ann_enable( dlg, ____spine_classifier_ann_enabled );
					break;

				case IDB_SC_ANN_NEW:
					ns_assert( ____spine_classifier_ann_enabled );
					_spine_classifier_ann_on_new( dlg );
					break;

				case IDB_SC_ANN_DELETE:
					ns_assert( ____spine_classifier_ann_enabled );
					_spine_classifier_ann_on_delete( dlg );
					break;

				/*case IDB_SC_ANN_LOAD:
					ns_assert( ____spine_classifier_ann_enabled );
					_spine_classifier_ann_on_load( dlg );
					break;*/

				case IDB_SC_ANN_EDIT:
					ns_assert( ____spine_classifier_ann_enabled );
					_spine_classifier_ann_on_edit( dlg );
					_spine_classifier_ann_show( dlg );
					break;

				case IDC_SC_ANN_PLUGINS:
					if( CBN_SELCHANGE == HIWORD( wparam ) )
						_spine_classifier_ann_on_select_plugin( 
							dlg,
							( nssize )SendMessage( GetDlgItem( dlg, IDC_SC_ANN_PLUGINS ), CB_GETCURSEL, 0, 0 )
							);
					break;
				}
			return TRUE;

		case WM_INITDIALOG:

			_spine_classifier_ann_init_plugins( dlg );

			SendMessage(
				GetDlgItem( dlg, IDC_SC_ANN_ENABLE ),
				BM_SETCHECK,
				____spine_classifier_ann_enabled ? BST_CHECKED : BST_UNCHECKED,
				0
				);

			_spine_classifier_ann_show( dlg );

			_spine_classifier_ann_enable( dlg, ____spine_classifier_ann_enabled );
			SetFocus( GetDlgItem( dlg, IDC_SC_ANN_ENABLE ) );
			return FALSE;
		}

	return FALSE;
	}


NS_PRIVATE void _spine_classifier_dialog_create_tabs( void )
	{
	ns_tab_control_add(
		&____spine_classifier_dialog_tc,
		"Default",
		"SPINE_CLASSIFIER_DEFAULT",
		_spine_classifier_default_dialog_proc
		);

	ns_tab_control_add(
		&____spine_classifier_dialog_tc,
		"Trainable",
		"SPINE_CLASSIFIER_ANN",
		_spine_classifier_ann_dialog_proc
		);

	ns_tab_control_on_select( &____spine_classifier_dialog_tc );
	}


INT_PTR CALLBACK _spine_classifier_dialog_proc( HWND dlg, UINT msg, WPARAM wparam, LPARAM lparam )
   {
   switch( msg )
      {
      case WM_INITDIALOG:
			{
         CenterWindow( dlg, ____spine_classifier_dialog_owner );

			ns_tab_control_init(
				&____spine_classifier_dialog_tc,
				g_Instance,
				dlg,
				IDT_SPINE_CLASSIFIER,
				2
				);

			_spine_classifier_dialog_create_tabs();
			}
         return FALSE;


		case WM_NOTIFY:
			switch( ( ( LPNMHDR )lparam )->code )
				{
				case TCN_SELCHANGE:
					ns_tab_control_on_select( &____spine_classifier_dialog_tc );
					break;
				} 
			break;

      case WM_COMMAND:
         switch( LOWORD( wparam ) )
            {
            case IDOK:
					if( ____spine_classifier_ann_enabled )
						{
						if( NULL == ____spine_classifier_ann_current ||
							 0 == ns_spines_classifier_num_classes( ____spine_classifier_ann_current ) )
							MessageBox( dlg, "Please create or select a trainable classifier.", "NeuronStudio", MB_OK | MB_ICONINFORMATION );
						else
							EndDialog( dlg, 1 );
						}
					else
						EndDialog( dlg, 1 );
               break;

            case IDCANCEL:
               EndDialog( dlg, 0 );
               break;
				}
         return TRUE;

      case WM_CLOSE:
         EndDialog( dlg, 0 );
         return TRUE;
      }
   
   return FALSE;
   }


NsError _spine_classifier_dialog( HWND owner )
	{
	NsSettings    *spine_classifier_default_current;
	nsboolean      retype;
	nsint          ret;


	retype = NS_FALSE;

   ____spine_classifier_dialog_owner = owner;

	ns_spines_plugins_construct( &____spine_classifier_ann_plugins );

	spine_classifier_default_current          = workspace_settings( s_MainWindow.activeWorkspace );
	____spine_classifier_default_current      = *spine_classifier_default_current;
	____spine_classifier_default_last_applied = *spine_classifier_default_current;

	____spine_classifier_ann_current = ns_spines_classifier_get();
	____spine_classifier_ann_enabled = ( NULL != ____spine_classifier_ann_current );
   
	ret = ( nsint )DialogBox( g_Instance, "SPINE_CLASSIFIER", owner, _spine_classifier_dialog_proc );

   if( 1 == ret )
		{
		if( ! ____spine_classifier_ann_enabled )
			{
			*spine_classifier_default_current = ____spine_classifier_default_current;

			/* User hit OK but the ANN is not enabled, so delete the current one
				if it is not the global one, and then delete the global one. */

			if( ____spine_classifier_ann_current != ns_spines_classifier_get() )
				ns_spines_classifier_delete_ex( &____spine_classifier_ann_current );

			ns_spines_classifier_delete();
			}
		else
			{
			/* User hit OK and the ANN is enabled. If the current one is not the same
				as the global one, delete the global one and then set it to the current
				one. */
			if( ____spine_classifier_ann_current != ns_spines_classifier_get() )
				{
				//const nschar *name1, *name2;

				retype = NS_TRUE;

				/* Small optimization? If they are the same classifier (by name), then we
					DONT need to re-type the spines. */

				//name1 = ( NULL != ____spine_classifier_ann_current ? ns_spines_classifier_get_name( ____spine_classifier_ann_current ) : NULL );
				//name2 = ( NULL != ns_spines_classifier_get() ? ns_spines_classifier_get_name( ns_spines_classifier_get() ) : NULL );

				//if( NULL != name1 && NULL != name2 )
				//	if( ns_ascii_streq( name1, name2 ) )
						//retype = NS_FALSE;

				ns_spines_classifier_delete();
			
				ns_assert( NULL != ____spine_classifier_ann_current );
				ns_spines_classifier_set( ____spine_classifier_ann_current );
				}
			}
		}
	else
		{
		/* User hit the Cancel button, so delete the current ANN only, that is if it is not
			the same as the global one. */

		if( ____spine_classifier_ann_current != ns_spines_classifier_get() )
			ns_spines_classifier_delete_ex( &____spine_classifier_ann_current );
		}

	if( 1 != ret )
		ns_spines_plugins_remove_and_delete_temps( &____spine_classifier_ann_plugins );

	ns_spines_plugins_destruct( &____spine_classifier_ann_plugins );

	if( ! ____spine_classifier_ann_enabled )
		{
		//if( _spine_classifier_default_confirm_retyping( spine_classifier_default_current, &____spine_classifier_default_last_applied ) )
			ns_model_spines_retype_by_default_classifier(
				GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ),
				spine_classifier_default_current
				);
		}
	else if( retype )
		workspace_run_ann_classifier_retyping( owner, s_MainWindow.activeWorkspace );

	return ns_no_error();
	}
