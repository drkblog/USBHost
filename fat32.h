/* FAT32 driver - Copyright (C) 2012 Leandro Fernández 

For USB Host Library (C) Circuits At Home, LTD. All rights reserved.

This software may be distributed and modified under the terms of the GNU
General Public License version 2 (GPL2) as published by the Free Software
Foundation and appearing in the file GPL2.TXT included in the packaging of
this file. Please note that GPL2 Section 2[b] requires that all works based
on this software must also be made publicly available under the terms of
the GPL2 ("Copyleft").

Contact information
-------------------

Leandro Fernández
Web      :  http://www.drk.com.ar
e-mail   :  drkbugs@gmail.com

Circuits At Home, LTD (http://www.circuitsathome.com)
*/

#if !defined(__FAT32_H__)
#define __FAT32_H__

#include <max3421e.h>
#include <Usb.h>
#include <masstorage.h>

class FAT32
{
private:
  BulkOnly * bulk;
  
public:
  FAT32(BulkOnly  * bulk) : bulk(bulk) {};
  void dump();
};

#endif // __FAT32_H__
