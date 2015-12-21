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
// $Id: NodeSurface.cpp,v 1.15 2009/07/20 23:28:18 neeravp Exp $
//=============================================================================

#include "Precompiled.h"
#include "NodeSurface.h"

namespace FlyCapture2
{
    const double NodeSurface::sk_pi = 4.0 * atan(1.0);

    NodeSurface::NodeSurface( 
        Cairo::RefPtr<Cairo::ImageSurface> iconSurface,
        Glib::RefPtr<Gdk::Pixbuf> iconPixBuf )
    {
        m_initialized = false;
        m_iconSurface = iconSurface;
        m_iconPixBuf = iconPixBuf;
        m_xPos = 0;
        m_yPos = 0;
        m_deviceId = 0;  
        
        for ( int i=0; i < 4; i++ )
        {
            m_guid.value[i] = 0;
        }

        Initialize();
    }

    NodeSurface::~NodeSurface()
    {
    }

    void NodeSurface::Initialize()
    {
        // Create node image surface
        m_surface = Cairo::ImageSurface::create(
            Cairo::FORMAT_RGB24,
            sk_nodeWidth,
            sk_nodeHeight );        

        m_initialized = true;
    }

    void NodeSurface::Update( TopologyNode* pNode, bool selected )
    {
        if ( pNode == NULL )
        {
            return;
        }

        // Store the PGRGuid so we can handle device information on clicks
        m_guid = pNode->GetGuid();
        m_deviceId = pNode->GetDeviceId();

        // Create a context for the surface
        Cairo::RefPtr<Cairo::Context> refCairo = Cairo::Context::create( m_surface );
        int width = m_surface->get_width();
        int height=  m_surface->get_height();
        int radius = 10;        

        double red, green, blue;
        TopologyNode::NodeType nodeType = pNode->GetNodeType();
        GetNodeTypeColor( nodeType, red, green, blue );

        FillBackground( refCairo, 1.0, 1.0, 1.0 );

        DrawRoundedRectangle( 
            refCairo, 
            10, 
            10, 
            width-20, 
            height-20, 
            radius, 
            red, 
            green, 
            blue, 
            selected );

        DrawNodeText( refCairo, pNode, width/2, height/2 );

        //DrawIcon( refCairo, m_iconSurface, m_iconPixBuf, width/2, height/2 );

        DrawPorts( refCairo, pNode );
    }

    Cairo::RefPtr<Cairo::ImageSurface> NodeSurface::GetSurface()
    {
        return m_surface;
    }

    void NodeSurface::FillBackground( 
        Cairo::RefPtr<Cairo::Context> refCairo, 
        double red, 
        double green, 
        double blue )
    {
        // Get width / height of surface
        int surfaceWidth = m_surface->get_width();
        int surfaceHeight = m_surface->get_height();

        refCairo->set_source_rgb( red, green, blue );
        refCairo->rectangle( 0, 0, surfaceWidth, surfaceHeight );               
        refCairo->fill();
    }

    void NodeSurface::DrawRoundedRectangle( 
        Cairo::RefPtr<Cairo::Context> refCairo, 
        double x, 
        double y, 
        double width, 
        double height, 
        double radius,
        double red,
        double green,
        double blue,
        bool selected )
    {
        refCairo->save();

        if ( (radius > height/2.0) || (radius > width/2.0) )
        {
            radius = std::min(height / 2, width / 2);
        }

        refCairo->move_to( x, y + radius );
        refCairo->arc( x + radius, y + radius, radius, sk_pi, -sk_pi / 2.0 );
        refCairo->line_to( x + width - radius, y );
        refCairo->arc( x + width - radius, y + radius, radius, -sk_pi / 2.0, 0 );
        refCairo->line_to( x + width, y + height - radius );
        refCairo->arc( x + width - radius, y + height - radius, radius, 0, sk_pi / 2.0 );
        refCairo->line_to( x + radius, y + height );
        refCairo->arc( x + radius, y + height - radius, radius, sk_pi / 2.0, sk_pi );
        refCairo->close_path();

        refCairo->set_source_rgb( red, green, blue );
        refCairo->fill_preserve();

        if ( selected == true )
        {
            refCairo->set_source_rgb( 1.0, 0.0, 0.0 );
            refCairo->set_line_width( 2 );
            refCairo->stroke();   
        }
        else
        {
            refCairo->set_source_rgb( 0.0, 0.0, 0.0 );
            refCairo->set_line_width( 1 );
            refCairo->stroke();   
        }            

        refCairo->restore();
    }

