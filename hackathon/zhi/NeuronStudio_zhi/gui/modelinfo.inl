
#include "tabcontrol.h"


NsTabControl  ____model_info_tc;

NsModelInfo  ____model_info;
NsSpinesInfo  ____spines_info;
NsSpinesClassifierInfo ____spines_classifier_info;

NS_PRIVATE nschar ____model_info_buffer[ 512 ];


NS_PRIVATE void _model_info_all_show( HWND hDlg, const NsModel *model )
   {
	NsString s;

	ns_string_construct( &s );
	/*error*/ns_string_format( &s, "Total Dendritic Length = " NS_FMT_DOUBLE, ns_model_total_dendritic_length( model ) );

	SetDlgItemText( hDlg, IDE_MODEL_INFO, ns_string_get( &s ) );

	ns_string_destruct( &s );
   }


INT_PTR CALLBACK _model_info_all_dialog_proc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   switch( uMsg )
      {
      case WM_INITDIALOG:
         _model_info_all_show( hDlg, GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace ) );
         return FALSE;
      }

   return FALSE;
   }

NS_PRIVATE void _model_info_vertices_show( HWND hDlg )
   {
   ns_snprint( ____model_info_buffer, NS_ARRAY_LENGTH( ____model_info_buffer ), NS_FMT_ULONG, ____model_info.num_vertices );
   SetDlgItemText( hDlg, IDS_MODEL_INFO_NUM_VERTICES, ____model_info_buffer );

   ns_snprint( ____model_info_buffer, NS_ARRAY_LENGTH( ____model_info_buffer ), NS_FMT_ULONG, ____model_info.num_origin_vertices );
   SetDlgItemText( hDlg, IDS_MODEL_INFO_NUM_ORIGIN_VERTICES, ____model_info_buffer );

   ns_snprint( ____model_info_buffer, NS_ARRAY_LENGTH( ____model_info_buffer ), NS_FMT_ULONG, ____model_info.num_external_vertices );
   SetDlgItemText( hDlg, IDS_MODEL_INFO_NUM_EXTERNAL_VERTICES, ____model_info_buffer );

   ns_snprint( ____model_info_buffer, NS_ARRAY_LENGTH( ____model_info_buffer ), NS_FMT_ULONG, ____model_info.num_line_vertices );
   SetDlgItemText( hDlg, IDS_MODEL_INFO_NUM_LINE_VERTICES, ____model_info_buffer );

   ns_snprint( ____model_info_buffer, NS_ARRAY_LENGTH( ____model_info_buffer ), NS_FMT_ULONG, ____model_info.num_junction_vertices );
   SetDlgItemText( hDlg, IDS_MODEL_INFO_NUM_JUNCTION_VERTICES, ____model_info_buffer );

   ns_snprint( ____model_info_buffer, NS_ARRAY_LENGTH( ____model_info_buffer ), NS_FMT_ULONG, ____model_info.num_soma_vertices );
   SetDlgItemText( hDlg, IDS_MODEL_INFO_NUM_SOMA_VERTICES, ____model_info_buffer );

   ns_snprint( ____model_info_buffer, NS_ARRAY_LENGTH( ____model_info_buffer ), NS_FMT_ULONG, ____model_info.num_basal_vertices );
   SetDlgItemText( hDlg, IDS_MODEL_INFO_NUM_BASAL_VERTICES, ____model_info_buffer );

   ns_snprint( ____model_info_buffer, NS_ARRAY_LENGTH( ____model_info_buffer ), NS_FMT_ULONG, ____model_info.num_apical_vertices );
   SetDlgItemText( hDlg, IDS_MODEL_INFO_NUM_APICAL_VERTICES, ____model_info_buffer );

   ns_snprint( ____model_info_buffer, NS_ARRAY_LENGTH( ____model_info_buffer ), NS_FMT_ULONG, ____model_info.num_axon_vertices );
   SetDlgItemText( hDlg, IDS_MODEL_INFO_NUM_AXON_VERTICES, ____model_info_buffer );

   ns_snprint( ____model_info_buffer, NS_ARRAY_LENGTH( ____model_info_buffer ), NS_FMT_ULONG, ____model_info.num_unknown_vertices );
   SetDlgItemText( hDlg, IDS_MODEL_INFO_NUM_UNKNOWN_VERTICES, ____model_info_buffer );
   }


