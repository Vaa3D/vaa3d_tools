//=============================================================================
// Copyright © 2008 Point Grey Research, Inc. All Rights Reserved.
//
// This software is the confidential and proprietary information of Point
// Grey Research, Inc. ("Confidential Information").  You shall not
// disclose such Confidential Information and shall use it only in
// accordance with the terms of the license agreement you entered into
// with PGR.
//
// PGR MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
// SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, OR NON-INFRINGEMENT. PGR SHALL NOT BE LIABLE FOR ANY DAMAGES
// SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
// THIS SOFTWARE OR ITS DERIVATIVES.
//=============================================================================
//=============================================================================
// $Id: BusTopologyDrawingArea.cpp,v 1.17 2009/06/11 00:38:53 soowei Exp $
//=============================================================================

#include "Precompiled.h"
#include "BusTopologyDrawingArea.h"

namespace FlyCapture2
{        
    BusTopologyDrawingArea::BusTopologyDrawingArea( BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& /*refGlade*/ )
        : Gtk::DrawingArea(cobject)
    {
        Gdk::EventMask mask = Gdk::BUTTON_PRESS_MASK;
        set_events( mask ); 

        signal_realize().connect( sigc::mem_fun( *this, &BusTopologyDrawingArea::OnRealize ));        

        m_pBaseNode = NULL;
        m_currAspectRatio = 0.0;      

        m_usedWidth = 0;
        m_usedHeight = 0;

        m_offsetX = 0;
        m_offsetY = 0;

        m_redrawTree = false;

        m_selectedGuid.value[0] = 0;
        m_selectedGuid.value[1] = 0;
        m_selectedGuid.value[2] = 0;
        m_selectedGuid.value[3] = 0;
        
        m_selectionValid = false;
    }

    BusTopologyDrawingArea::~BusTopologyDrawingArea()
    {
    }

    void BusTopologyDrawingArea::Redraw( TopologyNode* pNode )
    {        
        if ( pNode != NULL )
        {
            m_pBaseNode = pNode;
            m_redrawTree = true;
            queue_draw();
        }        
    }
    
    bool BusTopologyDrawingArea::on_expose_event( GdkEventExpose* event )
    {
        Glib::RefPtr<Gdk::Window> window = get_window();
        if( window == NULL)
        {
            return true;
        }

        if ( m_pBaseNode == NULL )
        {
            return true;
        }
        
        Cairo::RefPtr<Cairo::Context> refCairo = window->create_cairo_context();

        // clip to the area indicated by the expose event so that we only redraw
        // the portion of the window that needs to be redrawn
        refCairo->rectangle(
            event->area.x, 
            event->area.y,
            event->area.width, 
            event->area.height);
        refCairo->clip();   

        FillBackground( refCairo, 1.0, 1.0, 1.0 );

        if ( m_redrawTree == true )
        {          
            DrawTree();

            m_redrawTree = false;
        }        
       
        // Get width / height of widget
        int widgetWidth, widgetHeight;
        GetWidgetSize( widgetWidth, widgetHeight );        

        double finalRatio = 1 / static_cast<double>(sk_multiplier);

        double widthRatio = m_usedWidth / static_cast<double>(widgetWidth);
        double heightRatio = m_usedHeight / static_cast<double>(widgetHeight); 
        
        finalRatio = 1 / std::max( widthRatio, heightRatio );

        // Scale the surface to fit on the widget
        refCairo->scale( finalRatio, finalRatio );
        m_currAspectRatio = finalRatio;

        int finalWidth = static_cast<int>(finalRatio * m_usedWidth);
        int finalHeight = static_cast<int>(finalRatio * m_usedHeight);

        m_offsetX = (widgetWidth/2) - (finalWidth/2);
        m_offsetY = (widgetHeight/2) - (finalHeight/2);

        // Draw ImageSurface to Cairo context
        refCairo->set_source( m_refMainSurface, m_offsetX, m_offsetY );

        // Paint it
        refCairo->paint();  

        return true;
    }    