    void NodeSurface::DrawNodeText( 
        Cairo::RefPtr<Cairo::Context> refCairo, 
        TopologyNode* pNode,
        int x, 
        int y )
    {
        refCairo->save();

        // Set the font parameters
        refCairo->select_font_face( 
            "monospace",
            Cairo::FONT_SLANT_NORMAL, 
            Cairo::FONT_WEIGHT_BOLD );
        refCairo->set_font_size( 24 );

        // Set draw color to black
        refCairo->set_source_rgb(0.0, 0.0, 0.0);  

        char idCaption[128];

        TopologyNode::NodeType nodeType = pNode->GetNodeType();        
        switch (nodeType)
        {
        case TopologyNode::COMPUTER:
            sprintf( idCaption, "PC" );
            break;

        case TopologyNode::BUS:
            {
                refCairo->set_font_size( 18 );

                InterfaceType interfaceType = pNode->GetInterfaceType();
                switch (interfaceType)
                {
                case INTERFACE_IEEE1394:
                    sprintf( idCaption, "1394 Bus" );
                    break;
                case INTERFACE_USB2:
                    sprintf( idCaption, "USB Bus" );
                    break;
				case INTERFACE_GIGE:
                    sprintf( idCaption, "GigE Bus" );
                    break;
                default:
                    sprintf( idCaption, "Bus" );
                    break;
                }                
            }            
            break;

        case TopologyNode::CAMERA:
            sprintf( idCaption, "ID: %d", pNode->GetDeviceId() );
            break;

        case TopologyNode::NODE:
            sprintf( idCaption, "Node" );
            break;
        }

        Cairo::TextExtents textExtents;
        refCairo->get_text_extents( idCaption, textExtents );
        refCairo->move_to( x - (textExtents.width / 2) , y + (textExtents.height / 2) );
        refCairo->show_text( idCaption );

        refCairo->restore();
    }

    void NodeSurface::DrawIcon( 
        Cairo::RefPtr<Cairo::Context> refCairo, 
        Cairo::RefPtr<Cairo::ImageSurface> refIconSurface,
        Glib::RefPtr<Gdk::Pixbuf> pixbufIcon,
        int x,
        int y )
    {
        refCairo->save();

        Cairo::RefPtr<Cairo::Context> refCairoIcon = Cairo::Context::create(refIconSurface);
        Gdk::Cairo::set_source_pixbuf( refCairoIcon, pixbufIcon, 0.0, 0.0 );
        refCairoIcon->paint();

        int width = pixbufIcon->get_width();
        int height = pixbufIcon->get_height();

        refCairo->set_source( refIconSurface, x - (width/2), y - (height/2) );
        refCairo->paint();

        refCairo->restore();
    }     

    void NodeSurface::DrawPorts( 
        Cairo::RefPtr<Cairo::Context> refCairo, 
        TopologyNode* pNode )
    {
        refCairo->save();

        const float k_divisor = 255.0;
        float red = 38 / k_divisor;
        float green = 199 / k_divisor;
        float blue = 38 / k_divisor;
        refCairo->set_source_rgb( red, green, blue );

        refCairo->set_source_rgb( 0, 0, 0 );

        refCairo->set_line_width( 5 );

        int numChildren = 0;
        for ( unsigned int i=0; i < pNode->GetNumPorts(); i++ )
        {
            if ( pNode->GetPortType(i) == TopologyNode::CONNECTED_TO_CHILD )
            {
                numChildren++;
            }
        }

        int childIndex = 0;
        for ( unsigned int i=0; i < pNode->GetNumPorts(); i++ )
        {
            TopologyNode::PortType currPort = pNode->GetPortType(i);

            switch (currPort)
            {
            case TopologyNode::NOT_CONNECTED:
                break;

            case TopologyNode::CONNECTED_TO_PARENT:                
                {
                    refCairo->move_to( sk_nodeWidth/2 - sk_portWidth/2, 0 );
                    refCairo->rel_line_to( sk_portWidth, 0 );
                    refCairo->rel_line_to( 0, sk_portHeight );
                    refCairo->rel_line_to( -sk_portWidth, 0 );
                    refCairo->fill();
                }
                break;

            case TopologyNode::CONNECTED_TO_CHILD:
                {
                    int startX = (sk_nodeWidth / (numChildren+1)) * (childIndex+1);

                    refCairo->move_to( startX - sk_portWidth/2, sk_nodeHeight );
                    refCairo->rel_line_to( sk_portWidth, 0 );
                    refCairo->rel_line_to( 0, -sk_portHeight );
                    refCairo->rel_line_to( -sk_portWidth, 0 );
                    refCairo->fill();                    
 
                    childIndex++;
                }
                break;

            default:
                break;

            }
        }

        refCairo->restore();
    }

    void NodeSurface::GetPosition( int& xPos, int& yPos )
    {
        xPos = m_xPos;
        yPos = m_yPos;
    }

    void NodeSurface::SetPosition( int xPos, int yPos )
    {
        m_xPos = xPos;
        m_yPos = yPos;
    }

    void NodeSurface::GetDimensions( int& width, int& height )
    {
        width = sk_nodeWidth;
        height=  sk_nodeHeight;
    }

    void NodeSurface::GetPortDimensions( int& width, int& height )
    {
        width = sk_portWidth;
        height = sk_portHeight;
    }

    PGRGuid NodeSurface::GetGuid()
    {
        return m_guid;
    }   

    void NodeSurface::GetNodeTypeColor( 
        TopologyNode::NodeType nodeType, 
        double& red, 
        double& green, 
        double& blue )
    {        
        switch (nodeType)
        {
        case TopologyNode::COMPUTER:
            red = 154;
            green = 184;
            blue = 210;
            break;

        case TopologyNode::BUS:
            red = 153;
            green = 0;
            blue = 0;
            break;

        case TopologyNode::CAMERA:
            red = 197;
            green = 210;
            blue = 200;
            break;

        case TopologyNode::NODE:
            red = 209;
            green = 148;
            blue = 12;
            break;
        }

        const double k_divisor = 255.0;
        red /= k_divisor;
        green /= k_divisor;
        blue /= k_divisor;
    }
}
