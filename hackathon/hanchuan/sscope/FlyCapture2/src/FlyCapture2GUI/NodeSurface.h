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
// $Id: NodeSurface.h,v 1.11 2009/06/11 00:38:53 soowei Exp $
//=============================================================================

#include "TopologyNode.h"

namespace FlyCapture2
{   
    /**
     * This class represents a surface that can be updated with information
     * from a TopologyNode. This information is used to render a representation
     * of the node. It can be retrieved and drawn on to a DrawingArea. The
     * x and y coordinates that are used when drawing the node to the 
     * DrawingArea can also be stored here.
     */ 
    class NodeSurface
    {
    public:
        /**
         * Constructor. Creates a new node surface with the specified icon and
         * icon pixbuf.
         *
         * @param iconSurface RefPtr to a ImageSurface containing an icon.
         * @param iconPixBuf RefPtr to a Pixbuf containing an icon.
         */ 
        NodeSurface( 
            Cairo::RefPtr<Cairo::ImageSurface> iconSurface,
            Glib::RefPtr<Gdk::Pixbuf> iconPixBuf );

        /** Default destructor. */
        virtual ~NodeSurface();
        
        /** 
         * Update image surface with data contained within the specified node.
         *
         * @param pNode The TopologyNode to use as data.
         * @param selected Whether the node is the currently selected node.
         */
        void Update( TopologyNode* pNode, bool selected = false );

        /**
         * Get the x and y coordinates for the node.
         *
         * @param xPos X coordinate of the node.
         * @param yPos Y coordinate of the node.
         */ 
        void GetPosition( int& xPos, int& yPos );

        /**
         * Set the x and y coordinates for the node.
         *
         * @param xPos X coordinate of the node.
         * @param yPos Y coordinate of the node.
         */ 
        void SetPosition( int xPos, int yPos );   

        /**
         * Get the dimensions of the node.
         *
         * @param width Width of the node.
         * @param height Height of the node.
         */ 
        static void GetDimensions( int& width, int& height );

        /**
         * Get the dimensions of a single port on the node.
         *
         * @param width Width of the port.
         * @param height Height of the port.
         */ 
        static void GetPortDimensions( int& width, int& height );

        /**
         * Get the PGRGuid associated with the node.
         *
         * @return The PGRGuid associated with the node.
         */ 
        PGRGuid GetGuid();

        /**
         * Get the ImageSurface for the node.
         *
         * @return A RefPtr pointing to an ImageSurface containing information
         *         the node.
         */ 
        Cairo::RefPtr<Cairo::ImageSurface> GetSurface();
    
    protected:        
    private:        
        static const double sk_pi;
        static const int sk_nodeWidth = 120;
        static const int sk_nodeHeight = 60;

        static const int sk_portWidth = 20;
        static const int sk_portHeight = 10;

        bool m_initialized;

        int m_xPos;
        int m_yPos;

        PGRGuid m_guid;
        
        int m_deviceId;

        /** The image surface that will be drawn on. */
        Cairo::RefPtr<Cairo::ImageSurface> m_surface;

        /** RefPtr to an image surface containing an icon. */
        Cairo::RefPtr<Cairo::ImageSurface> m_iconSurface;

        /** RefPtr to a PixBuf containing an icon. */
        Glib::RefPtr<Gdk::Pixbuf> m_iconPixBuf;

        /** Default constructor. */
        NodeSurface();

        /**
         * Fill the background of the node with the specified color.
         *
         * @param refCairo The Cairo context to use.
         * @param red The red value to use (0-1).
         * @param green The green value to use (0-1).
         * @param blue The blue value to use (0-1).
         */
        void FillBackground(
            Cairo::RefPtr<Cairo::Context> refCairo,
            double red,
            double green,
            double blue );

        /**
         * Draw a rounded rectangle that will define the outline of the node.
         *
         * @param refCairo The Cairo context to use.
         * @param x X offset from top left of node.
         * @param y Y offset from top left of node.
         * @param width Width of rectangle.
         * @param height Height of rectangle.
         * @param radius Radius of rounded corners.
         * @param red The red value to use (0-1).
         * @param green The green value to use (0-1).
         * @param blue The blue value to use (0-1).
         * @param selected Whether the node is currently selected.
         */
        void DrawRoundedRectangle(
            Cairo::RefPtr<Cairo::Context> refCairo,
            double x,
            double y,
            double width,
            double height,
            double radius,
            double red,
            double green,
            double blue,
            bool selected = false );

        /**
         * Draw text on the node. The text depends on the type of the node.
         * The text will be centered around the coordinates specified.
         *
         * @param refCairo The Cairo context to use.
         * @param pNode The node to retrieve information from.
         * @param x X offset from top left of node.
         * @param y Y offset from top left of node.
         */
        void DrawNodeText(
            Cairo::RefPtr<Cairo::Context> refCairo,
            TopologyNode* pNode,
            int x,
            int y );

        /**
         * Draw an icon on the node. The icon will be centered around the
         * coordinates specified.
         *
         * @param refCairo The Cairo context to use.
         * @param refIconSurface The icon surface to use.
         * @param pixbufIcon  The icon Pixbuf to use.
         * @param x X offset from top left of node.
         * @param y Y offset from top left of node.
         */
        void DrawIcon(
            Cairo::RefPtr<Cairo::Context> refCairo,
            Cairo::RefPtr<Cairo::ImageSurface> refIconSurface,
            Glib::RefPtr<Gdk::Pixbuf> pixbufIcon,
            int x,
            int y );

        /**
         * Draw ports for the node. Ports connected to parent are drawn on
         * top. Ports connected to children are drawn on the bottom.
         *
         * @param refCairo The Cairo context to use.
         * @param vecPorts Vector of ports to draw.
         */
        void DrawPorts(
            Cairo::RefPtr<Cairo::Context> refCairo,
            TopologyNode* pNode );
        
        /**
         * Initialize node surface for drawing.
         */
        void Initialize();
        
        /**
         * Get the color associated with a particular node type.
         *
         * @param nodeType The type of node to get the color for.
         * @param red The red color for the node type.
         * @param green The green color for the node type.
         * @param blue The blue color for the node type.
         */
        static void GetNodeTypeColor( 
            TopologyNode::NodeType nodeType, 
            double& red, 
            double& green, 
            double& blue );
    };
}