    bool BusTopologyDrawingArea::on_button_press_event( GdkEventButton* event )
    {
        gdouble startX = event->x;
        gdouble startY = event->y;
        
        switch ( event->button )
        {
        case 1:    
            {                
                double actualX = startX * (1 / m_currAspectRatio);
                double actualY = startY * (1 / m_currAspectRatio);

                // Get the node surface width
                int surfaceWidth, surfaceHeight;
                NodeSurface::GetDimensions( surfaceWidth, surfaceHeight );

                actualX += (surfaceWidth / 2) - m_offsetX;
                actualY += (surfaceHeight / 2) - m_offsetY;

                m_selectedGuid.value[0] = 0;
                m_selectedGuid.value[1] = 0;
                m_selectedGuid.value[2] = 0;
                m_selectedGuid.value[3] = 0;
                m_selectionValid = false;

                for ( unsigned int i=0; i < m_vecNodeSurfaces.size(); i++ )
                {
                    int xPos, yPos;
                    m_vecNodeSurfaces[i].GetPosition( xPos, yPos );
                    PGRGuid currGuid = m_vecNodeSurfaces[i].GetGuid();

                    if ( actualX >= xPos &&
                        actualX <= xPos + surfaceWidth &&
                        actualY >= yPos &&
                        actualY <= yPos + surfaceHeight &&
                        IsGuidValid(currGuid) == true )
                    {
                        // Store the guid so we can draw something around
                        // the current selection
                        m_selectedGuid = currGuid;
                        m_selectionValid = true;        
                        
                        break;
                    }
                }

                // Fire off a guid. Fire an empty one if there was no
                // successful match
                if ( m_selectionValid == true )
                {
                    m_node_selection_changed( m_selectedGuid );
                }
                else
                {
                    PGRGuid blankGuid;
                    m_node_selection_changed( blankGuid );                    
                }

                m_redrawTree = true;
                queue_draw();             
            }
            break;
        default:
            break;
        }

        return true;
    }    

    void BusTopologyDrawingArea::OnRealize()
    {
        Glib::RefPtr<Gdk::Window> window = get_window();
        if( window == NULL)
        {
            return;
        }

        // Get width / height of widget
        int width, height;
        GetWidgetSize( width, height );

        int surfaceWidth = width * sk_multiplier;
        int surfaceHeight = height * sk_multiplier;

        m_usedWidth = width;
        m_usedHeight = height;

        // Create main image surface
        m_refMainSurface = Cairo::ImageSurface::create( 
            Cairo::FORMAT_RGB24, 
            surfaceWidth, 
            surfaceHeight );   

        // Create a temporary pix buf for resizing purposes
        Glib::RefPtr<Gdk::Pixbuf> tempPixBuf;
        tempPixBuf = Gdk::Pixbuf::create_from_inline( sizeof(PGRIcon), PGRIcon, false );
        int iconWidth = tempPixBuf->get_width();
        int iconHeight = tempPixBuf->get_height();

        // Scale the icon to be larger
        const int k_multiplier = 1;
        m_iconPixBuf = tempPixBuf->scale_simple( 
            iconWidth * k_multiplier, 
            iconHeight * k_multiplier, 
            Gdk::INTERP_BILINEAR );

        // Create icon image surface
        m_iconSurface = Cairo::ImageSurface::create(
            m_iconPixBuf->get_has_alpha() == true ? Cairo::FORMAT_ARGB32 : Cairo::FORMAT_RGB24,
            m_iconPixBuf->get_width(),
            m_iconPixBuf->get_height() );
    }

    void BusTopologyDrawingArea::GetWidgetSize( int& width, int& height )
    {
        get_window()->get_size( width, height );
    }

