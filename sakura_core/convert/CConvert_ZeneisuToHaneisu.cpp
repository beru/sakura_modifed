#include "StdAfx.h"
#include "CConvert_ZeneisuToHaneisu.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ƒCƒ“ƒ^[ƒtƒF[ƒX                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ‘SŠp‰p”¨”¼Šp‰p”
bool Converter_ZeneisuToHaneisu::DoConvert(NativeW* pcData)
{
	// ‘SŠp‰p”¨”¼Šp‰p”
	Convert_ZeneisuToHaneisu(pcData->GetStringPtr(), pcData->GetStringLength());

	return true;
}