NS_PRIVATE void _model_info_edges_show( HWND hDlg )
   {
   ns_snprint( ____model_info_buffer, NS_ARRAY_LENGTH( ____model_info_buffer ), NS_FMT_ULONG, ____model_info.num_edges );
   SetDlgItemText( hDlg, IDS_MODEL_INFO_NUM_EDGES, ____model_info_buffer );
   }


NS_PRIVATE void _model_info_spines_show( HWND hDlg )
   {
   ns_snprint( ____model_info_buffer, NS_ARRAY_LENGTH( ____model_info_buffer ), NS_FMT_ULONG,
      ____spines_info.num_spines );
   SetDlgItemText( hDlg, IDS_MODEL_INFO_NUM_SPINES, ____model_info_buffer );

   ns_snprint( ____model_info_buffer, NS_ARRAY_LENGTH( ____model_info_buffer ), NS_FMT_ULONG,
      ____spines_info.num_mushroom_spines );
   SetDlgItemText( hDlg, IDS_MODEL_INFO_NUM_MUSHROOM_SPINES, ____model_info_buffer );

   ns_snprint( ____model_info_buffer, NS_ARRAY_LENGTH( ____model_info_buffer ), NS_FMT_ULONG,
      ____spines_info.num_thin_spines );
   SetDlgItemText( hDlg, IDS_MODEL_INFO_NUM_THIN_SPINES, ____model_info_buffer );

   ns_snprint( ____model_info_buffer, NS_ARRAY_LENGTH( ____model_info_buffer ), NS_FMT_ULONG,
      ____spines_info.num_stubby_spines );
   SetDlgItemText( hDlg, IDS_MODEL_INFO_NUM_STUBBY_SPINES, ____model_info_buffer );

   ns_snprint( ____model_info_buffer, NS_ARRAY_LENGTH( ____model_info_buffer ), NS_FMT_ULONG,
      ____spines_info.num_other_spines );
   SetDlgItemText( hDlg, IDS_MODEL_INFO_NUM_UNKNOWN_SPINES, ____model_info_buffer );
   }


NS_PRIVATE void _model_info_spines_ann_show( HWND hdlg )
	{
	const NsSpinesClassifier  *classifier;
	nsspineclass               curr, end;
	nsint                      value;
	nssize                     count;
	NsString                   s;           


	ns_string_construct( &s );

	classifier = ns_spines_classifier_get();
	ns_assert( NULL != classifier );

	curr = ns_spines_classifier_begin( classifier );
	end  = ns_spines_classifier_end( classifier );

	for( ; ns_spine_class_not_equal( curr, end ); curr = ns_spine_class_next( curr ) )
		{
		count = 0;
		value = ns_spine_class_value( curr );

		ns_assert( 0 < value );
		ns_assert( value <= ( nsint )____spines_classifier_info.num_counts );

		count = ____spines_classifier_info.counts[ value - 1 ];

		ns_snprint(
			____model_info_buffer,
			NS_ARRAY_LENGTH( ____model_info_buffer ),
			NS_FMT_STRING ": " NS_FMT_ULONG "\r\n",
			ns_spine_class_name( curr ),
			count
			);

		/*error*/ns_string_append( &s, ____model_info_buffer );
		}

	ns_snprint(
		____model_info_buffer,
		NS_ARRAY_LENGTH( ____model_info_buffer ),
		"Total: " NS_FMT_ULONG,
		____spines_classifier_info.num_spines
		);

	/*error*/ns_string_append( &s, ____model_info_buffer );

	SetDlgItemText( hdlg, IDE_MODEL_INFO_SPINES_ANN, ns_string_get( &s ) );
	ns_string_destruct( &s );
	}


INT_PTR CALLBACK _model_info_spines_ann_dialog_proc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   switch( uMsg )
      {
      case WM_INITDIALOG:
         _model_info_spines_ann_show( hDlg );
         return FALSE;
      }

   return FALSE;
   }


