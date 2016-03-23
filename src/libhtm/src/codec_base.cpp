#include "codec_base.h"
#include "htmregion.h"
#include "sensoryregion.h"
#include "sensoryinput.h"

Codec* Codec::Instance = 0;

Codec::Codec()
{
    pidx = 0;
}

Codec::~Codec()
{
}

char* Codec::GetCodecName()
{
    return codecName;
}

