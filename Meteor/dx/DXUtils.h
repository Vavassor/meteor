#ifndef DX_UTILS_H
#define DX_UTILS_H

#include <Windows.h>

#include "DXInfo.h"
#include "BString.h"

String dxerr_text(HRESULT hr);
String hresult_text(HRESULT hr);

const char* get_feature_level_name(D3D_FEATURE_LEVEL level);

int dx_get_max_tex_dim();
bool dx_non_power_of_two_cond();
int dx_get_max_anisotropy();

size_t size_of_format(DXGI_FORMAT format);
void get_tex_image(void* data, ID3D11Texture2D* texture);

#endif
