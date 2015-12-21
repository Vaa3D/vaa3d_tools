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
// $Id: BusTopologyDrawingArea.h,v 1.13 2009/04/22 16:32:56 soowei Exp $
//=============================================================================

#include "FlyCapture2.h"
#include "NodeSurface.h"

namespace FlyCapture2
{   
    class TopologyNode;

    /**
     * This class inherits from Gtk::DrawingArea and performs custom drawing
     * to render a bus topology.
     */ 
    class BusTopologyDrawingArea : public Gtk::DrawingArea
    {
    public:
        
        /** Constructor. Needed to inherit from Gtk::DrawingArea. */
        BusTopologyDrawingArea(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);

        /** Destructor. */
        virtual ~BusTopologyDrawingArea();

        /**
         * Redraw the bus topology using the specified node.
         *
         * @param TopologyNode * pNode
         */
        void Redraw( TopologyNode* pNode ); 

        /**
         * Get a signal that can be used to receive events when the node
         * selection has been changed.
         *
         * @return Signal that can be connected to in order to receive
         *         node selection change events.
         */
        sigc::signal<void, PGRGuid> signal_node_selection_changed();

    protected:
        /**
         * Implementation of on_expose_event() to draw the topology
         * to the drawing area.
         *
         * @param event The expose event.
         */
        virtual bool on_expose_event(GdkEventExpose* event);

        /** Node selection change signal. */
        sigc::signal<void, PGRGuid> m_node_selection_changed;

    private:     

        /**
         * Contains coordinates for a line to be drawn, usually between
         * two nodes.
         */ 
        struct Line
        {
            int startX;
            int startY;
            int endX;
            int endY;

            Line() 
            {
                this->startX = 0;
                this->startY = 0;
                this->endX = 0;
                this->endY = 0;
            }

            Line( int startX, int startY, int endX, int endY )
            {
                this->startX = startX;
                this->startY = startY;
                this->endX = endX;
                this->endY = endY;
            }
        };

        /** Multiplier used to generate original image surface area. */
        static const int sk_multiplier = 8; 

        /** Vertical spacing between nodes. */
        static const int sk_vertSpacing = 40;

        /** Current aspect ratio. */
        double m_currAspectRatio;

        /** ImageSurface where everything will be drawn to. */
        Cairo::RefPtr<Cairo::ImageSurface> m_refMainSurface;  

        /** Icon related data, usually passed into NodeSurface. */
        Cairo::RefPtr<Cairo::ImageSurface> m_iconSurface;
        Glib::RefPtr<Gdk::Pixbuf> m_iconPixBuf;
        
        /** NodeSurfaces that will be drawn on the main ImageSurface. */
        std::vector<NodeSurface> m_vecNodeSurfaces;

        /** Lines that will be drawn on the main ImageSurface. */
        std::vector<Line> m_vecLines;

        /** Pointer to the first node to start drawing from (usually the PC). */
        TopologyNode* m_pBaseNode;

        /** Width of the main ImageSurface used, in pixels. */
        int m_usedWidth;

        /** Height of the main ImageSurface used, in pixels. */
        int m_usedHeight;

        /** X offset from top left of drawing area to place ImageSurface. */
        int m_offsetX;

        /** Y offset from top left of drawing area to place ImageSurface. */
        int m_offsetY;

        /** Whether the current selection is a node with a valid PGRGuid. */
        bool m_selectionValid;

        /** PGRGuid of the current selected node. */
        PGRGuid m_selectedGuid;

        /** Whether the tree should be redrawn. */
        bool m_redrawTree;

        /**
         * Initializes various data such as the ImageSurface when the
         * BusTopologyDrawingArea is realized. This is not done in the 
         * constructor because the size of the drawing area is needed.
         */ 
        void OnRealize();

        /**
         * Get the size of the BusTopologyDrawingArea.
         *
         * @param int & width
         * @param int & height
         */
        void GetWidgetSize( int& width, int& height );

        /** Render the topology. */
        void DrawTree();   

        /**
         * Parse a single node. This is called recursively to render all
         * topology nodes.
         *
         * @param pNode The node to parse.
         * @param width Width available to work with.
         * @param height Height available to work with.
         * @param horzOffset X offset of the node from top left.
         * @param vertOffset Y offset of the node from top left.
         *
         * @return int Additional width that was used to parse node.
         */
        int ParseNode( 
            TopologyNode* pNode,
            int width, 
            int height, 
            int horzOffset, 
            int vertOffset );

        /**
         * Draw a single node connector line as specified by the argument.
         *
         * @param refCairo The Cairo context to use.
         * @param line Coordinates of line to draw.
         */
        void DrawNodeConnector(
            Cairo::RefPtr<Cairo::Context> refCairo,
            Line line );

        /**
         * Fill the background of the specified Cairo context with the
         * specified color.
         *
         * @param refCairo The Cairo context to use.
         * @param red The red color to use (0-1).
         * @param green The green color to use (0-1).
         * @param blue The blue color to use (0-1).
         */
        void FillBackground(
            Cairo::RefPtr<Cairo::Context> refCairo,
            double red,
            double green,
            double blue );

        /**
         * Count the depth of the topology. This is called recursively.
         *
         * @param pNode The node to calculate from.
         * @param seed The current level. This argument should be 1 when
         *             beginning the calculation.
         *
         * @return int The depth of the topology.
         */
        static int CountLevels( TopologyNode* pNode, int seed = 1 );

        /**
         * Count the number of second generation children to help with
         * width calculations.
         *
         * @param pNode The node to use for calculation.
         *
         * @return int Number of second generation children.
         */
        static int CountNum2ndGenChildren( TopologyNode* pNode );

        /**
         * Checks if a PGRGuid is valid. It is considered valid if any
         * of the internal values are non-zero.
         *
         * @param guid The PGRGuid to check.
         *
         * @return bool Whether the PGRGuid is valid.
         */
        static bool IsGuidValid( PGRGuid guid );

        /**
         * Check if 2 PGRGuids are equal.
         *
         * @param guid1 First PGRGuid to compare.
         * @param guid2 Second PGRGuid to compare.
         *
         * @return bool Whether the PGRGuids are equal.
         */
        static bool AreGuidsEqual( PGRGuid guid1, PGRGuid guid2 );

        /** Helper function to clear node and node connector vectors. */
        void ClearVectors();

        /**
         * Insert a new TopologyNode into the vector of nodes to be drawn.
         *
         * @param pNode The TopologyNode to insert.
         * @param xPos X position where the node should be placed.
         * @param yPos Y position where the node should be placed.
         */
        void InsertNewNodeSurface( TopologyNode* pNode, int xPos, int yPos );

        /**
         * Insert a new node connector line to be drawn.
         *
         * @param newLine The Line to insert.
         */
        void InsertNewNodeConnector( Line newLine );

        /**
         * Event handler for handling mouse clicks.
         *
         * @param event Data about the event.
         *
         * @return bool Whether to continue to propagate event handling.
         */
        bool on_button_press_event(GdkEventButton* event);
    };    
}