LRESULT CALLBACK _model_info_color_proc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
	switch( uMsg )
		{
		case WM_PAINT:
			{
         PAINTSTRUCT ps;
         HDC         hDC;
			RECT        rc;
			HBRUSH      brush;


         hDC = BeginPaint( hWnd, &ps );

			GetClientRect( hWnd, &rc );

			brush = CreateSolidBrush( ( COLORREF )GetWindowLongPtr( hWnd, GWLP_USERDATA ) );
			FillRect( hDC, &rc, brush );
			DeleteObject( brush );

         EndPaint( hWnd, &ps );
			}
			break;

		default:
			return DefWindowProc( hWnd, uMsg, wParam, lParam );
		}

	return 0;
	}


NS_PRIVATE void _model_info_color_init( HWND hDlg, nsint id, NsColor4ub C )
	{
	HWND box = GetDlgItem( hDlg, id );

	SetWindowLongPtr( box, GWLP_USERDATA, ( LONG_PTR )RGB( C.x, C.y, C.z ) );
	SetWindowLongPtr( box, GWLP_WNDPROC, ( LONG_PTR )_model_info_color_proc );
	}


INT_PTR CALLBACK _model_info_vertices_dialog_proc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   switch( uMsg )
      {
      case WM_INITDIALOG:
         _model_info_vertices_show( hDlg );

			_model_info_color_init( hDlg, IDS_MODEL_INFO_ORIGIN_VERTICES_COLOR,
				workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_ORIGIN_VERTEX ) );

			_model_info_color_init( hDlg, IDS_MODEL_INFO_LINE_VERTICES_COLOR,
				workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_LINE_VERTEX ) );

			_model_info_color_init( hDlg, IDS_MODEL_INFO_JUNCTION_VERTICES_COLOR,
				workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_JUNCTION_VERTEX ) );

			_model_info_color_init( hDlg, IDS_MODEL_INFO_EXTERNAL_VERTICES_COLOR,
				workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_EXTERNAL_VERTEX ) );

			_model_info_color_init( hDlg, IDS_MODEL_INFO_SOMA_VERTICES_COLOR,
				workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_FUNCTION_SOMA ) );

			_model_info_color_init( hDlg, IDS_MODEL_INFO_BASAL_VERTICES_COLOR,
				workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_FUNCTION_BASAL_DENDRITE ) );

			_model_info_color_init( hDlg, IDS_MODEL_INFO_APICAL_VERTICES_COLOR,
				workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_FUNCTION_APICAL_DENDRITE ) );

			_model_info_color_init( hDlg, IDS_MODEL_INFO_AXON_VERTICES_COLOR,
				workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_FUNCTION_AXON ) );

			_model_info_color_init( hDlg, IDS_MODEL_INFO_UNKNOWN_VERTICES_COLOR, NS_COLOR4UB_BLACK );

         return FALSE;
      }

   return FALSE;
   }


INT_PTR CALLBACK _model_info_edges_dialog_proc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   switch( uMsg )
      {
      case WM_INITDIALOG:
         _model_info_edges_show( hDlg );
         return FALSE;
      }

   return FALSE;
   }


