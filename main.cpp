//#include <iostream>
#include <omp.h>

#include "vec3.h"
#include "image.h"
#include "random.h"
#include "sampling.h"
//#include "hon.h"
#include "hmath.h"
#include "timer.h"
#include "ray.h"
#include "intersection.h"
#include "brdf.h"
#include "ibl.h"
#include "bmpexporter.h"

//not used
//#include "kdtree.h"

#include "objMesh.h"

unsigned char to_bmp_value(const float value, const float display_gamma) {
	const unsigned int v = (unsigned int)(pow(value, 1.0f / display_gamma) * 255.0f + 0.5f);
	if (v >= 256)
		return (unsigned char)255;
	return (unsigned char)v;
}

//tigra: time functions
float get_time_str(char * s,float endT,float startT)
{
	int hours,mins,secs;
		
		float elapsedTime = float(endT - startT) / float(CLOCKS_PER_SEC );
		
		hours=(int)floor(elapsedTime/3600);
		mins=(int)floor((elapsedTime-hours*3600)/60);
		secs=(int)floor(elapsedTime-hours*3600-mins*60);			

		sprintf(s,"%0.2fs",elapsedTime);		
		
			if(elapsedTime>=60)
				{
				 strcat(s,"(");
				 if(hours>0) sprintf(s,"%s%dh",s,hours);
				 if(mins>0) 
					{
					if(hours>0) strcat(s,":");
					sprintf(s,"%s%dmin:",s,mins);
					}
				 sprintf(s,"%s%02ds)",s,secs);
				}
				
	return elapsedTime;
}

