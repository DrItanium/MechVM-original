////////////////////////////////////////////////////////////////////////////////
// Lempel-Ziv decoder for MW2
// Copyright Bjoern Ganster 2008
////////////////////////////////////////////////////////////////////////////////

#ifndef LZdecode__H
#define LZdecode__H

#include "FileCache.h"
#include "Texture.h"
#include "MWBase.h"

// Exported function: decode Lempel-Ziv compression used in MW2
MemoryBlock* LZdecode(MemoryBlock* in);

#endif