NS_PRIVATE void _model_info_trees_show( HWND hDlg )
   {
   ns_snprint( ____model_info_buffer, NS_ARRAY_LENGTH( ____model_info_buffer ), NS_FMT_ULONG, ____model_info.num_trees );
   SetDlgItemText( hDlg, IDS_MODEL_INFO_NUM_TREES, ____model_info_buffer );

   ns_snprint( ____model_info_buffer, NS_ARRAY_LENGTH( ____model_info_buffer ), NS_FMT_ULONG, ____model_info.num_basal_trees );
   SetDlgItemText( hDlg, IDS_MODEL_INFO_NUM_BASAL_TREES, ____model_info_buffer );

   ns_snprint( ____model_info_buffer, NS_ARRAY_LENGTH( ____model_info_buffer ), NS_FMT_ULONG, ____model_info.num_apical_trees );
   SetDlgItemText( hDlg, IDS_MODEL_INFO_NUM_APICAL_TREES, ____model_info_buffer );

   ns_snprint( ____model_info_buffer, NS_ARRAY_LENGTH( ____model_info_buffer ), NS_FMT_ULONG, ____model_info.num_axon_trees );
   SetDlgItemText( hDlg, IDS_MODEL_INFO_NUM_AXON_TREES, ____model_info_buffer );

   ns_snprint( ____model_info_buffer, NS_ARRAY_LENGTH( ____model_info_buffer ), NS_FMT_ULONG, ____model_info.num_unknown_trees );
   SetDlgItemText( hDlg, IDS_MODEL_INFO_NUM_UNKNOWN_TREES, ____model_info_buffer );

   ns_snprint( ____model_info_buffer, NS_ARRAY_LENGTH( ____model_info_buffer ), NS_FMT_ULONG, ____model_info.num_invalid_trees );
   SetDlgItemText( hDlg, IDS_MODEL_INFO_NUM_INVALID_TREES, ____model_info_buffer );
   }


INT_PTR CALLBACK _model_info_trees_dialog_proc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   switch( uMsg )
      {
      case WM_INITDIALOG:
         _model_info_trees_show( hDlg );

			_model_info_color_init( hDlg, IDS_MODEL_INFO_BASAL_TREES_COLOR,
				workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_FUNCTION_BASAL_DENDRITE ) );

			_model_info_color_init( hDlg, IDS_MODEL_INFO_APICAL_TREES_COLOR,
				workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_FUNCTION_APICAL_DENDRITE ) );

			_model_info_color_init( hDlg, IDS_MODEL_INFO_AXON_TREES_COLOR,
				workspace_get_color( s_MainWindow.activeWorkspace, WORKSPACE_COLOR_FUNCTION_AXON ) );

			_model_info_color_init( hDlg, IDS_MODEL_INFO_UNKNOWN_TREES_COLOR, NS_COLOR4UB_BLACK );

         return FALSE;
      }

   return FALSE;
   }


INT_PTR CALLBACK _model_info_spines_dialog_proc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   switch( uMsg )
      {
      case WM_INITDIALOG:
         _model_info_spines_show( hDlg );

			_model_info_color_init( hDlg, IDS_MODEL_INFO_STUBBY_SPINES_COLOR,
				workspace_get_spine_color_by_type( s_MainWindow.activeWorkspace, NS_SPINE_STUBBY ) );

			_model_info_color_init( hDlg, IDS_MODEL_INFO_THIN_SPINES_COLOR,
				workspace_get_spine_color_by_type( s_MainWindow.activeWorkspace, NS_SPINE_THIN ) );

			_model_info_color_init( hDlg, IDS_MODEL_INFO_MUSHROOM_SPINES_COLOR,
				workspace_get_spine_color_by_type( s_MainWindow.activeWorkspace, NS_SPINE_MUSHROOM ) );

			_model_info_color_init( hDlg, IDS_MODEL_INFO_OTHER_SPINES_COLOR,
				workspace_get_spine_color_by_type( s_MainWindow.activeWorkspace, NS_SPINE_OTHER ) );

         return FALSE;
      }

   return FALSE;
   }


