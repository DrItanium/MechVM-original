////////////////////////////////////////////////////////////////////////////////
// Renderable object implementation
// Copyright Bjoern Ganster 2000-2007
////////////////////////////////////////////////////////////////////////////////

#include "RenderableObject.h"
#include "BGString.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#ifndef WIN32
#include <inttypes.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// Constructor

RenderableObject::RenderableObject (int classID, RenderableObject* parent, 
                                    const char* _name)
: VirtualClass (classID),
  visible (true), 
  name (NULL),
  NeedsToDetach (true)
{
	if (parent != NULL) {
      //addParent (parent); // done as part of instruction below
      parent->addChild (this);
   }

   if (_name != NULL) { 
      size_t namelen = strlen (_name)+1;
      name = new char [namelen];
      memcpy (name, _name, namelen);
   }
   
   #ifdef UseMemMan
   memman.store (this, sizeof (*this), _name);
   #endif
}

////////////////////////////////////////////////////////////////////////////////
// Destructor

RenderableObject::~RenderableObject () 
{
   // Delete childs
   // Do not use iterators to objects that will be removed
   while (childs.size() != 0) 
   {
      RenderableObject* other = (*childs.rbegin());
      // Speed up destruction by indicating that childs need not detach
      if (other != NULL) {
         //other->NeedNotDetach (); // necessary because of instancing!
         delete other;
         other = NULL;
      }
      childs.pop_back();
   }

   if (NeedsToDetach)
      detachObject();

   if (name != NULL)
      delete name;

   #ifdef UseMemMan
   memman.release (this);
   #endif

   #ifdef MATERIAL_SUPPORT
   for (size_t i = 0; i < parameters.size(); i++)
      delete parameters[i];
   parameters.clear();
   #endif
}

////////////////////////////////////////////////////////////////////////////////
// Return number of contained points

size_t RenderableObject::getRecursivePointCount () const
{ 
   size_t pc = 0;
   for (size_t i = 0; i < childs.size(); ++i) {
      pc += childs[i]->getRecursivePointCount();
   }
   return pc;
}

////////////////////////////////////////////////////////////////////////////////
// Add contained object

bool RenderableObject::addChild (RenderableObject* obj)
{
   if (obj == NULL) {
      //cout << "ComplexObject::addObject: Trying to add NULL object" << endl;
      return false;
   }

   if (obj == this) {
      //BGString str;
      //str.assignInt ((int) obj, 16);
      //cout << "ComplexObject::addObject: This and obj at same address "
      //     << str.getChars() << endl;
      return false;
   }

   childs.push_back (obj);
   obj->addParent (this);
   return true;
}

////////////////////////////////////////////////////////////////////////////////
// Remove contained object

bool RenderableObject::removeObject (RenderableObject* obj, bool DeleteObject)
{
   bool found = false;
   obj->removeParent (this);

/*   vector <RenderableObject*>::iterator i = ContainedObjects.begin();
   while (i != ContainedObjects.end())
   {
      if ((*i) == obj) {
         i = ContainedObjects.erase (i);
         //(*i) = (*ContainedObjects.rend());
         //ContainedObjects.pop_back();
         found = true;
      } else
         i++;
   }*/

   unsigned int i = 0;
   while (i < childs.size()) {
      if (childs[i] == obj) {
         //if (i < ContainedObjects.size()-1)
         //   ContainedObjects[i] = ContainedObjects[ContainedObjects.size()-1];
         //ContainedObjects.pop_back();
         childs[i] = NULL;
      } else
         i++;
   }

   if (DeleteObject)
      delete obj;
   return found;
}

////////////////////////////////////////////////////////////////////////////////
// Add parent objects

bool RenderableObject::addParent (RenderableObject* obj)
{ 
   if (obj == NULL) {
      // Tried to add NULL object
      return false;
   }

   if (obj == this) {
      // Child equals parent
      return false;
   }

   parents.push_back (obj); 
   return true;
}

////////////////////////////////////////////////////////////////////////////////
// Detach an object from its parents

void RenderableObject::detachObject()
{
   while (parents.begin() != parents.end()) {
      RenderableObject* ro = *parents.rbegin();
      ro->removeObject (this, false);
      parents.pop_back();
   }
}

////////////////////////////////////////////////////////////////////////////////
// Remove parent objects

void RenderableObject::removeParent (RenderableObject* obj)
{
   vector <RenderableObject*>::iterator i = parents.begin();
   int hits = 0;
   while (i != parents.end()) 
   {
      RenderableObject* other = (*i);
      if (obj == other) {
         (*i) = NULL;
         hits++;
         return;
      } else
         i++;
   }
}

////////////////////////////////////////////////////////////////////////////////
// Get object's name

const char* RenderableObject::getName() const 
{
   if (name != NULL)
      return name;
   else
      return "";
}

void RenderableObject::setName(const char* newName)
{
   if (name != NULL)
      delete name;

   size_t len = strlen (newName);
   name = new char[len+1];
   memcpy (name, newName, len+1);
}

