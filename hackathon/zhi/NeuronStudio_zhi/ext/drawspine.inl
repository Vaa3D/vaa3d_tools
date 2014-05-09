
int DrawSpine
	(
	HDC hdc,
	const char *type,
	float headdiam,
	float neckdiam,
	float basewidth,
	float length,
	float angle,
	int windowx,
	int windowy,
	float fieldsize
	)
	{
	RECT rect;
	RECT client;
	float res;
	float dend;// = 0.25f; //portion of window showing dendrite
	POINT base[4];
	SIZE size;
	int bk_mode;
	//COLORREF text_color;
	HGDIOBJ font;
	HGDIOBJ pen;
	LONG h;
	LONG a;
	LONG cx, cy, px, py, ax, ay;


	//dend = 2.0f * 0.25f / fieldsize;
	dend = 0.1f;

	//compute resolution based on fieldsize;
	res = fieldsize/windowx;

	// draw a gray rectangle over window
	rect.left=0;
	rect.top=0;
	rect.right=windowx;
	rect.bottom=windowy;
	FillRect(hdc,&rect,(HBRUSH)(COLOR_3DSHADOW+1)/*(COLOR_BTNFACE+1)*/);

	client = rect;

	// draw a solid bar at the bottom to represent dendrite
	rect.top=(LONG)(windowy*(1.0-dend));
	FillRect(hdc,&rect,(HBRUSH)GetStockObject( WHITE_BRUSH )/*(COLOR_WINDOWTEXT+1)*/);
	FrameRect(hdc,&rect,(HBRUSH)GetStockObject( BLACK_BRUSH )/*(COLOR_WINDOWTEXT+1)*/);

	//draw spine neck
	if( neckdiam > headdiam )
		{			
		base[0].x = (LONG)(windowx/2-headdiam/2/res);
		base[0].y = (LONG)((windowy*(1-dend))-length/res+headdiam/2/res);
		base[1].x = (LONG)(windowx/2+(headdiam/2/res)-1);
		base[1].y = (LONG)((windowy*(1-dend))-length/res+headdiam/2/res);
		base[2].x = (LONG)(windowx/2+neckdiam/2/res);
		base[2].y = (LONG)((windowy*(1-dend))-(length-headdiam)/res/3);
		base[3].x = (LONG)(windowx/2-neckdiam/2/res);
		base[3].y = (LONG)((windowy*(1-dend))-(length-headdiam)/res/3);

		if( (LONG)((windowy*(1-dend))-(length-headdiam)/res/3) > (LONG)(windowy*(1.0-dend)) )
			{
			base[2].y = (LONG)(windowy*(1.0-dend));
			base[3].y = (LONG)(windowy*(1.0-dend));
			}

		Polygon(hdc,base,4);
		}
	else
		{
		rect.left =(LONG)(windowx/2-neckdiam/2/res);
		rect.top =(LONG)((windowy*(1-dend))-length/res+headdiam/2/res);
		rect.right =(LONG)(windowx/2+neckdiam/2/res);
		rect.bottom =(LONG)(windowy*(1.0-dend));

		FillRect(hdc,&rect,(HBRUSH)GetStockObject( WHITE_BRUSH )/*(COLOR_WINDOWTEXT+1)*/);
		FrameRect(hdc,&rect,GetStockObject(BLACK_BRUSH));
		}

	//draw base of spine
	if( (LONG)((windowy*(1-dend))-(length-headdiam)/res/3) < (LONG)(windowy*(1.0-dend)) )
		{
		base[0].x = (LONG)(windowx/2-neckdiam/2/res);
		base[0].y = (LONG)((windowy*(1-dend))-(length-headdiam)/res/3);
		base[1].x = (LONG)(windowx/2+(neckdiam/2/res)-1);
		base[1].y = (LONG)((windowy*(1-dend))-(length-headdiam)/res/3);
		base[2].x = (LONG)(windowx/2+basewidth/2/res);
		base[2].y = (LONG)(windowy*(1.0-dend));
		base[3].x = (LONG)(windowx/2-basewidth/2/res);
		base[3].y = (LONG)(windowy*(1.0-dend));
		Polygon(hdc,base,4);
		}

	// draw head of spine
	Ellipse(
		hdc,
		(LONG)((windowx/2)-headdiam/2/res),
		(LONG)((windowy*(1-dend))-length/res),
		(LONG)((windowx/2)+headdiam/2/res),
		(LONG)((windowy*(1-dend))-length/res+headdiam/res)
		);

	h  = windowy / 8;
	a  = h / 3;
	cx = 8;
	cy = 8 + h;

	MoveToEx( hdc, cx, cy, NULL );
	LineTo( hdc, cx, cy - h );
	MoveToEx( hdc, cx, cy, NULL );
	LineTo( hdc, cx, cy + h );
	MoveToEx( hdc, cx, cy, NULL );
	LineTo( hdc, cx + h, cy );
	Arc( hdc, cx - 5, cy - 5, cx + 5 + 1, cy + 5 + 1, cx, cy + 5, cx, cy - 5 );

	pen = SelectObject( hdc, GetStockObject( WHITE_PEN ) );

	px = cx + ( LONG )( h * ns_cos( NS_DEGREES_TO_RADIANS( angle ) ) + 0.5 );
	py = cy - ( LONG )( h * ns_sin( NS_DEGREES_TO_RADIANS( angle ) ) + 0.5 );
	MoveToEx( hdc, cx, cy, NULL );
	LineTo( hdc, px, py );

	ax = px + ( LONG )( a * ns_cos( NS_DEGREES_TO_RADIANS( angle + 135.0 ) ) + 0.5 );
	ay = py - ( LONG )( a * ns_sin( NS_DEGREES_TO_RADIANS( angle + 135.0 ) ) + 0.5 );
	MoveToEx( hdc, px, py, NULL );
	LineTo( hdc, ax, ay );

	ax = px + ( LONG )( a * ns_cos( NS_DEGREES_TO_RADIANS( angle + 225.0 ) ) + 0.5 );
	ay = py - ( LONG )( a * ns_sin( NS_DEGREES_TO_RADIANS( angle + 225.0 ) ) + 0.5 );
	MoveToEx( hdc, px, py, NULL );
	LineTo( hdc, ax, ay );

	SelectObject( hdc, pen );

	GetTextExtentPoint32( hdc, type, ( int )ns_ascii_strlen( type ), &size );

	bk_mode    = GetBkMode( hdc );
	//text_color = GetTextColor( hdc );
	font       = SelectObject( hdc, GetStockObject( SYSTEM_FONT ) );

	SetBkMode( hdc, TRANSPARENT );
	//SetTextColor( hdc, RGB( 255, 255, 255 ) );

	TextOut(
		hdc,
		windowx / 2 - size.cx / 2,
		//deleted ?
		//	size.cy * 2
		//	:
			( LONG )( windowy * ( 1.0 - dend / 2.0 ) ) - size.cy / 2,
		type,
		( int )ns_ascii_strlen( type )
		);

	SetBkMode( hdc, bk_mode );
	//SetTextColor( hdc, text_color );
	SelectObject( hdc, font );

	FrameRect(hdc,&client,(HBRUSH)GetStockObject( BLACK_BRUSH ) );

	return 0;
	}