NS_PRIVATE void _model_info_build_vertex_list( HWND list, const NsModel *model )
   {
   nsmodelvertex vertex;
   nssize i;
   NsVector3f position;
   nsfloat radius;
   nssize max_extent, curr_extent;
   TEXTMETRIC tm;
   HDC hDC;
   nschar buffer[ 512 ];


   if( 0 < ns_model_num_vertices( model ) )
      {
      max_extent = 0;
      i          = 0;

      hDC = GetDC( list );
      GetTextMetrics( hDC, &tm );
      ReleaseDC( list, hDC );

      NS_MODEL_VERTEX_FOREACH( model, vertex )
         {
         ns_model_vertex_get_position( vertex, &position );
         radius = ns_model_vertex_get_radius( vertex );

         ns_snprint(
            buffer,
            NS_ARRAY_LENGTH( buffer ),
            "ID:%lu   POS:{%.3f, %.3f, %.3f}   RADIUS:%.3f   TYPE:%s",
            i,
            position.x,
            position.y,
            position.z,
            radius,
            ns_model_vertex_type_to_string( ns_model_vertex_get_type( vertex ) )
            );

         SendMessage( list, LB_ADDSTRING, 0, ( LPARAM )buffer );

         /* TEMP Is this right? */
         curr_extent = ( tm.tmAveCharWidth + 2 ) * ns_ascii_strlen( buffer );

         if( max_extent < curr_extent )
            max_extent = curr_extent;

         ++i;
         }

      SendMessage( list, LB_SETHORIZONTALEXTENT, ( WPARAM )max_extent, 0 );
      }
   else
      SendMessage( list, LB_ADDSTRING, 0, ( LPARAM )"None" );
   }


NS_PRIVATE void _model_info_build_edge_list( HWND list, const NsModel *model )
   {
   nsmodelvertex  curr_vertex;
   nsmodelvertex  end_vertices;
   nsmodeledge    curr_edge;
   nsmodeledge    end_edges;
   nssize i;
   nslong order;
   nsulong section;
   nssize max_extent, curr_extent;
   TEXTMETRIC tm;
   HDC hDC;
   nschar buffer[ 512 ];


   if( 0 < ns_model_num_edges( model ) )
      {
      max_extent = 0;
      i          = 0;

      hDC = GetDC( list );
      GetTextMetrics( hDC, &tm );
      ReleaseDC( list, hDC );

      curr_vertex  = ns_model_begin_vertices( model );
      end_vertices = ns_model_end_vertices( model );

      for( ; ns_model_vertex_not_equal( curr_vertex, end_vertices );
             curr_vertex = ns_model_vertex_next( curr_vertex ) )
         {
         curr_edge = ns_model_vertex_begin_edges( curr_vertex );
         end_edges = ns_model_vertex_end_edges( curr_vertex );

         for( ; ns_model_edge_not_equal( curr_edge, end_edges );
                curr_edge = ns_model_edge_next( curr_edge ) )
            {
            order   = ns_model_edge_get_order( curr_edge );
            section = ns_model_edge_get_section( curr_edge );

            ns_snprint(
               buffer,
               NS_ARRAY_LENGTH( buffer ),
               "ID:%lu   ORDER:%ld   SECTION:%lu",
               i,
               order,
               section
               );

            SendMessage( list, LB_ADDSTRING, 0, ( LPARAM )buffer );

            /* TEMP Is this right? */
            curr_extent = ( tm.tmAveCharWidth + 2 ) * ns_ascii_strlen( buffer );

            if( max_extent < curr_extent )
               max_extent = curr_extent;

            ++i;
            }
         }

      SendMessage( list, LB_SETHORIZONTALEXTENT, ( WPARAM )max_extent, 0 );
      }
   else
      SendMessage( list, LB_ADDSTRING, 0, ( LPARAM )"None" );
   }


NS_PRIVATE void _model_info_build_tree_list( HWND list, const NsModel *model )
	{
	nsmodeltree  curr, end;
	NsVector3f position;
   nssize max_extent, curr_extent;
   TEXTMETRIC tm;
   HDC hDC;
   nschar       buffer[ 512 ];


   if( 0 < ns_model_num_trees( model ) )
      {
      max_extent = 0;

      hDC = GetDC( list );
      GetTextMetrics( hDC, &tm );
      ReleaseDC( list, hDC );

		curr = ns_model_begin_trees( model );
		end  = ns_model_end_trees( model );

      for( ; ns_model_tree_not_equal( curr, end ); curr = ns_model_tree_next( curr ) )
         {
         ns_model_vertex_get_position( ns_model_tree_root( curr ), &position );

         ns_snprint(
            buffer,
            NS_ARRAY_LENGTH( buffer ),
            "ID:%lu   TYPE:%s   ROOT:{%.3f, %.3f, %.3f}",
            ns_model_tree_total_index( curr ),
            ns_model_function_type_to_string( ns_model_tree_function_type( curr ) ),
            position.x,
            position.y,
            position.z
            );

         SendMessage( list, LB_ADDSTRING, 0, ( LPARAM )buffer );

         /* TEMP Is this right? */
         curr_extent = ( tm.tmAveCharWidth + 2 ) * ns_ascii_strlen( buffer );

         if( max_extent < curr_extent )
            max_extent = curr_extent;
         }

      SendMessage( list, LB_SETHORIZONTALEXTENT, ( WPARAM )max_extent, 0 );
      }
   else
      SendMessage( list, LB_ADDSTRING, 0, ( LPARAM )"None" );
	}


