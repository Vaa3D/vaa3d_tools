
NS_PRIVATE void _ns_spine_set_classifier_input( NsSpine *spine, fann_type *input )
	{
	input[ NS_SPINES_CLASSIFIER_HEAD_DIAMETER ] = ( fann_type )spine->stats.head_diameter;
	input[ NS_SPINES_CLASSIFIER_NECK_DIAMETER ] = ( fann_type )spine->stats.neck_diameter;
	input[ NS_SPINES_CLASSIFIER_HAS_NECK      ] = ( fann_type )( spine->stats.has_neck ? 1 : -1 );
	input[ NS_SPINES_CLASSIFIER_HEIGHT        ] = ( fann_type )spine->height;
	input[ NS_SPINES_CLASSIFIER_BASE_DEPTH    ] = ( fann_type )spine->stats.base_depth;
	input[ NS_SPINES_CLASSIFIER_BASE_WIDTH    ] = ( fann_type )spine->stats.base_width;
	input[ NS_SPINES_CLASSIFIER_XYPLANE_ANGLE ] = ( fann_type )spine->xyplane_angle;
	}


nsboolean ns_model_spines_any_selected_are_invalid( const NsModel *model )
	{
   nslistiter   curr;
   nslistiter   end;
   NsSpine     *spine;


	ns_assert( NULL != model );

   curr = ns_list_begin( &(model->spines->list) );
   end  = ns_list_end( &(model->spines->list) );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
      {
      spine = ns_list_iter_get_object( curr );

		if( spine->auto_detected && spine->is_selected )
			if( 0 == ( nsuint )spine->type )
				return NS_TRUE;
		}

	return NS_FALSE;
	}


NsError ns_model_spines_add_selected_to_classifier( const NsModel *model, NsSpinesClassifier *sc )
	{
	fann_type    input[ NS_SPINES_CLASSIFIER_NUM_INPUT ];
	fann_type   *output;
	nsuint       num_output;
	nsuint       klass;
   nslistiter   curr;
   nslistiter   end;
   NsSpine     *spine;
	NsError      error;


	ns_assert( NULL != model );
	ns_assert( NULL != sc );

	num_output = ( nsuint )ns_spines_classifier_num_classes( sc );
	ns_assert( 0 < num_output );

	if( NULL == ( output = ns_malloc( num_output * sizeof( fann_type ) ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

   curr = ns_list_begin( &(model->spines->list) );
   end  = ns_list_end( &(model->spines->list) );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
      {
      spine = ns_list_iter_get_object( curr );

		if( spine->auto_detected && spine->is_selected )
			{
			klass = ( nsuint )spine->type;

			if( 0 < klass && klass <= num_output )
				{
				_ns_spine_set_classifier_input( spine, input );
				ns_memzero( output, num_output * sizeof( fann_type ) );

				output[ klass - 1 ] = ( fann_type )1;

				if( NS_FAILURE(
						ns_spines_classifier_add_datum(
							sc,
							input,
							NS_SPINES_CLASSIFIER_NUM_INPUT,
							output,
							num_output
							),
						error ) )
					{
					ns_free( output );
					return error;
					}
				}
			}
		}

	ns_free( output );

	//ns_spines_classifier_print( sc );

	return ns_no_error();
	}


NsError ns_model_spines_retype_by_ann_classifier
	(
	NsModel             *model,
	NsSpinesClassifier  *classifier,
	NsProgress          *progress
	)
	{
	fann_type    input[ NS_SPINES_CLASSIFIER_NUM_INPUT ];
	fann_type   *output;
	nsuint       num_output;
   nslistiter   curr, end;
	NsSpine     *spine;
	nsboolean    valid;
	NsError      error;


	ns_assert( NULL != model );
	ns_assert( NULL != classifier );

	num_output = ( nsuint )ns_spines_classifier_num_classes( classifier );
	ns_assert( 0 < num_output );

	if( NULL == ( output = ns_malloc( num_output * sizeof( fann_type ) ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

   curr = ns_list_begin( &(model->spines->list) );
   end  = ns_list_end( &(model->spines->list) );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
		{
		spine = ns_list_iter_get_object( curr );

		if( spine->auto_detected )
			{
			_ns_spine_set_classifier_input( spine, input );
			ns_memzero( output, num_output * sizeof( fann_type ) );

			if( NS_FAILURE(
					ns_spines_classifier_run(
						classifier,
						input,
						NS_SPINES_CLASSIFIER_NUM_INPUT,
						output,
						num_output,
						&valid,
						progress
						),
					error ) )
				{
				ns_free( output );
				return error;
				}

			spine->type =
				valid ?
					ns_spines_classifier_type( classifier, output, num_output )
					:
					NS_SPINE_INVALID; /* i.e. 0 */
			}
		}

	ns_free( output );
	return ns_no_error();
	}


NsError ns_model_spines_retype_unselected_by_ann_classifier
	(
	NsModel             *model,
	NsSpinesClassifier  *classifier,
	NsProgress          *progress
	)
	{
	fann_type    input[ NS_SPINES_CLASSIFIER_NUM_INPUT ];
	fann_type   *output;
	nsuint       num_output;
   nslistiter   curr, end;
	NsSpine     *spine;
	nsboolean    valid;
	NsError      error;


	ns_assert( NULL != model );
	ns_assert( NULL != classifier );

	num_output = ( nsuint )ns_spines_classifier_num_classes( classifier );
	ns_assert( 0 < num_output );

	if( NULL == ( output = ns_malloc( num_output * sizeof( fann_type ) ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

   curr = ns_list_begin( &(model->spines->list) );
   end  = ns_list_end( &(model->spines->list) );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
		{
		spine = ns_list_iter_get_object( curr );

		if( spine->auto_detected && ( ! spine->is_selected ) )
			{
			_ns_spine_set_classifier_input( spine, input );
			ns_memzero( output, num_output * sizeof( fann_type ) );

			if( NS_FAILURE(
					ns_spines_classifier_run(
						classifier,
						input,
						NS_SPINES_CLASSIFIER_NUM_INPUT,
						output,
						num_output,
						&valid,
						progress
						),
					error ) )
				{
				ns_free( output );
				return error;
				}

			spine->type =
				valid ?
					ns_spines_classifier_type( classifier, output, num_output )
					:
					NS_SPINE_INVALID; /* i.e. 0 */
			}
		}

	ns_free( output );
	return ns_no_error();
	}