//good optimised project in vc 2010
int main(int argc, char *argv[]) {
	using namespace hstd;

	printf("Akkari-n\n");
	
	// 時間測定用
	Timer timer;
	timer.begin();
	float accum_time = 0;
	float prev_now = timer.end();
	
	float now00 = timer.end();
	float now0iter;
	float now;
	
	int image_iteration = 0;
	
	char str[10000];

	rt::ImageBasedLight test("asset/Barce_Rooftop_C_3k.hdr");
	test.create_importance_map(60, 30);
	
	Image normal01(0, 0);
	HDROperator::load("asset/normal02.hdr", &normal01);

//	test.create_sample_from_importance_map(256, normalize(Float3(0.2, -0.5, 0.2)), Float3(0, 1, 0), rt::PhongBRDF(Color(1, 1, 1), 200));


	rt::TriangleMesh mesh;
//	rt::OBJOperator::load("I:\\3DModel\\sdragon2.obj", &mesh);
//	if (!rt::OBJOperator::load("asset/hb-scene2.obj", &mesh)) {
	if (!rt::OBJOperator::load("asset/scene2.obj", &mesh)) {
		return 0;
	}

	Image image(1920, 1080);
	
	//Image image(1024, 768);
	const int width = image.width();
	const int height = image.height();
	std::vector<unsigned char> bmp_arr(width * height * 3);

	Random random(0);

	const int SuperX = 1;
	const int SuperY = 1;
	const int kMaxDepth = 5;
	const int kMaxIteration = 10000000;

	float now0 = timer.end();
	
	//omp_set_num_threads(32);

	//tigra: вынес из цикла
						Float3 camera_position(12, 1, 6);
						Float3 camera_lookat(0, 3.7, 1);
						Float3 camera_direction = normalize(camera_lookat - camera_position);
						float distance_to_screen = 0.65f;

						Float3 camera_up = normalize(Float3(-0.2, 1.0, 0.0));
						float aspect = (float)width / height;
						Float3 screen_side = aspect * cross(camera_direction, camera_up);
						Float3 screen_up = normalize(cross(camera_direction, screen_side));
						Float3 screen_center = camera_position + distance_to_screen * camera_direction;

	
	//float SuperXY =1.0f / (SuperX * SuperY);

	//tigra: это будет spp для aaa
	for (int iteration = 0; iteration < kMaxIteration; ++iteration) {
		printf("Iteration: %d\n", iteration + 1);
				
		now0iter = timer.end();
		

		//iteration code begin		
		for (int iy = 0; iy < height; ++iy) {
			// std::cout << iy << std::endl;

			#pragma omp parallel for schedule(dynamic, 1)
			for (int ix = 0; ix < width; ++ix) {
			
				//for (int suby = 0; suby < SuperY; ++suby) 
				{
					//for (int subx = 0; subx < SuperX; ++subx) 
					{
						//not used
						/*
						float nowtime = 0;
						float R = 1;
						*/

						const float u = float(ix + random.next01()) / width;
						const float v = float(iy + random.next01()) / height;

						const float sx = (u * 2.0f) - 1.0f;
						const float sy = (v * 2.0f) - 1.0f;
					

						const Float3 pos_on_screen = screen_center + sx * screen_side + sy * screen_up;

					
						rt::Ray now_ray(camera_position, normalize(pos_on_screen - camera_position));

						// レンズ
						float lensU, lensV;
						const float r = random.next01();
						const float lens_radius = 0.015f;
						const float focal_distance = 5.0f;

						Sampling::uniformCircle(random, &lensU, &lensV);
						lensU *= lens_radius;
						lensV *= lens_radius;
						const float ft = fabs(focal_distance / dot(now_ray.dir, camera_direction));

						const Float3 Pfocus = now_ray.org + now_ray.dir * ft;
						now_ray.org = now_ray.org + lensU * screen_side +  lensV * screen_up;
						now_ray.dir = normalize(Pfocus - now_ray.org);
					
						//tigra: eye tracing path
						{
							rt::Hitpoint now_hp;
							Color througput(1, 1, 1);
							for (int i = 0; i < kMaxDepth; ++i) {
								//bool result1 = false;
								bool result1 = mesh.intersect(now_ray, &now_hp);

								if (!result1) {
									//ibl sample
									const Color L = test.sample_from_direction(now_ray.dir);
							
									//image.at(ix, iy) += times(througput, L) / (SuperX * SuperY);
									//image.at(ix, iy) += times(througput, L) * SuperXY;
									image.at(ix, iy) += times(througput, L);

									break;
								}
						
								const rt::TriangleElement t = mesh.getTriangle(now_hp.triangle_index);

								// 法線
								//tigra: calc triangle normal here 
								const Float3 e1 = 
								//normalize
								(*t.v[1] - *t.v[0]);
								const Float3 e2 = 
								//normalize
								(*t.v[2] - *t.v[0]);
								
								Float3 normal = normalize(cross(e1, e2));
								
								const Float3 hp_position = now_ray.org + now_hp.distance * now_ray.dir;

								rt::Material* material = t.material;

								if (material == NULL)
									continue;

								rt::BRDF* now_brdf = NULL;
								rt::PhongBRDF phong(material->specular, material->specular_coefficient);
								rt::LambertianBRDF lambertian(material->diffuse);
							
								// std::cout << material->specular << " " << material->diffuse << " " << material->specular_coefficient << std::endl;

								//tigra: metalic is GLOSSY 
								if (random.next01() < material->metalic) {
									// スペキュラ
									now_brdf = &phong;
									//tigra: normal mapping

									Color col = normal01.sampleLoop(hp_position.x * 250, hp_position.z * 250);
									Float3 mnormal = times(col, Float3(2, 2, 2)) - Float3(1, 1, 1);
									normal += 0.2f * Float3(mnormal.x, 0, mnormal.y);
								} else {
									// ディフューズ
									now_brdf = &lambertian;
								}

								Color color = now_brdf->reflectance();
								Float3 dir = now_brdf->sample(random, now_ray.dir, normal, NULL);
							
								now_ray = rt::Ray(hp_position +  1e-2f * dir, dir);
								througput = times(color, througput);
							}
						} //eye tracing path
						
					}
				}
			}

			// 出力チェック
			now = timer.end();

			if (accum_time >= 60.0f * CLOCKS_PER_SEC) {
				accum_time = 0;
				/*
				++image_iteration;
		
				Image tmpimage(width, height);
				for (int iy = 0; iy < height; ++iy) {
					for (int ix = 0; ix < width; ++ix) {
						tmpimage.at(ix, iy) = 4.0f * image.at(ix, iy) / iteration;
					}
				}

				// bmpに出力
				// tigra: gamma only
				for (int iy = 0; iy < height; ++iy) {
					for (int ix = 0; ix < width; ++ix) {
						const float display_gamma = 2.2f;
						const unsigned char r = to_bmp_value(tmpimage.at(ix, iy).x, display_gamma);
						const unsigned char g = to_bmp_value(tmpimage.at(ix, iy).y, display_gamma);
						const unsigned char b = to_bmp_value(tmpimage.at(ix, iy).z, display_gamma);
						bmp_arr[(iy * width + ix) * 3 + 0] = b;
						bmp_arr[(iy * width + ix) * 3 + 1] = g;
						bmp_arr[(iy * width + ix) * 3 + 2] = r;

					}
				}
				char buf[1024];
				sprintf(buf, "image_%02d.bmp", image_iteration);
				exportToBmp(buf, &bmp_arr[0], width, height);

				/*
				char buf[1024];
				sprintf(buf, "image_%02d.hdr", image_iteration);
				HDROperator::save(buf, &tmpimage, true);
				*/
/*
				std::cout << "Save: " << buf << std::endl << std::endl;

				if (image_iteration == 30)
					return 0;
					*/
			} else {
				accum_time += now - prev_now;
			}
			prev_now = now;
		}
		//iteration code done
		
			//tigra: show iteration time
			now = timer.end();
			
			float tttime;
			
			tttime=get_time_str(str, now, now0iter);			
			printf("time %s(%.2fs)\n",str, tttime);
			
			tttime=get_time_str(str, now, now0);			
			printf("total time %s(%.2fs)\n",str, tttime);
			
			//tigra : save after iteration
			
				++image_iteration;

				float coeff= 1.0f  / image_iteration;
		
				Image tmpimage(width, height);
				for (int iy = 0; iy < height; ++iy) {
					for (int ix = 0; ix < width; ++ix) {
						//tmpimage.at(ix, iy) = 4.0f * image.at(ix, iy) / image_iteration;
						tmpimage.at(ix, iy) = coeff * image.at(ix, iy) ;
					}
				}

				// bmpに出力
				// tigra: gamma only

				int idxx;
				const float display_gamma = 2.2f;

				for (int iy = 0; iy < height; ++iy) {
					for (int ix = 0; ix < width; ++ix) {
						const unsigned char r = to_bmp_value(tmpimage.at(ix, iy).x, display_gamma);
						const unsigned char g = to_bmp_value(tmpimage.at(ix, iy).y, display_gamma);
						const unsigned char b = to_bmp_value(tmpimage.at(ix, iy).z, display_gamma);						

						idxx = (iy * width + ix) * 3;

						bmp_arr[idxx    ] = b;
						bmp_arr[idxx + 1] = g;
						bmp_arr[idxx + 2] = r;

					}
				}
				char buf[1024];
				sprintf(buf, "image_%03d.bmp", image_iteration);
				
#ifdef BMP_SAVER
				exportToBmp(buf, &bmp_arr[0], width, height);
#endif

				sprintf(buf, "image_%03d.hdr", image_iteration);
				HDROperator::save(buf, &tmpimage, true);

				printf("Save: %s\n\n", buf);
			
	}

	return 0;
}