NS_PRIVATE void _model_info_build_spine_list( HWND list, const NsModel *model )
   {
   nssize i;
   nslistiter curr, end;
   nssize max_extent, curr_extent;
   TEXTMETRIC tm;
   HDC hDC;
   nschar buffer[ 512 ];


   if( 0 < ns_model_num_spines( model ) && NS_MODEL_SPINES_SHOW_RESULTS )
      {
      max_extent = 0;
      i          = 0;

      hDC = GetDC( list );
      GetTextMetrics( hDC, &tm );
      ReleaseDC( list, hDC );

      curr = ns_model_begin_spines( model );
      end = ns_model_end_spines( model );

      for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
         {
         ns_snprint(
            buffer,
            NS_ARRAY_LENGTH( buffer ),
            "ID:%lu   CENTER:{%.3f, %.3f, %.3f}   TYPE:%s",
            ( nssize )ns_spine_get_id( curr ),
            ns_spine_get_x( curr ),
            ns_spine_get_y( curr ),
            ns_spine_get_z( curr ),
            ns_spine_type_to_string( ns_spine_get_type( curr ) )
            );

         SendMessage( list, LB_ADDSTRING, 0, ( LPARAM )buffer );

         /* TEMP Is this right? */
         curr_extent = ( tm.tmAveCharWidth + 2 ) * ns_ascii_strlen( buffer );

         if( max_extent < curr_extent )
            max_extent = curr_extent;

         ++i;
         }

      SendMessage( list, LB_SETHORIZONTALEXTENT, ( WPARAM )max_extent, 0 );
      }
   else
      SendMessage( list, LB_ADDSTRING, 0, ( LPARAM )"None" );
   }


INT_PTR CALLBACK _model_info_vertex_list_dialog_proc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   switch( uMsg )
      {
      case WM_INITDIALOG:
         _model_info_build_vertex_list(
            GetDlgItem( hDlg, IDL_MODEL_INFO_VERTEX_LIST ), 
            GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace )
            );
         return FALSE;
      }

   return FALSE;
   }


INT_PTR CALLBACK _model_info_edge_list_dialog_proc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   switch( uMsg )
      {
      case WM_INITDIALOG:
         _model_info_build_edge_list(
            GetDlgItem( hDlg, IDL_MODEL_INFO_EDGE_LIST ), 
            GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace )
            );
         return FALSE;
      }

   return FALSE;
   }


INT_PTR CALLBACK _model_info_tree_list_dialog_proc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   switch( uMsg )
      {
      case WM_INITDIALOG:
         _model_info_build_tree_list(
            GetDlgItem( hDlg, IDL_MODEL_INFO_TREE_LIST ), 
            GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace )
            );
         return FALSE;
      }

   return FALSE;
   }


INT_PTR CALLBACK _model_info_spine_list_dialog_proc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   switch( uMsg )
      {
      case WM_INITDIALOG:
         _model_info_build_spine_list(
            GetDlgItem( hDlg, IDL_MODEL_INFO_SPINE_LIST ), 
            GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace )
            );
         return FALSE;
      }

   return FALSE;
   }


