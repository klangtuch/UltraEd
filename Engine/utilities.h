#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <nusys.h>
#include "actor.h"
#include "n64sdk\ultra\GCC\MIPSE\INCLUDE\MATH.H"

#define RGB5551(R, G, B) (((R >> 3) << 11) | ((G >> 3) << 6) | ((B >> 3) << 1) | 1)

void rom_2_ram(void *from_addr, void *to_addr, s32 seq_size);

unsigned short *image_24_to_16(const unsigned char *data, int size_x, int size_y);

float vec3_dot(vector3 a, vector3 b);

float vec3_len(vector3 a, vector3 b);

vector3 vec3_norm(vector3 vector);

vector3 vec3_add(vector3 a, vector3 b);

vector3 vec3_sub(vector3 a, vector3 b);

vector3 vec3_mul(vector3 vector, float scalar);

vector3 vec3_mul_mat3x3(vector3 vector, Mtx mat);

vector3 vec3_mul_mat4x4(vector3 vector, Mtx mat);

#endif