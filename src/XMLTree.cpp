////////////////////////////////////////////////////////////////////////////////
// XMLTree.cpp
// Copyright Bjoern Ganster 2005-2011
////////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <assert.h>
#include <math.h>

#include "XMLTree.h"
#include "FileCache.h"

////////////////////////////////////////////////////////////////////////////////

XMLAttribute::XMLAttribute (const char* _name, const char* _value)
: VirtualClass (XMLAttributeID),
  m_name (_name),
  m_value (_value)
{
   m_name.removeTrailingWhitespace();
   m_value.removeTrailingWhitespace();
}

////////////////////////////////////////////////////////////////////////////////

XMLTreeNode::XMLTreeNode (XMLTreeNode* parent, const char* name, const char* text)
: VirtualClass (XMLTreeNodeID),
  m_parent (parent),
  m_name (name),
  m_text (text)
{
   m_name.removeTrailingWhitespace();
   m_text.removeTrailingWhitespace();
}

XMLTreeNode::~XMLTreeNode ()
{
   unsigned int i;
   
   for (i = 0; i < m_childs.size(); i++) {
      delete m_childs[i];
   }

   for (i = 0; i < m_attributes.size(); i++) {
      delete m_attributes[i];
   }
}

////////////////////////////////////////////////////////////////////////////////
// XMLTreeConstIterator constructors

XMLTreeConstIterator::XMLTreeConstIterator (const XMLTreeNode* node)
: VirtualClass (XMLTreeConstIteratorID),
  currNode (node)
{
}

XMLTreeConstIterator::XMLTreeConstIterator (const XMLTreeConstIterator& iter)
: VirtualClass (XMLTreeIteratorID)
{
   currNode = iter.currNode;
}

////////////////////////////////////////////////////////////////////////////////
// Getters

const char* XMLTreeConstIterator::getAttributeName (unsigned int i) const
{
   if (currNode != NULL)
      return currNode->getAttributeName(i);
   else
      return NULL;
}

const char* XMLTreeConstIterator::getAttributeValueByIndex (unsigned int i) const
{ 
   const char* result = NULL;
   if (currNode != NULL)
      if (i < currNode->getAttributeCount()) {
         result = currNode->getAttributeValueByIndex(i);
      }
   return result;
}

const char* XMLTreeConstIterator::getAttributeValueByName (const char* AttrName) const
{
   for (unsigned i = 0; i < getAttributeCount(); i++) {
      const char* attrName = getAttributeName(i);
      if (attrName != NULL)
         if (strcmp (attrName, AttrName) == 0) {
            return getAttributeValueByIndex(i);
         }
   }
   return NULL;
}

int XMLTreeConstIterator::getIntAttributeValueByName (const char* AttrName) const
{
   const char* str = getAttributeValueByName (AttrName);
   if (str != NULL)
      return atoi (str);
   else 
      return 0;
}

bool XMLTreeConstIterator::getDoubleAttributeByName (const char* attrName, 
                                                     double& val) const
{
   BGString value = getAttributeValueByName (attrName);
   if (value.getLength() != 0) {
      return value.toDouble(val);
   } else
      return false;
}

const char* XMLTreeConstIterator::getText() const
{
   if (currNode != NULL)
      return currNode->getText();
   else
      return NULL;
}

size_t XMLTreeConstIterator::getAttributeCount() const
{
   if (currNode != NULL)
      return currNode->getAttributeCount();
   else
      return 0;   
}

////////////////////////////////////////////////////////////////////////////////
// XML tree navigation

bool XMLTreeConstIterator::goToParent()
{
   if (currNode != NULL) {
      currNode = currNode->getParent();
      return true;
   } else 
      return false;
}

bool XMLTreeConstIterator::goToChild (size_t child)
{
   if (currNode != NULL) {
      if (child < currNode->getChildCount()) {
         currNode = currNode->getChild(child);
         return true;
      } else {
         return false;
      }
   } else {
      return false;
   }
}

bool XMLTreeConstIterator::goToChildByName (const char* childName)
{
   size_t i = 0;
   bool keepScanning = true;
   while (keepScanning) {
      keepScanning = goToChild(i);
      if (keepScanning) {
         if (strcmp(getNodeName(), childName) == 0) {
            return true;
         } else {
            i++;
            goToParent();
         }
      }
   }

   return false;
}

////////////////////////////////////////////////////////////////////////////////

