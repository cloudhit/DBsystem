/*
 *  Description of a simple page
 *  $Id: page.C,v 1.1 $
 */

#ifdef PAGE_DEBUG
#    include <iostream.h>
#endif

#include "page.h"

// Methods of the class Page
//***********************************************************

// Constructor
Page::Page()
{
#ifdef PAGE_DEBUG
    cout << "constructing page" << endl;
#endif
}

//*************************************************************

// Destructor
Page::~Page()
{
#ifdef PAGE_DEBUG
    cout << "Destroying page" << endl;
#endif
}

//************************************************************
