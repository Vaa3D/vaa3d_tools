
/*****************************************************
 Routine(s) to locate spine in the spread profile of
 a Maximum. It returns type/invalid as well as
 where in the profile the base of the spine is
 located.
******************************************************/

typedef struct
   {
	nsdouble   depth;
	nsdouble   spread;
	nsdouble   diameter;
	nsdouble   count_ratio;
	nsdouble   spread_ratio;
	nssize     voxel_count;
	nspointer  user_data;
   }
   LAYERDESC;

typedef struct
   {
	nsint      spine_type;
	nsint      base_id; /* zero based */
	nsint      has_neck;
	nsdouble   head_diameter;
	nsint      head_layer;
	nsdouble   neck_diameter;
	nsint      neck_layer;
	nsdouble   depth_to_width;
	nsdouble   base_depth;
	nsdouble   base_width;
	nsboolean  large_base;
   }
   SPINESTATS;


#define SPINE_TYPE_INVALID      0
#define SPINE_TYPE_STUBBY       1
#define SPINE_TYPE_THIN         2
#define SPINE_TYPE_MUSHROOM     3
#define SPINE_TYPE_OTHER        4


nsdouble  ____thin_ratio;
nsdouble  ____invalid_ratio;
nsdouble  ____neck_ratio;
nsdouble  ____mushroom_size;
nsdouble  ____max_count_ratio;
nsdouble  ____max_spread_ratio;
nsdouble  ____min_height;


void GetSpineStats
   (
   LAYERDESC  *layers,
   nsint       count,
   SPINESTATS *stats
   )
   {
   nsint     neckpos;
   nsint     headpos;
   nsint     i;
   nsint     j;
   nsdouble  ratio,neckness;


	ns_assert( NULL != layers );
	ns_assert( NULL != stats );

	stats->spine_type     = SPINE_TYPE_OTHER;
	stats->base_id        = -1;
	stats->has_neck       = NS_FALSE;
	stats->head_diameter  = NS_SPINE_DEFAULT_HEAD_DIAMETER;
	stats->neck_diameter  = NS_SPINE_DEFAULT_NECK_DIAMETER;
	stats->depth_to_width = 0.0;
	stats->base_depth     = 0.0;
	stats->base_width     = 0.0;

   /* Select base */
   stats->base_id = count - 1;

	if( 0 != layers[ count - 1 ].voxel_count )
		{
      stats->base_id = count - 2;

      if( stats->base_id < 0 )
			{
         stats->base_id    = 0;
         stats->spine_type = SPINE_TYPE_INVALID;

			return;
			}

		while(
			stats->base_id > 0 &&
				(
				layers[ stats->base_id ].depth / layers[ stats->base_id ].spread <= ____invalid_ratio
				||
				layers[ stats->base_id ].count_ratio > ____max_count_ratio
				||
				layers[ stats->base_id ].spread_ratio > ____max_spread_ratio
				)
			//layers[ stats->base_id - 1 ].depth >= ____min_height &&
			//layers[ stats->base_id ].depth / layers[ stats->base_id ].spread <
			//layers[ stats->base_id - 1 ].depth / layers[ stats->base_id - 1 ].spread
			)
		 --(stats->base_id);

		/*
       for( i = 1; i <= stats->base_id; ++i )
			if( ____max_count_ratio < layers[i].count_ratio ||
				 ____max_spread_ratio < layers[i].spread_ratio )
				{
            stats->base_id = i - 1;
				break;
				}
		*/
		}

   /* find out if not a valid spine */
   ratio = layers[ stats->base_id ].depth / layers[ stats->base_id ].spread;

    if( layers[ stats->base_id ].depth < ____min_height || ratio <= ____invalid_ratio )
		{
      stats->spine_type = SPINE_TYPE_INVALID;
		return;
		}

	/* Search for a possible neck. */
	 headpos  = 0;
    neckpos  = stats->base_id;
    neckness = layers[ headpos ].diameter / layers[ neckpos ].diameter;

    for( i = 0; i <= stats->base_id; i++ )
		{
		/* For every possible head. */
		for( j = 0; j < i; j++ )
			{
			ratio = layers[j].diameter / layers[i].diameter;

			if( ratio > neckness )
				{
				neckness = ratio;
				neckpos  = i;
				headpos  = j;
				}
			}
		}

	stats->head_diameter  = layers[ headpos ].diameter;
	stats->neck_diameter  = layers[ neckpos ].diameter;

	stats->head_layer = headpos;
	stats->neck_layer = neckpos;

	stats->depth_to_width = layers[ stats->base_id ].depth / layers[ stats->base_id ].diameter;
	stats->base_depth     = layers[ stats->base_id ].depth;
	stats->base_width     = layers[ stats->base_id ].diameter;

    if( neckness >= ____neck_ratio )
		stats->has_neck = NS_TRUE;

	/* Remove "wide" base layers. */

	//stats->large_base = NS_FALSE;

	if( stats->has_neck )
		{
		if( layers[ stats->base_id ].spread > layers[ stats->head_layer ].spread )
			{
			//stats->large_base = NS_TRUE;
			//--(stats->base_id);
			}

		//while( stats->base_id > 0 && ( layers[ stats->base_id ].spread > layers[ stats->head_layer ].spread ) )
		//	--(stats->base_id);
		}
   }


void SetSpineType( SPINESTATS *stats )
   {
	nsdouble neckness;

	stats->has_neck = NS_FALSE;

	neckness = stats->head_diameter / stats->neck_diameter;

   /* Do clasification stored values */
	if( neckness >= ____neck_ratio )
		{
		stats->has_neck   = NS_TRUE;
      stats->spine_type = SPINE_TYPE_THIN;
	
		if( stats->head_diameter >=  ____mushroom_size )
			stats->spine_type = SPINE_TYPE_MUSHROOM;
		}
	else
		{
      if( stats->depth_to_width >= ____thin_ratio )
            stats->spine_type = SPINE_TYPE_THIN;
		else
            stats->spine_type = SPINE_TYPE_STUBBY;
		}
   }