const char* XMLTreeIterator::getAttributeValueByName (const char* AttrName) const
{
   for (unsigned i = 0; i < getAttributeCount(); i++) {
      const char* attrName = getAttributeName(i);
      if (attrName != NULL)
         if (strcmp (attrName, AttrName) == 0) {
            return getAttributeValueByIndex(i);
         }
   }
   return NULL;
}

int XMLTreeIterator::getIntAttributeValueByName (const char* AttrName) const
{
   const char* str = getAttributeValueByName (AttrName);
   if (str != NULL)
      return atoi (str);
   else 
      return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Constructors

XMLTreeIterator::XMLTreeIterator (XMLTree* tree, XMLTreeNode* node)
: VirtualClass (XMLTreeIteratorID),
  m_tree (tree),
  m_currNode (node)
{
}

XMLTreeIterator::XMLTreeIterator (const XMLTreeIterator& iter)
: VirtualClass (XMLTreeIteratorID),
  m_currNode (iter.m_currNode),
  m_tree (iter.m_tree)
{
}

////////////////////////////////////////////////////////////////////////////////
// Operations (strings are copied)

size_t XMLTreeIterator::addChild (const char* name)
{ 
   assert (name != NULL);
   assert (name[0] != 0);
   XMLTreeNode* newnode = new XMLTreeNode(m_currNode, name);
   if (m_currNode != NULL) {
      m_currNode->addChild(newnode);
   } else {
      m_currNode = newnode;
      m_tree->setRoot(newnode);
   }
   return m_currNode->getChildCount()-1;
}

void XMLTreeIterator::addAttribute (const char* name, const char* value)
{
   if (m_currNode != NULL) {
      m_currNode->addAttribute(name, value);
   }
}

void XMLTreeIterator::addAttribute (const char* name, int value)
{
   BGString valueStr;
   valueStr.assignInt (value);
/*   char valueStr[10];
   _itoa (value, valueStr);*/
   addAttribute (name, valueStr.getChars());
}

size_t XMLTreeIterator::addText (const char* text)
{
   int result = -1;
   if (text != NULL && text[0] != 0) {
      if (m_currNode != NULL) {
         XMLTreeNode* textNode = new XMLTreeNode (m_currNode, "", text);
         m_currNode->addChild(textNode);
         return m_currNode->getChildCount()-1;
      } 
   }
   return result;
}

const char* XMLTreeIterator::getAttributeName (unsigned int i) const
{
   if (m_currNode != NULL)
      return m_currNode->getAttributeName(i);
   else
      return NULL;
}

const char* XMLTreeIterator::getAttributeValueByIndex (unsigned int i) const
{
   if (m_currNode != NULL) {
      if (i < m_currNode->getAttributeCount())
         return m_currNode->getAttributeValueByIndex(i);
   }
   return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Constructur, Destructor

XMLTree::XMLTree(const char* rootName)
: VirtualClass (XMLTreeID),
  indentStr ("   "),
  root (NULL)
  //iter (),
  //constIter ()
{}

XMLTree::~XMLTree()
{
   delete root;
}

////////////////////////////////////////////////////////////////////////////////
// Load from file

bool XMLTree::loadFromFile (const char* path)
{
   FileCache fc;
   if (!fc.openReadOnly(path))
      return false;

   size_t fileSize = fc.getFileSize();
   char* data = new char[fileSize];
   bool result = fc.read(0, data, fileSize);
   fc.close();

   if (result) 
      result = loadFromFile(data, fileSize);
   delete data;
   return result;
}

bool XMLTree::loadFromFile (const char* data, size_t dataSize)
{
   if (root != NULL)
      delete root;
   
   size_t offset = 0;
   const int ReadText = 0;
   const int ReadTag = 1;
   const int ReadAttr = 2;
   const int ReadAttrVal = 3;
   const int ReadEscapeSequence = 4;
   bool quotedValue = false;
   int mode = 0;
   char c;
   BGString attr = "";
   BGString value = "";
   BGString tag = "";
   BGString text = "";
   BGString esc = ""; // for reading escape sequences
   XMLTreeNode* currNode = NULL;

   while (offset < dataSize) 
   {
      c = data[offset];
      offset++;
      // Read text?
      if (mode == ReadText) {
        if (c == '<') {
            // Create text node, then stop reading text 
            text.removeTrailingWhitespace();
            if (text.getLength() > 0 && root != NULL) {
               XMLTreeNode* newNode = new XMLTreeNode (currNode, "", text.getChars());
               if (currNode != NULL)
                  currNode->addChild(newNode);
            }
            if (data[offset] != '/') {
               XMLTreeNode* newNode = new XMLTreeNode (currNode, "", "");
               if (currNode != NULL)
                  currNode->addChild(newNode);
               if (root == NULL)
                  root = currNode;
               currNode = newNode;
            }
            mode = ReadTag;
            tag = "";
            text ="";
         } else {
            if (c == '&') {
               mode = ReadEscapeSequence;
               esc = "";
            } else 
            if ((   c != ' ' 
                 && c != 9
                 && c != 0x0d 
                 && c != 0x0a) 
            || text.getLength() != 0)
            {
               text += c;
            }
         } 

      // Read tag?
      } else if (mode == ReadTag) {
         if (c == '/') {
            currNode = currNode->getParent();
            // Skip to >
            do {
               c = data[offset];
               offset++;
            } while (c != '>');
            // Store non-empty tag ending immediately?
            if (tag.getLength() != 0) {
               XMLTreeNode* newNode = new XMLTreeNode (currNode, tag.getChars());
               currNode->addChild (newNode);
               currNode = newNode;
            }
            tag = "";
            mode = ReadText;
         } else if (c == '>') {
            currNode->setName (tag.getChars());
            mode = ReadText;
         } else if (c!= ' ' && c != 0x0d && c != 0x0a && c != 9) {
            tag += c;
         } else {
            if (tag.getLength() != 0) 
            {
               mode = ReadAttr;
               attr = "";
               value = "";
            }
         }

      // Read tag attribute
      } else if (mode == ReadAttr) {
        if (c == '>') {
            mode = ReadText;
            currNode->setName (tag.getChars());
         } else if (c == '=') {
            mode = ReadAttrVal;
         } else if (c!= ' ' && c != 0x0d && c != 0x0a && c != 9) {
            attr += c;
         } 

      // Read attribute value
      } else if (mode == ReadAttrVal) {
         if (c == '>'&& !quotedValue) {
            mode = ReadText;
            currNode->setName (tag.getChars());
            currNode->addAttribute (attr.getChars(), value.getChars());
            attr = "";
            value = "";
         } else if (c == '\"') {
            quotedValue = !quotedValue;
         } else 
         if ((c == ' ' && !quotedValue) 
         ||   c == 0x0d 
         ||   c == 0x0a 
         &&   !quotedValue) {
            if (value.getLength() != 0) {
               currNode->addAttribute (attr.getChars(), value.getChars());
               attr = "";
               value = "";
               mode = ReadAttr;
            }
         } else if (c != '/' || quotedValue) {
            value += c;
         } else { // c=='/'
            currNode->addAttribute (attr.getChars(), value.getChars());
            currNode->setName (tag.getChars());
            attr = "";
            value = "";
            mode = ReadText;
            currNode = currNode->getParent();
            // Skip to >
            do {
               c = data[offset];
               offset++;
            } while (c != '>');
            mode = ReadText;
         }
      } else if (mode == ReadEscapeSequence) {
         if (c ==';') {
            if (strcmp (esc.getChars(), "amp") == 0)
               text += "&";
            else if (strcmp (esc.getChars(), "lt") == 0)
               text += "<";
            else if (strcmp (esc.getChars(), "gt") == 0)
               text += ">";
            mode = ReadText;
         } else
            esc += c;
      }
   }

   return true;
}

////////////////////////////////////////////////////////////////////////////////
// Save to file

bool XMLTree::recSave (BGString& out, unsigned int indents,
                       XMLTreeNode* node)
{
   BGString line;
   bool result = true;
   unsigned int i;
   BGString LineIndent;
   if (indentStr.getLength() != 0)
      for (i = 0; i < indents; i++)
         LineIndent += indentStr.getChars();

   // Save tag node
   const char* nodeName = node->getName();
   if (nodeName != NULL)
      if (strlen(nodeName) == 0)
         nodeName = NULL;
   if (nodeName != NULL) {
      line = LineIndent;
      line += "<";
      line += nodeName;

      // Save tag node's attributes
      for (i = 0; i < node->getAttributeCount(); i++) {
         line += " ";
         line += node->getAttributeName (i);
         line += " = \"";
         line += node->getAttributeValueByIndex (i);
         line += "\"";
      }
   }

   const char* text = node->getText();
   if (text != NULL)
      if (strlen(text) == 0)
         text = NULL;
   if (node->getChildCount() == 0 
   &&  text == NULL)
   //&&  node->getAttributeCount() == 0) 
   {
      // Early exit
      line += "/>\n";
      out += line;
      return true;
   }

   if (nodeName != NULL) {
      line += ">\n";
      out += line;
      line = "";
   }

   // Save text node
   if (text != NULL) {
      size_t len = strlen (text);
      for (unsigned int i = 0; i < len; i++) {
         char c = text[i];
         if (c == '<') 
            out += "&lt;";
         else if (c == '>') 
            out += "&gt;";
         else if (c == '&') 
            out += "&amp;";
         else
            out += c;
      }
      out += "\n";
   }

   // Add child tags
   if (node->getChildCount() != 0) {
      assert (line.getChars() != NULL);
      out += line;
      for (unsigned int j = 0; j < node->getChildCount(); j++) {
         XMLTreeNode* child = node->getChild(j);
         result &= recSave (out, indents+1, child);
      }
      //line += "/>\n";
   } 

   // Add end tag
   if (nodeName != NULL) {
      line = LineIndent;
      line += "</";
      line += node->getName();
      line += ">\n";
      out += line;
   }

   return result;
}

bool XMLTree::saveToFile (BGString& out)
{
   return recSave (out, 0, root);
}

bool XMLTree::saveToFile (const char* fn)
{
   BGString data;
   if (saveToFile(data)) {
      return data.saveToFile(fn);
   } else
      return false;
}

////////////////////////////////////////////////////////////////////////////////
// Create a copy of a part of another tree

void XMLTreeIterator::copyFromChilds (XMLTreeConstIterator constIter)
{
   //const char* nodeName = getNodeName();
   //setNodeName (nodeName);

   // Copy attributes
   for (size_t i = 0; i < constIter.getAttributeCount(); i++) {
      const char* attrName = constIter.getAttributeName(i);
      const char* attrValue = constIter.getAttributeValueByIndex(i);
      addAttribute (attrName, attrValue);
   }

   // Copy child nodes
   for (size_t i = 0; i < constIter.getChildCount(); i++) {
      constIter.goToChild(i);

      size_t newChildNum;
      const char* childText = constIter.getText();
      if (childText != NULL) {
         newChildNum = addText (constIter.getText());
      } else {
         newChildNum = addChild(constIter.getNodeName()); // Proper name set during recursion
      }
      goToChild(newChildNum);
      copyFromChilds (constIter);
      goToParent();

      constIter.goToParent();
   }
}

////////////////////////////////////////////////////////////////////////////////
// Iterators

/*XMLTreeIterator XMLTree::getIterator()
{
   return XMLTreeIterator (root);
}

XMLTreeConstIterator XMLTree::getConstIterator () const
{
   return XMLTreeConstIterator (root);
}*/

////////////////////////////////////////////////////////////////////////////////
// Self tests, define SELF_TEST (above) and compile with someting like
// g++ XMLTree.cpp FileCache.cpp -o xmltest

#ifdef SELF_TEST

void test1()
{
   // Try writing empty XML tree
   cout << "Running test 1" << endl;
   XMLTree tree;
   remove ("empty.xml");
   cout << "Saving" << endl;
   tree.saveToFile ("empty.xml");
}

////////////////////////////////////////////////////////////////////////////////

void test2()
{
   // Try writing empty XML tree
   cout << "Running test 2" << endl;
   XMLTree tree;
   XMLTreeIterator iter (tree.getIterator());
   for (unsigned int i = 0; i < 5; i++) {
      int newchild = iter.addChild ("FirstLevelNode");
      assert (iter.getChildCount() > 0);
      assert (newchild == i);
      assert (iter.goToChild (newchild));
      char num[2];
      num[0] = '0' + i;
      num[1] = 0;
      iter.addAttribute ("number", num);
      if (i > 1)
         iter.addText ("This node's text");
      iter.goToParent();
   }
   remove ("first.xml");
   tree.saveToFile ("first.xml");
}

////////////////////////////////////////////////////////////////////////////////

void test3()
{
   cout << "Running test 3" << endl;
   XMLTree tree;
   cout << "Loading file Components+Operators.xml" << endl;
   tree.loadFromFile ("Components+Operators.xml");
   tree.saveToFile ("co.xml");
}

////////////////////////////////////////////////////////////////////////////////

int main()
{
   test1();
   test2();
   test3();
}

#endif

