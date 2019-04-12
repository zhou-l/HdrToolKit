//
// pch.h
// Header für Standardsystem-Includedateien.
//

#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

#include <collection.h>
#include <ppltasks.h>
#include <ppl.h>

#include <wrl/client.h>
#include <d3d11_2.h>
#include <d2d1_2.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <DirectXMath.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include "App.xaml.h"

#include "../../DirectXTex/DirectXTex/DirectXTex.h"
#define DEBUG_STR std::cout