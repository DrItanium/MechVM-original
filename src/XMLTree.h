////////////////////////////////////////////////////////////////////////////////
// XMLTree.h
// Copyright Björn Ganster 2005-2010
////////////////////////////////////////////////////////////////////////////////

#ifndef XMLTree__H
#define XMLTree__H

////////////////////////////////////////////////////////////////////////////////

//#define SELF_TEST

////////////////////////////////////////////////////////////////////////////////

//#include "FileCache.h"
#include "BGString.h"

//#include <string>
#include <vector>
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Records used by XMLTree

struct XMLAttribute: VirtualClass
{
   BGString m_name, m_value;
   XMLAttribute (const char* _name, const char* _value);
};

class XMLTreeNode: public VirtualClass
{
private:
   BGString m_name, m_text;
   XMLTreeNode* m_parent;
   vector <XMLTreeNode*> m_childs;
   vector <XMLAttribute*> m_attributes;

public:
   XMLTreeNode (XMLTreeNode* parent, const char* name, const char* text = NULL);
   virtual ~XMLTreeNode ();

   // Get parent
   inline XMLTreeNode* getParent() 
   { return m_parent; }
   inline const XMLTreeNode* getParent() const
   { return m_parent; }

   XMLAttribute* addAttribute (const char* attr, const char* value)
   { 
      XMLAttribute* newAttr = new XMLAttribute(attr, value);
      m_attributes.push_back(newAttr); 
      return newAttr;
   }

   inline size_t getAttributeCount() const
   { return m_attributes.size(); }

   inline const char* getAttributeName (size_t i) const
   { 
      if (i < m_attributes.size())
         return m_attributes[i]->m_name.getChars();
      else
         return NULL;
   }

   inline const char* getAttributeValueByIndex (size_t i) const
   { 
      if (i < m_attributes.size())
         return m_attributes[i]->m_value.getChars();
      else
         return NULL;
   }

   inline bool isRoot () const
   { return (m_parent == NULL); }

   inline void setName (const char* newName) 
   { m_name = newName; }
   inline const char* getName () const
   { return m_name.getChars(); }

   // Access childs
   inline size_t getChildCount() const
   { return m_childs.size(); }
   inline XMLTreeNode* getChild (int i)
   { return m_childs[i]; }
   inline const XMLTreeNode* getChild (int i) const
   { return m_childs[i]; }
   inline void addChild (XMLTreeNode* newChild)
   { m_childs.push_back(newChild); }

   inline const char* getText() const
   { return m_text.getChars(); }
};

////////////////////////////////////////////////////////////////////////////////
// Class XMLTree

class XMLTreeConstIterator: public VirtualClass
{
public:
   // Constructors
   XMLTreeConstIterator (const XMLTreeNode* node = NULL);
   XMLTreeConstIterator (const XMLTreeConstIterator& iter);

   // XML tree navigation
   bool goToParent();
   bool goToChild (size_t child);
   bool goToChildByName (const char* childName);
   void goToRoot ()
   {
      while (!isRoot())
         goToParent();
   }

   // Getters
   inline const char* getNodeName() const 
   { return currNode->getName(); }
   const char* getAttributeName (unsigned int i) const;
   const char* getAttributeValueByIndex (unsigned int i) const;
   const char* getAttributeValueByName (const char* AttrName) const;
   int getIntAttributeValueByName (const char* AttrName) const;
   bool getDoubleAttributeByName (const char* attrName, double& val) const;
   const char* getText() const;
   inline size_t getChildCount() const
   { return currNode->getChildCount(); }
   size_t getAttributeCount() const;

   inline bool isRoot() const 
   { return currNode->isRoot(); }

   inline const XMLTreeNode* getTreeNode() const
   { return currNode; }

protected:
   const XMLTreeNode* currNode;
};

class XMLTree;

class XMLTreeIterator: public VirtualClass
{
public:
   // Constructors
   XMLTreeIterator (XMLTree* tree, XMLTreeNode* node = NULL);
   XMLTreeIterator (const XMLTreeIterator& iter);

   // XML tree navigation
   bool goToParent()
   { 
      if (m_currNode != NULL) {
         m_currNode = m_currNode->getParent();
         return true;
      } else
         return false;
   }
   bool goToChild (size_t child)
   { 
      bool result = false;
      if (m_currNode != NULL) {
         if (child < m_currNode->getChildCount()) {
            m_currNode = m_currNode->getChild(child);
            result = true;
         }
      }
      return result;
   }
   bool goToChildByName (const char* childName);
   void goToRoot ()
   {
      while (!isRoot())
         goToParent();
   }

   // Getters
   inline const char* getNodeName() const 
   { return m_currNode->getName(); }
   const char* getAttributeName (unsigned int i) const;
   const char* getAttributeValueByIndex (unsigned int i) const;
   const char* getAttributeValueByName (const char* AttrName) const;
   int getIntAttributeValueByName (const char* AttrName) const;
   bool getDoubleAttributeByName (const char* attrName, double& val) const;
   inline const char* getText() const
   { return m_currNode->getText(); }
   inline size_t getChildCount() const
   { return m_currNode->getChildCount(); }
   size_t getAttributeCount() const
   { return m_currNode->getAttributeCount(); }

   inline bool isRoot() const 
   { return m_currNode->isRoot(); }

   inline const XMLTreeNode* getTreeNode() const
   { return m_currNode; }

   // Add a child to a node, returning its number
   size_t addChild (const char* name);
   
   // Add an attribute 
   void addAttribute (const char* name, const char* value);
   void addAttribute (const char* name, int value);
   
   // Add text to a node
   size_t addText (const char* text);

   // Create a copy of a part of another tree
   void copyFromChilds (XMLTreeConstIterator iter);

protected:
   XMLTreeNode* m_currNode;
   XMLTree* m_tree;
};

class XMLTree: public VirtualClass
{
public:
   BGString indentStr;

   // Constructur, Destructor
   XMLTree(const char* rootName = "root");
   virtual ~XMLTree();

   // Persistence
   bool loadFromFile (const char* path);
   bool loadFromFile (const char* data, size_t dataSize);
   bool saveToFile (BGString& out);
   bool saveToFile (const char* fn);

   // Iterators
   inline XMLTreeIterator* getIterator() 
   { return new XMLTreeIterator (this, root); }
   inline XMLTreeConstIterator* getConstIterator () 
   { return new XMLTreeConstIterator (root); }

   inline void setRoot(XMLTreeNode* newRoot)
   {
      if (root != NULL)
         delete root;
      root = newRoot;
   }
   
private:
   XMLTreeNode* root;
   bool recSave (BGString& out, unsigned int indents, XMLTreeNode* node);
};

#endif
