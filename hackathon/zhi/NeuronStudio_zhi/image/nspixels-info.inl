
NS_PRIVATE void _ns_pixel_infos_init( void )
	{
	NsPixelInfo     *info;
	NsPixelChannel  *channel0;
	NsPixelChannel  *channel1;
	NsPixelChannel  *channel2;
	NsPixelChannel  *channel3;


	/**********************************/
	info = _ns_pixel_infos + NS_PIXEL_LUM_U1;

	info->pixel_type   = NS_PIXEL_LUM_U1;
	info->format_type  = NS_PIXEL_FORMAT_LUM;
	info->num_channels = 1;

	channel0 = info->channels + NS_PIXEL_CHANNEL_LUM;

	channel0->type  = NS_PIXEL_CHANNEL_UINT;
	channel0->bits  = 1;
	channel0->ubits = 1;
	channel0->name  = NS_PIXEL_CHANNEL_LUM_NAME;
	channel0->index = NS_PIXEL_CHANNEL_LUM;

	info->channel  = channel0;
	channel0->next = NULL;


	/**********************************/
	info = _ns_pixel_infos + NS_PIXEL_LUM_U8;

	info->pixel_type   = NS_PIXEL_LUM_U8;
	info->format_type  = NS_PIXEL_FORMAT_LUM;
	info->num_channels = 1;

	channel0 = info->channels + NS_PIXEL_CHANNEL_LUM;

	channel0->type  = NS_PIXEL_CHANNEL_UINT;
	channel0->bits  = 8;
	channel0->ubits = 8;
	channel0->name  = NS_PIXEL_CHANNEL_LUM_NAME;
	channel0->index = NS_PIXEL_CHANNEL_LUM;

	info->channel  = channel0;
	channel0->next = NULL;


	/**********************************/
	info = _ns_pixel_infos + NS_PIXEL_LUM_U12;

	info->pixel_type   = NS_PIXEL_LUM_U12;
	info->format_type  = NS_PIXEL_FORMAT_LUM;
	info->num_channels = 1;

	channel0 = info->channels + NS_PIXEL_CHANNEL_LUM;

	channel0->type  = NS_PIXEL_CHANNEL_UINT;
	channel0->bits  = 16;
	channel0->ubits = 12;
	channel0->name  = NS_PIXEL_CHANNEL_LUM_NAME;
	channel0->index = NS_PIXEL_CHANNEL_LUM;

	info->channel  = channel0;
	channel0->next = NULL;


	/**********************************/
	info = _ns_pixel_infos + NS_PIXEL_LUM_U16;

	info->pixel_type   = NS_PIXEL_LUM_U16;
	info->format_type  = NS_PIXEL_FORMAT_LUM;
	info->num_channels = 1;

	channel0 = info->channels + NS_PIXEL_CHANNEL_LUM;

	channel0->type  = NS_PIXEL_CHANNEL_UINT;
	channel0->bits  = 16;
	channel0->ubits = 16;
	channel0->name  = NS_PIXEL_CHANNEL_LUM_NAME;
	channel0->index = NS_PIXEL_CHANNEL_LUM;

	info->channel  = channel0;
	channel0->next = NULL;

	
	/**********************************/
	info = _ns_pixel_infos + NS_PIXEL_LUM_F32;

	info->pixel_type   = NS_PIXEL_LUM_F32;
	info->format_type  = NS_PIXEL_FORMAT_LUM;
	info->num_channels = 1;

	channel0 = info->channels + NS_PIXEL_CHANNEL_LUM;

	channel0->type  = NS_PIXEL_CHANNEL_FLOAT;
	channel0->bits  = 32;
	channel0->ubits = 32;
	channel0->name  = NS_PIXEL_CHANNEL_LUM_NAME;
	channel0->index = NS_PIXEL_CHANNEL_LUM;

	info->channel  = channel0;
	channel0->next = NULL;


	/**********************************/
	info = _ns_pixel_infos + NS_PIXEL_LUM_ALPHA_U8_U8;

	info->pixel_type   = NS_PIXEL_LUM_ALPHA_U8_U8;
	info->format_type  = NS_PIXEL_FORMAT_LUM_ALPHA;
	info->num_channels = 2;

	channel0 = info->channels + NS_PIXEL_CHANNEL_LUM;
	channel1 = info->channels + NS_PIXEL_CHANNEL_ALPHA;

	channel0->type  = NS_PIXEL_CHANNEL_UINT;
	channel0->bits  = 8;
	channel0->ubits = 8;
	channel0->name  = NS_PIXEL_CHANNEL_LUM_NAME;
	channel0->index = NS_PIXEL_CHANNEL_LUM;

	channel1->type  = NS_PIXEL_CHANNEL_UINT;
	channel1->bits  = 8;
	channel1->ubits = 8;
	channel1->name  = NS_PIXEL_CHANNEL_ALPHA_NAME;
	channel1->index = NS_PIXEL_CHANNEL_ALPHA;

	info->channel  = channel0;
	channel0->next = channel1;
	channel1->next = NULL;


	/**********************************/
	info = _ns_pixel_infos + NS_PIXEL_RGB_U8_U8_U8;

	info->pixel_type   = NS_PIXEL_RGB_U8_U8_U8;
	info->format_type  = NS_PIXEL_FORMAT_RGB;
	info->num_channels = 3;

	channel0 = info->channels + NS_PIXEL_CHANNEL_RED;
	channel1 = info->channels + NS_PIXEL_CHANNEL_GREEN;
	channel2 = info->channels + NS_PIXEL_CHANNEL_BLUE;

	channel0->type  = NS_PIXEL_CHANNEL_UINT;
	channel0->bits  = 8;
	channel0->ubits = 8;
	channel0->name  = NS_PIXEL_CHANNEL_RED_NAME;
	channel0->index = NS_PIXEL_CHANNEL_RED;

	channel1->type  = NS_PIXEL_CHANNEL_UINT;
	channel1->bits  = 8;
	channel1->ubits = 8;
	channel1->name  = NS_PIXEL_CHANNEL_GREEN_NAME;
	channel1->index = NS_PIXEL_CHANNEL_GREEN;

	channel2->type  = NS_PIXEL_CHANNEL_UINT;
	channel2->bits  = 8;
	channel2->ubits = 8;
	channel2->name  = NS_PIXEL_CHANNEL_BLUE_NAME;
	channel2->index = NS_PIXEL_CHANNEL_BLUE;

	info->channel  = channel0;
	channel0->next = channel1;
	channel1->next = channel2;
	channel2->next = NULL;


	/**********************************/
	info = _ns_pixel_infos + NS_PIXEL_RGBA_U8_U8_U8_U8;

	info->pixel_type   = NS_PIXEL_RGBA_U8_U8_U8_U8;
	info->format_type  = NS_PIXEL_FORMAT_RGBA;
	info->num_channels = 4;

	channel0 = info->channels + NS_PIXEL_CHANNEL_RED;
	channel1 = info->channels + NS_PIXEL_CHANNEL_GREEN;
	channel2 = info->channels + NS_PIXEL_CHANNEL_BLUE;
	channel3 = info->channels + NS_PIXEL_CHANNEL_ALPHA;

	channel0->type  = NS_PIXEL_CHANNEL_UINT;
	channel0->bits  = 8;
	channel0->ubits = 8;
	channel0->name  = NS_PIXEL_CHANNEL_RED_NAME;
	channel0->index = NS_PIXEL_CHANNEL_RED;

	channel1->type  = NS_PIXEL_CHANNEL_UINT;
	channel1->bits  = 8;
	channel1->ubits = 8;
	channel1->name  = NS_PIXEL_CHANNEL_GREEN_NAME;
	channel1->index = NS_PIXEL_CHANNEL_GREEN;

	channel2->type  = NS_PIXEL_CHANNEL_UINT;
	channel2->bits  = 8;
	channel2->ubits = 8;
	channel2->name  = NS_PIXEL_CHANNEL_BLUE_NAME;
	channel2->index = NS_PIXEL_CHANNEL_BLUE;

	channel3->type  = NS_PIXEL_CHANNEL_UINT;
	channel3->bits  = 8;
	channel3->ubits = 8;
	channel3->name  = NS_PIXEL_CHANNEL_ALPHA_NAME;
	channel3->index = NS_PIXEL_CHANNEL_ALPHA;

	info->channel  = channel0;
	channel0->next = channel1;
	channel1->next = channel2;
	channel2->next = channel3;
	channel3->next = NULL;


	/**********************************/
	info = _ns_pixel_infos + NS_PIXEL_BGR_U8_U8_U8;

	info->pixel_type   = NS_PIXEL_BGR_U8_U8_U8;
	info->format_type  = NS_PIXEL_FORMAT_BGR;
	info->num_channels = 3;

	channel0 = info->channels + NS_PIXEL_CHANNEL_BLUE;
	channel1 = info->channels + NS_PIXEL_CHANNEL_GREEN;
	channel2 = info->channels + NS_PIXEL_CHANNEL_RED;

	channel0->type  = NS_PIXEL_CHANNEL_UINT;
	channel0->bits  = 8;
	channel0->ubits = 8;
	channel0->name  = NS_PIXEL_CHANNEL_BLUE_NAME;
	channel0->index = NS_PIXEL_CHANNEL_BLUE;

	channel1->type  = NS_PIXEL_CHANNEL_UINT;
	channel1->bits  = 8;
	channel1->ubits = 8;
	channel1->name  = NS_PIXEL_CHANNEL_GREEN_NAME;
	channel1->index = NS_PIXEL_CHANNEL_GREEN;

	channel2->type  = NS_PIXEL_CHANNEL_UINT;
	channel2->bits  = 8;
	channel2->ubits = 8;
	channel2->name  = NS_PIXEL_CHANNEL_RED_NAME;
	channel2->index = NS_PIXEL_CHANNEL_RED;

	info->channel  = channel0;
	channel0->next = channel1;
	channel1->next = channel2;
	channel2->next = NULL;


	/**********************************/
	info = _ns_pixel_infos + NS_PIXEL_BGRA_U8_U8_U8_U8;

	info->pixel_type   = NS_PIXEL_BGRA_U8_U8_U8_U8;
	info->format_type  = NS_PIXEL_FORMAT_BGRA;
	info->num_channels = 4;

	channel0 = info->channels + NS_PIXEL_CHANNEL_BLUE;
	channel1 = info->channels + NS_PIXEL_CHANNEL_GREEN;
	channel2 = info->channels + NS_PIXEL_CHANNEL_RED;
	channel3 = info->channels + NS_PIXEL_CHANNEL_ALPHA;

	channel0->type  = NS_PIXEL_CHANNEL_UINT;
	channel0->bits  = 8;
	channel0->ubits = 8;
	channel0->name  = NS_PIXEL_CHANNEL_BLUE_NAME;
	channel0->index = NS_PIXEL_CHANNEL_BLUE;

	channel1->type  = NS_PIXEL_CHANNEL_UINT;
	channel1->bits  = 8;
	channel1->ubits = 8;
	channel1->name  = NS_PIXEL_CHANNEL_GREEN_NAME;
	channel1->index = NS_PIXEL_CHANNEL_GREEN;

	channel2->type  = NS_PIXEL_CHANNEL_UINT;
	channel2->bits  = 8;
	channel2->ubits = 8;
	channel2->name  = NS_PIXEL_CHANNEL_RED_NAME;
	channel2->index = NS_PIXEL_CHANNEL_RED;

	channel3->type  = NS_PIXEL_CHANNEL_UINT;
	channel3->bits  = 8;
	channel3->ubits = 8;
	channel3->name  = NS_PIXEL_CHANNEL_ALPHA_NAME;
	channel3->index = NS_PIXEL_CHANNEL_ALPHA;

	info->channel  = channel0;
	channel0->next = channel1;
	channel1->next = channel2;
	channel2->next = channel3;
	channel3->next = NULL;
	}