    void BusTopologyDrawingArea::DrawTree()
    {    
        Cairo::RefPtr<Cairo::Context> refCairo = Cairo::Context::create( m_refMainSurface );

        refCairo->save();

        // Get width / height of surface
        int surfaceWidth = m_refMainSurface->get_width();
        int surfaceHeight = m_refMainSurface->get_height();

        refCairo->set_source_rgb( 1.0, 1.0, 1.0 );       
        refCairo->rectangle( 0, 0, surfaceWidth, surfaceHeight );               
        refCairo->fill(); 

        // Get width / height of node surface
        int nodeSurfaceWidth, nodeSurfaceHeight;
        NodeSurface::GetDimensions( nodeSurfaceWidth, nodeSurfaceHeight );

        // Count the depth of the topology
        int numLevels = CountLevels( m_pBaseNode );
        int heightPerLevel = m_usedHeight / numLevels;
        if ( heightPerLevel < (nodeSurfaceHeight + sk_vertSpacing) )
        {
            m_usedHeight = numLevels * (nodeSurfaceHeight + sk_vertSpacing);
            heightPerLevel = m_usedHeight / numLevels;
        }        
        
        int overlap = -1;
        while ( overlap != 0 )
        {
            ClearVectors();
            
            overlap = ParseNode( m_pBaseNode, m_usedWidth, heightPerLevel, 0, 0 );
            m_usedWidth += overlap;
        }

        // Draw node surfaces
        for ( unsigned int i=0; i < m_vecNodeSurfaces.size(); i++ )
        {
            NodeSurface currNodeSurface = m_vecNodeSurfaces[i];
            Cairo::RefPtr<Cairo::ImageSurface> currImageSurface;
            currImageSurface = currNodeSurface.GetSurface();

            int xPos, yPos;
            currNodeSurface.GetPosition( xPos, yPos );

            xPos -= nodeSurfaceWidth / 2;
            yPos -= nodeSurfaceHeight / 2;

            refCairo->set_source( currImageSurface, xPos, yPos );
            refCairo->paint();
        }   

        // Draw lines
        for ( unsigned int i=0; i < m_vecLines.size(); i++ )
        {
            DrawNodeConnector( refCairo, m_vecLines[i] );
        }

        refCairo->restore();
    }   

    void BusTopologyDrawingArea::FillBackground( 
        Cairo::RefPtr<Cairo::Context> refCairo, 
        double red, 
        double green, 
        double blue )
    {
        refCairo->save();

        // Get width / height of drawing area
        int width, height;
        GetWidgetSize( width, height );   

        refCairo->set_source_rgb( red, green, blue );
        refCairo->rectangle( 0, 0, width, height );               
        refCairo->fill();

        refCairo->restore();
    }

    int BusTopologyDrawingArea::CountLevels( TopologyNode* pNode, int seed )
    {
        if( pNode == NULL )
        {
            return -1;
        }

        int result = seed;
        for( unsigned int i = 0; i < pNode->GetNumChildren(); i++ )
        {
            TopologyNode childNode = pNode->GetChild(i);
            int newSeed = CountLevels( &childNode, seed + 1 );
            if( newSeed > result ) 
            {
                result = newSeed;
            }
        }

        return result;
    }

    int BusTopologyDrawingArea::CountNum2ndGenChildren( TopologyNode* pNode )
    {
        int num2ndGenChildren = 0;
        for( unsigned int i = 0; i < pNode->GetNumChildren(); i++ )
        {
            num2ndGenChildren += static_cast<int>(pNode->GetChild(i).GetNumChildren());
            if( pNode->GetChild(i).GetNumChildren() == 0 )
            {
                num2ndGenChildren++;
            }
        }

        return num2ndGenChildren;
    }