////////////////////////////////////////////////////////////////////////////////
// Print parents (used for debugging)

void RenderableObject::printParents()
{
#ifndef WIN32
   for (unsigned int i = 0; i < parents.size(); i++) {
      RenderableObject* obj = parents[i];
      if (obj != NULL) {
         log (obj->getName());
         log (", ");
      } else
         log ("NULL OBJECT!!!!\n");
   }
   log ("\n");
#endif
}

////////////////////////////////////////////////////////////////////////////////
// Subclasses should call this function before rendering

void RenderableObject::activateParameters ()
{
   for (unsigned int i = 0; i < parameters.size (); i++)
      parameters[i]->activate();
}

////////////////////////////////////////////////////////////////////////////////
// Subclasses should call this function after rendering

void RenderableObject::deactivateParameters ()
{
   for (unsigned int i = 0; i < parameters.size (); i++)
      parameters[i]->deactivate();
}

////////////////////////////////////////////////////////////////////////////////
// Subclasses should call this function to refresh render parameters

void RenderableObject::refreshParameters ()
{
   for (unsigned int i = 0; i < parameters.size (); i++)
      parameters[i]->refresh();
}

////////////////////////////////////////////////////////////////////////////////
// Mesh size calculation

void RenderableObject::getSize(Point3D& minCoords, 
   Point3D& maxCoords, size_t& polygons)
{
   try {
      MatrixStack m;
      getSizeRecursive (minCoords, maxCoords, polygons, m);
   } catch (...) {
      printf ("Cannot determine size of illegal object\n");
   }
}

////////////////////////////////////////////////////////////////////////////////
// Recursive Part of mesh size calculation

void RenderableObject::getSizeRecursive(Point3D& minCoords, 
   Point3D& maxCoords, size_t& polygons, MatrixStack& m)
{
   m.multiplyMatrix (getMatrix());
   size_t count = getObjectCount();
   for (size_t i = 0; i < count; i++) {
      RenderableObject* obj = getObject(i);
      if (obj != NULL)
         obj->getSizeRecursive (minCoords, maxCoords, polygons, m);
   }
   m.pop();
}

////////////////////////////////////////////////////////////////////////////////
// Render

// Use hardware matrix stack
/*void RenderableObject::render ()
{
   glPushMatrix();
   M.glMultMatrix();

   renderSelf();
   renderChilds();

   glPopMatrix();
}*/

// use only software matrix stack
/*void RenderableObject::render ()
{
   MatrixStack* renderMatrixStack = MatrixStack::getRenderMatrixStack();
   renderMatrixStack->multiplyMatrix (M);
   renderMatrixStack->top().glLoadMatrix();

   renderSelf();
   renderChilds();

   renderMatrixStack->pop();
   renderMatrixStack->top().glLoadMatrix();
}*/

// Mix software and hardware matrix stack
// This method can coexist with code that is unaware of software matrix stacks
void RenderableObject::render ()
{
   renderSelf();
   renderChilds();
}

////////////////////////////////////////////////////////////////////////////////
// Render childs

/*void RenderableObject::renderChilds()
{
   for (unsigned int i = 0; i < childs.size (); i++) {
      refreshParameters();
      RenderableObject* o = childs[i];
      if (o != NULL)
         if (o->visible)
            o->render ();
   }
}*/

void RenderableObject::renderChilds ()
{
   for (size_t i = 0; i < childs.size(); ++i) {
      RenderableObject* ro = childs[i];
      if (ro != NULL) {
         glPushMatrix();
         ro->getMatrix().glMultMatrix();
         ro->renderSelf();
         ro->renderChilds();
         glPopMatrix();
      }
   }
}

////////////////////////////////////////////////////////////////////////////////
// Mark front-facing polygons

/*void RenderableObject::markFrontFacingPolygons (const Point3D& ls)
{
   for (unsigned int i = 0; i < childs.size (); i++) {
      childs[i]->markFrontFacingPolygons (ls);
   }
}*/

////////////////////////////////////////////////////////////////////////////////
// Render shadow volumes

/*void RenderableObject::renderShadowVolumes (const Point3D& ls) const
{
   for (unsigned int i = 0; i < childs.size (); i++) {
      childs[i]->renderShadowVolumes (ls);
   }
}*/

////////////////////////////////////////////////////////////////////////////////
// Return number of contained polygons

/*size_t RenderableObject::getRecursivePolygonCount () const
{
   size_t sum = 0;
   for (unsigned int i = 0; i < childs.size (); i++)
      sum += childs[i]->getRecursivePolygonCount ();
   return sum;
}*/

////////////////////////////////////////////////////////////////////////////////
// Return number of contained points

/*size_t RenderableObject::getRecursivePointCount () const
{
   size_t sum = 0;
   for (unsigned int i = 0; i < childs.size (); i++)
      sum += childs[i]->getRecursivePolygonCount ();
   return sum;
}*/

