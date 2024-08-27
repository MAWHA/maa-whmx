/* Copyright 2024 周上行Ryer

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

#include <shlobj_core.h>

const CLSID CLSID_ImmersiveShell            = {0xC2F03A33, 0x21F5, 0x47FA, 0xB4, 0xBB, 0x15, 0x63, 0x62, 0xA2, 0xF2, 0x39};
const CLSID CLSID_VirtualDesktopAPI_Unknown = {0xC5E0CDCA, 0x7B6E, 0x41B2, 0x9F, 0xC4, 0xD9, 0x39, 0x75, 0xCC, 0x46, 0x7B};

//! ATTENTION: this is only valid for windows 10
const IID IID_IVirtualDesktopManagerInternal = {0xF31574D6, 0xB682, 0x4CDC, 0xBD, 0x56, 0x18, 0x27, 0x86, 0x0A, 0xBE, 0xC6};

struct IApplicationView : public IUnknown {};

MIDL_INTERFACE("FF72FFDD-BE7E-43FC-9C03-AD81681E88E4")
IVirtualDesktop : public IUnknown {
public:
    virtual HRESULT STDMETHODCALLTYPE IsViewVisible(IApplicationView * pView, int *pfVisible) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetID(GUID * pGuid)                                     = 0;
};

enum AdjacentDesktop {
    LeftDirection  = 3,
    RightDirection = 4
};

MIDL_INTERFACE("AF8DA486-95BB-4460-B3B7-6E7A6B2962B5")
IVirtualDesktopManagerInternal : public IUnknown {
public:
    virtual HRESULT STDMETHODCALLTYPE GetCount(UINT * pCount)                                                   = 0;
    virtual HRESULT STDMETHODCALLTYPE MoveViewToDesktop(IApplicationView * pView, IVirtualDesktop * pDesktop)   = 0;
    virtual HRESULT STDMETHODCALLTYPE CanViewMoveDesktops(IApplicationView * pView, int *pfCanViewMoveDesktops) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCurrentDesktop(IVirtualDesktop * *desktop)                             = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDesktops(IObjectArray * *ppDesktops)                                   = 0;
    virtual HRESULT STDMETHODCALLTYPE GetAdjacentDesktop(
        IVirtualDesktop * pDesktopReference, AdjacentDesktop uDirection, IVirtualDesktop * *ppAdjacentDesktop)     = 0;
    virtual HRESULT STDMETHODCALLTYPE SwitchDesktop(IVirtualDesktop * pDesktop)                                    = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateDesktopW(IVirtualDesktop * *ppNewDesktop)                              = 0;
    virtual HRESULT STDMETHODCALLTYPE RemoveDesktop(IVirtualDesktop * pRemove, IVirtualDesktop * pFallbackDesktop) = 0;
    virtual HRESULT STDMETHODCALLTYPE FindDesktop(GUID * desktopId, IVirtualDesktop * *ppDesktop)                  = 0;
};