    bool BusTopologyDrawingArea::IsGuidValid( PGRGuid guid )
    {
        if ( guid.value[0] != 0 &&
            guid.value[1] != 0 &&
            guid.value[2] != 0 &&
            guid.value[3] != 0 )
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool BusTopologyDrawingArea::AreGuidsEqual( PGRGuid guid1, PGRGuid guid2 )
    {
        if ( guid1.value[0] == guid2.value[0] &&
            guid1.value[1] == guid2.value[1] &&
            guid1.value[2] == guid2.value[2] &&
            guid1.value[3] == guid2.value[3] )
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    void BusTopologyDrawingArea::ClearVectors()
    {
        // Clear the vectors before parsing the nodes
        m_vecNodeSurfaces.clear();
        m_vecLines.clear();
    }

    void BusTopologyDrawingArea::InsertNewNodeSurface( TopologyNode* pNode, int xPos, int yPos )
    {
        NodeSurface nodeSurface( m_iconSurface, m_iconPixBuf );                      
        bool selected = false;
        PGRGuid nodeGuid = pNode->GetGuid();

        if ( AreGuidsEqual( nodeGuid, m_selectedGuid ) == true &&
            m_selectionValid == true )
        {
            selected = true;
        }

        nodeSurface.Update( pNode, selected );
        nodeSurface.SetPosition( xPos, yPos );

        m_vecNodeSurfaces.push_back( nodeSurface );
    }

    void BusTopologyDrawingArea::InsertNewNodeConnector( Line newLine )
    {
        m_vecLines.push_back( newLine );
    }

    int BusTopologyDrawingArea::ParseNode( 
        TopologyNode* pNode, 
        int width, 
        int height, 
        int horzOffset, 
        int vertOffset )
    {        
        // To improve the layout of the tree, the allocation of width is
        // determined by the number of second generation children rather than
        // just immediate children.
        int num2ndGenChildren = CountNum2ndGenChildren( pNode );

        int widthUnit = ( num2ndGenChildren == 0 ) ? width : width / num2ndGenChildren;
        int nextHorzOffset = horzOffset;

        // Calculate the position of the node
        int nodeXPos = horzOffset + (width/2);
        int nodeYPos = vertOffset + (height/2);

        // Add this node to the list of surfaces to be drawn        
        InsertNewNodeSurface( pNode, nodeXPos, nodeYPos );

        // Figure out there are any children to be recursively drawn
        int accumOverlap = 0;
        int childIndex = 0;
        for ( unsigned int portIndex=0; portIndex < pNode->GetNumPorts(); portIndex++ )
        {
            TopologyNode::PortType currPort = pNode->GetPortType(portIndex);

            if ( currPort ==  TopologyNode::CONNECTED_TO_CHILD )
            {                
                TopologyNode currChild = pNode->GetChild(childIndex);
                int thisWidth = widthUnit * static_cast<int>(currChild.GetNumChildren());
                if ( thisWidth == 0 )
                {
                    thisWidth = widthUnit;
                }

                int thisCenterX = nextHorzOffset + (thisWidth / 2);
                int thisCenterY = vertOffset + height + (height / 2);

                int surfaceWidth, surfaceHeight;
                NodeSurface::GetDimensions( surfaceWidth, surfaceHeight );

                // There might be more than 1 child, so perform some
                // calculations so that the lines don't start from
                // the same point           
                int numChildren = static_cast<int>(pNode->GetNumChildren());
                int startX = nodeXPos + 
                    ((surfaceWidth/(numChildren+1)) * (childIndex+1)) - 
                    (surfaceWidth / 2);
                int startY = nodeYPos + (surfaceHeight / 2);

                int endX = thisCenterX;
                int endY = thisCenterY - (surfaceHeight / 2);

                // Start a line to be drawn later
                Line newLine( startX, startY, endX, endY );
                InsertNewNodeConnector( newLine );                

                TopologyNode nextChild = pNode->GetChild(childIndex++);
                accumOverlap += ParseNode(
                    &nextChild,
                    thisWidth,
                    height,
                    nextHorzOffset,
                    vertOffset + height );

                nextHorzOffset += thisWidth;
            }                 
        }

        int nodeWidth, nodeHeight;
        NodeSurface::GetDimensions( nodeWidth, nodeHeight );     

        if ( width < nodeWidth )
        {
            accumOverlap += (nodeWidth - width);
        }

        if ( m_usedWidth < (horzOffset + width) )
        {
            accumOverlap += ((horzOffset + width) - m_usedWidth);
        }

        return accumOverlap;
    }

    void BusTopologyDrawingArea::DrawNodeConnector( Cairo::RefPtr<Cairo::Context> refCairo, Line line )
    {
        refCairo->save();

        refCairo->set_line_width( 5 );
        refCairo->set_source_rgb( 0, 0, 0 );

        int portWidth, portHeight;
        NodeSurface::GetPortDimensions( portWidth, portHeight );

        refCairo->move_to( line.startX, line.startY - (portHeight/2) );
        refCairo->rel_line_to( 0, (portHeight/2) + 5 );
        refCairo->line_to( line.endX, line.endY - 5 );
        refCairo->rel_line_to( 0, (portHeight/2) );
        refCairo->stroke();

        refCairo->restore();
    }

    sigc::signal<void, PGRGuid> BusTopologyDrawingArea::signal_node_selection_changed()
    {
        return m_node_selection_changed;
    }
}
