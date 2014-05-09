
#define _INVERT_LINE_TEMPLATE( c1, c2 )\
	NsVector2f A, B, C;\
	nsint x1, y1, x2, y2;\
	\
	A.x = NS_TO_IMAGE_SPACE( P1->c1, ns_voxel_info_one_over_size_##c1( voxel_info ) );\
	A.y = NS_TO_IMAGE_SPACE( P1->c2, ns_voxel_info_one_over_size_##c2( voxel_info ) );\
	B.x = NS_TO_IMAGE_SPACE( P2->c1, ns_voxel_info_one_over_size_##c1( voxel_info ) );\
	B.y = NS_TO_IMAGE_SPACE( P2->c2, ns_voxel_info_one_over_size_##c2( voxel_info ) );\
	\
	C.x = ( nsfloat )dw->corner.x;\
	C.y = ( nsfloat )dw->corner.y;\
	\
	____TO_CLIENT_SPACE( dw, A, C );\
	____TO_CLIENT_SPACE( dw, B, C );\
	\
	x1 = ( nsint )A.x;\
	y1 = ( nsint )A.y;\
	x2 = ( nsint )B.x;\
	y2 = ( nsint )B.y;\
	\
	MoveToEx( dw->backBuffer.hDC, x1, y1, NULL );\
	LineTo( dw->backBuffer.hDC, x2, y2 )


/*
	RECT r;\
	r.left = x1 - 2; r.right  = r.left + 5;\
	r.top  = y1 - 2; r.bottom = r.top  + 5;\
	InvertRect( dw->backBuffer.hDC, &r );\
	r.left = x2 - 2; r.right  = r.left + 5;\
	r.top  = y2 - 2; r.bottom = r.top  + 5;\
	InvertRect( dw->backBuffer.hDC, &r )
*/


void _invert_do_line_xy
   (
   DisplayWindow      *dw,
	const NsVoxelInfo  *voxel_info,
   const NsVector3f   *P1,
   const NsVector3f   *P2
   )
   {  _INVERT_LINE_TEMPLATE( x, y );  }


void _invert_do_line_zy
   (
   DisplayWindow      *dw,
	const NsVoxelInfo  *voxel_info,
   const NsVector3f   *P1,
   const NsVector3f   *P2
   )
   {  _INVERT_LINE_TEMPLATE( z, y );  }


void _invert_do_line_xz
   (
   DisplayWindow      *dw,
	const NsVoxelInfo  *voxel_info,
   const NsVector3f   *P1,
   const NsVector3f   *P2
   )
   {  _INVERT_LINE_TEMPLATE( x, z );  }


#define _invert_line_init( dw )\
	HPEN hOldPen, hNewPen;\
	hNewPen     = CreatePen( PS_SOLID, dw->pen_thickness, 0x000000FF );\
	hOldPen     = SelectObject( dw->backBuffer.hDC, hNewPen )


	/*
nsint old_bk_mode, old_rop;\
	old_rop     = SetROP2( dw->backBuffer.hDC, R2_MERGEPENNOT/*R2_XORPEN*//* );\
	old_bk_mode = SetBkMode( dw->backBuffer.hDC, TRANSPARENT )
	*/


#define _invert_line_finalize( dw )\
	SelectObject( dw->backBuffer.hDC, hOldPen );\
	DeleteObject( hNewPen )

	/*
	SetBkMode( dw->backBuffer.hDC, old_bk_mode );\
	SetROP2( dw->backBuffer.hDC, old_rop )
	*/


void _invert_line_xy
   (
   DisplayWindow      *dw,
	const NsVoxelInfo  *voxel_info,
   const NsVector3f   *P1,
   const NsVector3f   *P2
   )
   {
	_invert_line_init( dw );
	_invert_do_line_xy( dw, voxel_info, P1, P2 );
	_invert_line_finalize( dw );
	}


void _invert_line_zy
   (
   DisplayWindow      *dw,
	const NsVoxelInfo  *voxel_info,
   const NsVector3f   *P1,
   const NsVector3f   *P2
   )
   {
	_invert_line_init( dw );
	_invert_do_line_zy( dw, voxel_info, P1, P2 );
	_invert_line_finalize( dw );
	}


void _invert_line_xz
   (
   DisplayWindow      *dw,
	const NsVoxelInfo  *voxel_info,
   const NsVector3f   *P1,
   const NsVector3f   *P2
   )
   {
	_invert_line_init( dw );
	_invert_do_line_xz( dw, voxel_info, P1, P2 );
	_invert_line_finalize( dw );
	}


void _invert_line
   (
   DisplayWindow      *dw,
	const NsVoxelInfo  *voxel_info,
   const NsVector3f   *P1,
   const NsVector3f   *P2
   )
	{
	switch( dw->display )
		{
		case NS_XY:
			_invert_line_xy( dw, voxel_info, P1, P2 );
			break;

		case NS_ZY:
			_invert_line_zy( dw, voxel_info, P1, P2 );
			break;

		case NS_XZ:
			_invert_line_xz( dw, voxel_info, P1, P2 );
			break;

		default:
			ns_assert_not_reached();
		}
	}
