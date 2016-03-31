////////////////////////////////////////////////////////////////////////////////
// GLTableView.cpp
// Copyright Bjoern Ganster 2007
////////////////////////////////////////////////////////////////////////////////

#include "GLTableView.h"

////////////////////////////////////////////////////////////////////////////////
// Constructor

GLTableView::GLTableView (GLWindow* _parent, int _left, int _top, 
             int _width, int _height, int _ID)
: GLWindow (_parent, _left, _top, _width, _height),
  dataSource (NULL),
  textColor (0, 0, 0),
  selectionColor (1, 0, 0),
  selectedRow (-1),
  vScrollbar (NULL),
  ID (_ID)
{
}

////////////////////////////////////////////////////////////////////////////////
// Destructor

GLTableView::~GLTableView()
{
}

////////////////////////////////////////////////////////////////////////////////
// Get/Set column/row count

void GLTableView::setColWidth (size_t num, size_t newVal, size_t defaultWidth)
{
   // Store old size
   size_t oldVal = 0;
   if (num+1 < colStart.size())
      oldVal = colStart[num+1] - colStart[num];

   // Add columns before num
   size_t i = colStart.size();
   while (i < num+1) {
      colStart.push_back (i*defaultWidth);
      ++i;
   }

   // Adjust columns after num
   int delta = newVal - oldVal;
   i = num+1;
   while (i < colStart.size()) {
      colStart[i] += delta;
      i++;
   }
}

// More flexible, but buggy: some lines are sometimes set to zero width
/*void GLTableView::setRowHeight (size_t num, size_t newVal, size_t defaultHeight)
{
   // Store old size
   size_t oldVal = 0;
   size_t rowCount = rowStart.size();
   if (num+1 < rowCount)
      oldVal = rowStart[num+1] - rowStart[num];

   // Add rows before num
   size_t i = 0, y = 0;
   if (rowCount > 0)
      y = rowStart[rowCount-1];
   while (rowStart.size() < num+2) {
      rowStart.push_back (y+i*defaultHeight);
      ++i;
   }

   // Adjust columns after num
   int delta = newVal - oldVal;
   i = num+1;
   while (i < rowStart.size()) {
      rowStart[i] += delta;
      i++;
   }
}*/

// Set a single height for all rows
void GLTableView::setRowHeights (size_t num, size_t newVal)
{
   rowStart.resize(num+1);
   for (size_t i=0; i < rowStart.size(); i++)
      rowStart[i] = i*newVal;
}

////////////////////////////////////////////////////////////////////////////////

void GLTableView::findSelectedRow (int y)
{
   size_t colCount, rowCount;
   dataSource->getDimensions(ID, rowCount, colCount);

   y -= top-5;
   selectedRow = 0;
   while (selectedRow < (int)rowCount && y > (int)rowStart[selectedRow+1])
      selectedRow++;

   if (vScrollbar != NULL)
      selectedRow += vScrollbar->getStart();
}

////////////////////////////////////////////////////////////////////////////////
// Mouse events

GLWindow* GLTableView::buttonStateChanged (int button, bool down, 
   int x, int y)
{ 
   findSelectedRow (y);
   if (button == 1 && !down)
      sendMessage (listener, TableItemSelected);
   if (button == 3 && down)
      dataSource->showContextMenu(this, x, y);
   return this;
}

void GLTableView::mouseMoved (int /*x*/, int y)
{
   //findSelectedRow (y);
}

////////////////////////////////////////////////////////////////////////////////
// Keyboard Events

void GLTableView::keypressed (BGKey key, bool /*shiftHeld*/, bool /*ctrlHeld*/, 
                              bool down)
{
   if (key.sym == 13) {
//      if (offset == 0x18)
         sendMessage (listener, TableItemSelected);
   } else {
      BGString msg = "GLTableView: Key pressed - ";
      msg.appendInt(key.sym);
      msg.printLn();
   }
}

/*void GLTableView::specialKeyPressed (char key, bool shiftHeld, bool ctrlHeld)
{
   size_t colCount, rowCount;
   dataSource->getDimensions(ID, rowCount, colCount);

   if (key == 101 && selectedRow > 0) {
      selectedRow--;
   } else if (key == 103 && selectedRow < ((int) rowCount)-2) {
      selectedRow++;
   } else {
      BGString msg = "Special key pressed: ";
      msg.appendInt(key);
      msg.printLn();
   }
}*/

////////////////////////////////////////////////////////////////////////////////
// Render

void GLTableView::render(int wx, int wy, int w, int h)
{
   glDisable (GL_TEXTURE_2D);
   glDisable (GL_DEPTH_TEST);
   prepareToRender(wx, wy, w, h);

   int textTop = (height - 17) / 2;
   glTranslated (-1.0, 1.0, 0.0);
   glScaled (2/((double) getWidth()), -2/((double) getHeight()), 1.0);

   // Render texts
   if (dataSource != NULL) {
      size_t colCount, rowCount;
      dataSource->getDimensions(ID, rowCount, colCount);
      if (rowCount == 0)
         return;

      // Create or destroy scrollbar
      size_t maxRowHeight = rowCount * rowStart[1];
      size_t row = 0, startY = 0;
      if ((int) maxRowHeight > height && vScrollbar == NULL) {
         vScrollbar = new GLScrollbar (parent);
         const size_t sbw = 20;
         vScrollbar->setGeometry(left+width, top, sbw, height);
         vScrollbar->setMax(rowCount);
         vScrollbar->setSize(rowCount * rowStart[1] / height);
         //add (vScrollbar);
         parent->add (vScrollbar);
      } else if (vScrollbar != NULL) {
         if ((int) maxRowHeight < height) {
            delete vScrollbar;
            vScrollbar = NULL;
         } else {
            row = vScrollbar->getStart();
            startY = rowStart[row];
         }
      }

      size_t y = rowStart[row]-startY;
      while (row < rowCount && (int) y < height) {
         // Highlight selection
         y = rowStart[row]-startY;
         if (row == selectedRow) {
            int nextY = y;
            if (row < rowCount-1)
               nextY = rowStart[row+1]-startY;
            else
               nextY = height;
            selectionColor.glColor();
            glBegin(GL_POLYGON);
               glVertex2i(0, y);
               glVertex2i(width, y);
               glVertex2i(width, nextY);
               glVertex2i(0, nextY);
            glEnd ();
         }

         // Render cell text
         for (size_t col = 0; col < colCount; col++) {
            int x = colStart[col];
            BGString str;
            dataSource->getString (ID, row, col, str);
            textColor.glColor();
            renderText (str.getChars(), x, y, width);
         }
         row++;
      }

      if (vScrollbar != NULL) {
         vScrollbar->setMax(rowCount);
         vScrollbar->render(wx, wy, w, h);
      }
   }
}