NS_PRIVATE void _model_info_create_tabs( void )
   {
   ns_tab_control_add(
      &____model_info_tc,
      "Vertices",
      "MODEL_INFO_VERTICES",
      _model_info_vertices_dialog_proc
      );

   ns_tab_control_add(
      &____model_info_tc,
      "Edges",
      "MODEL_INFO_EDGES",
      _model_info_edges_dialog_proc
      );

   ns_tab_control_add(
      &____model_info_tc,
      "Trees",
      "MODEL_INFO_TREES",
      _model_info_trees_dialog_proc
      );

	if( NULL == ns_spines_classifier_get() )
		ns_tab_control_add(
			&____model_info_tc,
			"Spines",
			"MODEL_INFO_SPINES",
			_model_info_spines_dialog_proc
			);
	else
		ns_tab_control_add(
			&____model_info_tc,
			"Spines",
			"MODEL_INFO_SPINES_ANN",
			_model_info_spines_ann_dialog_proc
			);
		

   ns_tab_control_add(
      &____model_info_tc,
      "Misc.",
      "MODEL_INFO_ALL",
      _model_info_all_dialog_proc
      );

   /* IMPORTANT: Major flaw in Visual C++ 6.0 resource editor. The name
      "MODEL_INFO_VERTEX_LIST", i.e. without the trailing underscore,
      causes some sort of bug when the resource file is compiled.
      Specifically, when the resource file is re-opened after compilation
      the name appears without the quotation marks? */
/*
   ns_tab_control_add(
      &____model_info_tc,
      "Vertex List",
      "MODEL_INFO_VERTEX_LIST_",
      _model_info_vertex_list_dialog_proc
      );

   ns_tab_control_add(
      &____model_info_tc,
      "Edge List",
      "MODEL_INFO_EDGE_LIST",
      _model_info_edge_list_dialog_proc
      );

   ns_tab_control_add(
      &____model_info_tc,
      "Tree List",
      "MODEL_INFO_TREE_LIST",
      _model_info_tree_list_dialog_proc
      );

   ns_tab_control_add(
      &____model_info_tc,
      "Spine List",
      "MODEL_INFO_SPINE_LIST",
      _model_info_spine_list_dialog_proc
      );
*/

   ns_tab_control_on_select( &____model_info_tc );
   }


INT_PTR CALLBACK _model_info_dialog_proc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   switch( uMsg )
      {
      case WM_INITDIALOG:
         CenterWindow( hDlg, s_MainWindow.hWnd );

         ns_tab_control_init(
            &____model_info_tc,
            g_Instance,
            hDlg,
            IDT_MODEL_INFO,
            5//8
            );

         _model_info_create_tabs();

         return FALSE;


      case WM_NOTIFY:
         switch( ( ( LPNMHDR )lParam )->code )
            {
            case TCN_SELCHANGE:
               ns_tab_control_on_select( &____model_info_tc );
               break;
            } 
         break; 


      case WM_COMMAND:
         switch( LOWORD( wParam ) )
            {
            case IDOK:
               EndDialog( hDlg, 0 );
               break;
            }
         return TRUE;

      case WM_CLOSE:
         EndDialog( hDlg, 0 );
         return TRUE;
      }

   return FALSE;
   }


void _model_info_dialog( void )
   {
   const NsModel  *model;
	NsError         error;


	error = ns_no_error();
	model = GetWorkspaceNeuronTree( s_MainWindow.activeWorkspace );

   /* NOTE: Dont call these during WM_INITDIALOG since they
      may still be running when the various tabs will be shown. */
   ns_model_info(
		model,
		&____model_info,
		NS_MODEL_INFO_VERTICES | NS_MODEL_INFO_EDGES | NS_MODEL_INFO_TREES
		);

	ns_spines_info_init( &____spines_info );
	ns_spines_classifier_info_init( &____spines_classifier_info );

   if( NS_MODEL_SPINES_SHOW_RESULTS )
		{
		if( NULL == ns_spines_classifier_get() )
			ns_model_spines_info( model, &____spines_info );
		else
			error = ns_model_spines_classifier_info( model, ns_spines_classifier_get(), &____spines_classifier_info );
		}

	if( ! ns_is_error( error ) )
		DialogBox( g_Instance, "MODEL_INFO", s_MainWindow.hWnd, _model_info_dialog_proc );

	ns_spines_classifier_info_finalize( &____spines_classifier_info );
   }


