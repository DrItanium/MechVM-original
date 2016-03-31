////////////////////////////////////////////////////////////////////////////////
// GLTableView.h
// Copyright Bjoern Ganster 2007
////////////////////////////////////////////////////////////////////////////////

#ifndef GLTableView__H
#define GLTableView__H

#include "GLWindow.h"
#include "BGString.h"
#include "GLScrollbar.h"

#include <vector>
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Abstract base class for TableView data sources

class GLTableViewDataSource: public VirtualClass {
public:
   // Destructor
   virtual ~GLTableViewDataSource ()
   {}

   virtual void getDimensions (size_t senderID, size_t& rowCount, size_t& colCount) = 0;

   // Get string for a cell
   virtual bool getString (size_t senderID, int row, int col, BGString& str) = 0;

   // Request context menu
   virtual void showContextMenu(VirtualClass* /*sender*/, int /*x*/, int /*y*/)
   {
   }
};

////////////////////////////////////////////////////////////////////////////////
// Base class for a table view

class GLTableView: public GLWindow {
public:
   Color textColor, selectionColor;

   // Constructor
   GLTableView (GLWindow* _parent = NULL, int _left = 0, int _top = 0, 
                int _width = 0, int _height = 0, int _ID = 0);

   // Destructor
   virtual ~GLTableView();

   // Set data source
   void setDataSource (GLTableViewDataSource* ds)
   { dataSource = ds; }

   // Get/Set column/row count
   void setColWidth (size_t num, size_t newVal, size_t defaultWidth = 50);

   //void setRowHeight (size_t num, size_t newVal, size_t defaultHeight = 15);

   // Set a single height for all rows
   void setRowHeights (size_t num, size_t newVal);

   // Render
   virtual void render(int x, int y, int w, int h);

   // Keyboard Events
   virtual void keypressed (BGKey key, bool /*shiftHeld*/, bool /*ctrlHeld*/, bool down);
   //virtual void specialKeyPressed (char key, bool /*shiftHeld*/, bool /*ctrlHeld*/);

   // Button state changed over a window: returns child that was clicked
   virtual GLWindow* buttonStateChanged (int /*button*/, bool down, 
                                         int /*x*/, int /*y*/);
   
   virtual void mouseMoved (int /*x*/, int y);

   inline int getSelectedRow() const
   { return selectedRow; }
   inline void setSelectedRow(int newVal)
   { selectedRow = newVal; }

   inline void clearSelection()
   { selectedRow = -1; }

protected:
   vector <size_t> colStart, rowStart;
   GLTableViewDataSource* dataSource;
   GLScrollbar* vScrollbar;
   int selectedRow;
   size_t ID;

   // Find selected row
   void findSelectedRow (int y);
};

#endif
