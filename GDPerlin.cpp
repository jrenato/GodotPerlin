/**
 * Perlin.cpp
 *
 * Copyright Chris Little 2012
 * Author: Chris Little
 */

#include "GDPerlin.h"

#include <cstdlib>
#include <ctime>
#include <cmath>

GDPerlin::GDPerlin() {
	srand(time(NULL));

	p = new int[256];
	Gx = new float[256];
	Gy = new float[256];
	Gz = new float[256];

	for (int i=0; i<256; ++i) {
		p[i] = i;

		Gx[i] = (float(rand()) / (RAND_MAX/2)) - 1.0f;
		Gy[i] = (float(rand()) / (RAND_MAX/2)) - 1.0f;
		Gz[i] = (float(rand()) / (RAND_MAX/2)) - 1.0f;
	}

	int j=0;
	int swp=0;
	for (int i=0; i<256; i++) {
		j = rand() & 255;

		swp = p[i];
		p[i] = p[j];
		p[j] = swp;
	}
}

GDPerlin::~GDPerlin()
{
	delete p;
	delete Gx;
	delete Gy;
	delete Gz;
}


float GDPerlin::noise(float sample_x, float sample_y, float sample_z)
{
	// Unit cube vertex coordinates surrounding the sample point
	int x0 = int(floorf(sample_x));
	int x1 = x0 + 1;
	int y0 = int(floorf(sample_y));
	int y1 = y0 + 1;
	int z0 = int(floorf(sample_z));
	int z1 = z0 + 1;

	// Determine sample point position within unit cube
	float px0 = sample_x - float(x0);
	float px1 = px0 - 1.0f;
	float py0 = sample_y - float(y0);
	float py1 = py0 - 1.0f;
	float pz0 = sample_z - float(z0);
	float pz1 = pz0 - 1.0f;

	// Compute dot product between gradient and sample position vector
	int gIndex = p[(x0 + p[(y0 + p[z0 & 255]) & 255]) & 255];
	float d000 = Gx[gIndex]*px0 + Gy[gIndex]*py0 + Gz[gIndex]*pz0;
	gIndex = p[(x1 + p[(y0 + p[z0 & 255]) & 255]) & 255];
	float d001 = Gx[gIndex]*px1 + Gy[gIndex]*py0 + Gz[gIndex]*pz0;

	gIndex = p[(x0 + p[(y1 + p[z0 & 255]) & 255]) & 255];
	float d010 = Gx[gIndex]*px0 + Gy[gIndex]*py1 + Gz[gIndex]*pz0;
	gIndex = p[(x1 + p[(y1 + p[z0 & 255]) & 255]) & 255];
	float d011 = Gx[gIndex]*px1 + Gy[gIndex]*py1 + Gz[gIndex]*pz0;

	gIndex = p[(x0 + p[(y0 + p[z1 & 255]) & 255]) & 255];
	float d100 = Gx[gIndex]*px0 + Gy[gIndex]*py0 + Gz[gIndex]*pz1;
	gIndex = p[(x1 + p[(y0 + p[z1 & 255]) & 255]) & 255];
	float d101 = Gx[gIndex]*px1 + Gy[gIndex]*py0 + Gz[gIndex]*pz1;

	gIndex = p[(x0 + p[(y1 + p[z1 & 255]) & 255]) & 255];
	float d110 = Gx[gIndex]*px0 + Gy[gIndex]*py1 + Gz[gIndex]*pz1;
	gIndex = p[(x1 + p[(y1 + p[z1 & 255]) & 255]) & 255];
	float d111 = Gx[gIndex]*px1 + Gy[gIndex]*py1 + Gz[gIndex]*pz1;

	// Interpolate dot product values at sample point using polynomial interpolation 6x^5 - 15x^4 + 10x^3
	float wx = ((6*px0 - 15)*px0 + 10)*px0*px0*px0;
	float wy = ((6*py0 - 15)*py0 + 10)*py0*py0*py0;
	float wz = ((6*pz0 - 15)*pz0 + 10)*pz0*pz0*pz0;

	float xa = d000 + wx*(d001 - d000);
	float xb = d010 + wx*(d011 - d010);
	float xc = d100 + wx*(d101 - d100);
	float xd = d110 + wx*(d111 - d110);
	float ya = xa + wy*(xb - xa);
	float yb = xc + wy*(xd - xc);
	float value = ya + wz*(yb - ya);

	return value;
}

Dictionary GDPerlin::getSimpleNoiseMap(int width, int height, float scale, int seed) {
    Dictionary noiseMap = memnew(Dictionary);

    if (scale <= 0.0f) {
        scale = 0.0001f;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float sampleX = x / scale;
            float sampleY = y / scale;

            float perlinValue = noise(sampleX, sampleY, seed);
			noiseMap[Vector2(x, y)] = perlinValue;
        }
    }

    return noiseMap;
}

Image GDPerlin::getNoiseImage(int width, int height, Dictionary noiseMap) {
    Image noiseImage = Image(width, height, false, Image::FORMAT_RGB);

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < width; j++) {
            Color color = Color(0, 0, 128, 1).linear_interpolate(Color(1, 1, 1, 1), noiseMap[Vector2(i, j)]);
            noiseImage.put_pixel(i, j, color);
        }
    }

    return noiseImage;
}

Variant GDPerlin::getTextureFromImage(Image img)
{
    //ImageTexture imgTexture = memnew(ImageTexture);
    Variant imgTexture = memnew(Variant("ImageTexture"));
    imgTexture.call("create_from_image", img);

    return imgTexture;
}

Variant GDPerlin::getMaterialFromTexture(Variant texture) {

    Variant material = memnew(Variant("FixedMaterial"));
    material.call("set_texture", FixedMaterial::PARAM_DIFFUSE, texture);

    return material;
}

    /*
    func materialFromImage(image):
    	var texture = ImageTexture.new()
    	texture.create_from_image(image)
    	#texture.set_flags(0)

    	var mat = FixedMaterial.new()
    	mat.set_texture(FixedMaterial.PARAM_DIFFUSE, texture)
    	#mat.set_light_shader(FixedMaterial.LIGHT_SHADER_TOON)

    	return mat
    */

void GDPerlin::_bind_methods() {
    ObjectTypeDB::bind_method("noise",&GDPerlin::noise);
    ObjectTypeDB::bind_method("getSimpleNoiseMap",&GDPerlin::getSimpleNoiseMap);
    ObjectTypeDB::bind_method("getNoiseImage",&GDPerlin::getNoiseImage);
    ObjectTypeDB::bind_method("getTextureFromImage",&GDPerlin::getTextureFromImage);
    ObjectTypeDB::bind_method("getMaterialFromTexture",&GDPerlin::getMaterialFromTexture);
}
