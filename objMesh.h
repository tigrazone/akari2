#ifndef _OBJ_MESH_
#define _OBJ_MESH_

#include "vec3.h"
#include "constant.h"
#include "bbox.h"
#include "intersection.h"
#include "triangleMesh.h"
#include "qbvh.h"

#include "memfile.h"
// not used
// #include "textutil.h"

/*
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
*/

//
// Simple and fast atof (ascii to float) function.
//
// - Executes about 5x faster than standard MSCRT library atof().
// - An attractive alternative if the number of calls is in the millions.
// - Assumes input is a proper integer, fraction, or scientific format.
// - Matches library atof() to 15 digits (except at extreme exponents).
// - Follows atof() precedent of essentially no error checking.
//
// 09-May-2009 Tom Van Baak (tvb) www.LeapSecond.com
//

#define white_space(c) ((c) == ' ' || (c) == '\t')
#define valid_digit(c) ((c) >= '0' && (c) <= '9')

float FASTatof (const char *p)
{
    int frac;
    float sign, value, scale;

    // Skip leading white space, if any.

    while (white_space(*p) ) {
        p += 1;
    }

    // Get sign, if any.

    sign = 1.0;
    if (*p == '-') {
        sign = -1.0;
        p += 1;

    } else if (*p == '+') {
        p += 1;
    }

    // Get digits before decimal point or exponent, if any.

    for (value = 0.0; valid_digit(*p); p += 1) {
        value = value * 10.0 + (*p - '0');
    }

    // Get digits after decimal point, if any.

    if (*p == '.') {
        double pow10 = 10.0;
        p += 1;
        while (valid_digit(*p)) {
            value += (*p - '0') / pow10;
            pow10 *= 10.0;
            p += 1;
        }
    }

    // Handle exponent, if any.

    frac = 0;
    scale = 1.0;
    if ((*p == 'e') || (*p == 'E')) {
        unsigned int expon;

        // Get sign of exponent, if any.

        p += 1;
        if (*p == '-') {
            frac = 1;
            p += 1;

        } else if (*p == '+') {
            p += 1;
        }

        // Get digits of exponent, if any.

        for (expon = 0; valid_digit(*p); p += 1) {
            expon = expon * 10 + (*p - '0');
        }
        if (expon > 308) expon = 308;

        // Calculate scaling factor.

        while (expon >= 50) { scale *= 1E50; expon -= 50; }
        while (expon >=  8) { scale *= 1E8;  expon -=  8; }
        while (expon >   0) { scale *= 10.0; expon -=  1; }
    }

    // Return signed and scaled floating point result.

    return sign * (frac ? (value / scale) : (value * scale));
}


struct FATOF
{
	float mulsum;
	float muladd;
	float add;
	FATOF * next;
};
 
FATOF beforedot_table[256];
FATOF afterdot_table[256];
 
void prepareMy()
{
	// erase tables
	for(int i=0; i<256; i++)
	{
		beforedot_table[i].mulsum = 0.0;
		beforedot_table[i].muladd = 0.0;
		beforedot_table[i].add = 0.0;
		beforedot_table[i].next = 0;
		afterdot_table[i].mulsum = 0.0;
		afterdot_table[i].muladd = 0.0;
		afterdot_table[i].add = 0.0;
		afterdot_table[i].next = 0;
	}
	// digits
	for(int i=0; i<10; i++)
	{
		beforedot_table['0' + i].mulsum = 10.0;
		beforedot_table['0' + i].muladd = 1.0;
		beforedot_table['0' + i].add = float(i);
		beforedot_table['0' + i].next = &beforedot_table[0];
		afterdot_table['0' + i].mulsum = 1.0;
		afterdot_table['0' + i].muladd = 0.1;
		afterdot_table['0' + i].add = float(i);
		afterdot_table['0' + i].next = &afterdot_table[0];
	}
	// dot
	beforedot_table['.'].mulsum = 1.0;
	beforedot_table['.'].muladd = 0.1;
	beforedot_table['.'].add = 0.0;
	beforedot_table['.'].next = &afterdot_table[0];
}
 
float myATOF(const char * str)
{
	float sum = 0.0;
	float muladd = 1.0;
	FATOF * cur = &beforedot_table[0];
	while(true)
	{
		int i = *str;
		if(cur[i].next)
		{
			sum *= cur[i].mulsum;
			sum += cur[i].add * muladd;
			muladd *= cur[i].muladd;
			str++;
			cur = cur[i].next;
		}
		else
			break;
	}
	return sum;
}


float dbefore[301];
float dafter[301];

void prepareVariant2()
{
  int i1, i2;
  
  for(i1=0;i1<10;i1++) dbefore[i1]=i1;

  for(i2=1;i2<30;i2++) 
    for(i1=0;i1<10;i1++) dbefore[i2*10+i1]=dbefore[(i2-1)*10+i1]*10;

  for(i1=0;i1<10;i1++) dafter[i1]=i1/10;

  for(i2=1;i2<30;i2++)
	for(i1=0;i1<10;i1++) dafter[i2*10+i1]=dafter[(i2-1)*10+i1]/10;
}


float variant2(const char *str)
{
	float numDouble=0;
	
	int i11,i12,i4, i21;

    i11=0;
	i12=0;
	
// Идем до точки, либо до конца числа
	while((*(str+i12)!=0) && (*(str+i12)!='.')) i12=i12+1;
	
	i4=i12;
	i12=i12-1;
// В i11 имеем начало числа, в i12 - конец целой части числа

// Берем из таблицы dbefore число по адресу i21+код числа - код цифры 0
      i21=0;
	  
      while (i12>=i11) {
		numDouble+= dbefore[i21+*(str+i12)-0x30];
        i12--;
		i21+=10;
	  }
	  
// Восстанавливаем i12 до точки
      i12=i4+1;
      i21=0;
      while (*(str+i12)) {
        numDouble+= dafter[i21+*(str+i12)-0x30];
        i12++;
	  i21+=10;
	  }
	  
	  return numDouble;
}


#define ATOF FASTatof
//#define ATOF myATOF
//#define ATOF atof
//#define ATOF variant2

enum 
{
	TOK_ERR_PARAMS,
	UNK_TOK=-1, 
	V_TOK, VN_TOK, VT_TOK, O_TOK, F_TOK,
	mtllib_TOK, usemtl_TOK, newmtl_TOK, endmtl_TOK, 
	diffuse_TOK, specular_TOK, metalic_TOK, specular_coefficient_TOK 
};

struct tok_act
{
	unsigned long hsh;
	char tok_id;
};

struct sort_class_tok_act
{
	bool operator() (tok_act i, tok_act j)
	{ return ((i.hsh)<(j.hsh));}
} sort_objectX;
		
	
#define NMAX 3854
#define Base 65521       // largest prime smaller than 65536

union s1s2
{
	unsigned int ss[2];
	unsigned long lll;
};

//longword = unsigned 32bit
//string to adler32 checksum
unsigned long adler32s(char *s)
{
	unsigned int s1, s2, len, i;
	int K;

	s1s2 res;

	i=0;
  
  s1 = 0;
  s2 = 1;
  len=strlen(s);

  if(!len)
	  return 1;
  
  while(len > 0)
  {
      if(len < NMAX)
		 K = len;
      else K = NMAX;

      len-= K;
      while(K > 0)
      {
		s1 += s[i];
        s2 += s1;
        K--;
        i++;
      }
	  
      //s1 %= Base;
	  while(s1>=Base) s1-=Base;
      //s2 %= Base;
	  while(s2>=Base) s2-=Base;
  }
   
   res.ss[0] = s1;
   res.ss[1] = s2;

   //return ((s2 << 16) |  s1);
   return res.lll;
}

namespace hstd {

namespace rt {	
	
class OBJOperator {
private:
	
	static Int3 face_to_data1(const char *input) {
		std::string tmp;
		int data[3] = {-1, -1, -1};
		int now_data_index = 0;
		int now_index = 0;
		int inputl = strlen(input);

		for (;now_index < inputl; ++now_index) {
			if (input[now_index] == '/') {
				
				if (tmp.size()) {
					data[now_data_index] = atoi(tmp.c_str());
					tmp = "";
				}
				++now_data_index;

				continue;
			}

			tmp += input[now_index];
		}

		if (tmp.size()) {
			data[now_data_index] = atoi(tmp.c_str());
			++now_data_index;
		}

		return Int3(data[0], data[1], data[2]);
	}
public:
	static bool load_material(char *filename, MaterialMap *matmap) {
				
		FileManager manager;
		if (!manager.load(filename))
			return false;
		
		printf("load_material %s\n", filename);

		std::string now_material_name;
		Float3 diffuse;
		Float3 specular;
		float specular_coefficient = 0;
		float metalic = 0;
		
			char *str, *token;
			char delim = ' ';

			int strl,i;
			int tok_l;
		
		unsigned long key, v;
				int l, h, c, iis;

			std::vector <char*> ret;
			
			std::vector <tok_act> tokens;
			tok_act ta;
			
			//hash all tokens
			ta.hsh 		= adler32s("newmtl");
			ta.tok_id	= newmtl_TOK;
			tokens.push_back(ta);
			
			ta.hsh 		= adler32s("endmtl");
			ta.tok_id	= endmtl_TOK;
			tokens.push_back(ta);
			
			ta.hsh 		= adler32s("diffuse");
			ta.tok_id	= diffuse_TOK;
			tokens.push_back(ta);
			
			ta.hsh 		= adler32s("specular");
			ta.tok_id	= specular_TOK;
			tokens.push_back(ta);
			
			ta.hsh 		= adler32s("metalic");
			ta.tok_id	= metalic_TOK;
			tokens.push_back(ta);
			
			ta.hsh 		= adler32s("specular_coefficient");
			ta.tok_id	= specular_coefficient_TOK;
			tokens.push_back(ta);
			
			/*
			
			  Ka 1,000 1,000 0,000         # Цвет окружающего освещения (желтый)
			  Kd 1,000 1,000 1,000         # Диффузный цвет (белый)
			  # Параметры отражения
			  Ks 0,000 0,000 0,000         # Цвет зеркального отражения (0;0;0 - выключен)
			  Ns 10,000                    # Коэффициент зеркального отражения (от 0 до 1000)
			  # Параметры прозрачности
			  d 0,9                        # Прозрачность указывается с помощью директивы d
			  Tr 0,9                       #   или в других реализациях формата с помощью Tr

			  */
		
			//sort by hash value
			sort (tokens.begin(), tokens.end(), sort_objectX);
			
			int c0 = tokens.size()>>1;
			unsigned long hsh0 = tokens[c0].hsh;
	
	unsigned long long lines=0;

		
		for (;;) {
			std::string now_line;
			if (!manager.gets(&now_line))
				break;

			//puts(now_line.c_str());
			
			if (now_line.size() == 0 || now_line[0] == '#') {
				continue;
			}
			
			/*
			std::vector<std::string> ret = split(now_line, ' ');
			
			std::string ret0 = ret[0];
			*/
			
			//split string
			str = (char*)now_line.c_str();
			strl = strlen(str);
			
			ret.clear();
			ret.push_back(str);
			iis=1;

			for(i=1;i<strl;i++)
			{
				if(str[i]==delim)
				{
					if(strl>(i+1))
					{
						str[i]='\0';
						ret.push_back(str+i+1);
						iis++;
					}
				}
			}

			token = ret[0];
			int tok=UNK_TOK;
				
				//hash token
				key = adler32s(token);
				//std::cout << std::endl <<"TOKEN "<<token <<" "<<key<<std::endl;

				//binary search token
				l=0;
				h=tokens.size()-1;

					c=c0;
					v=hsh0;
				
				while((h-l)>1 && v!=key)
				{
					if(v<key)
						l=c;
					else
						if(v>key)
							h=c;
						else
							break;
						
					c=(h+l)>>1;
					v=tokens[c].hsh;
				}

				//std::cout << "v="<<v<<std::endl;
				
				if(v==key)
				{
					tok = tokens[c].tok_id;
				}
				else
					if(tokens[l].hsh == key)
					{
						tok = tokens[l].tok_id;
						//std::cout << "[l]="<<tokens[l].hsh<<std::endl;
					}
					else
						if(tokens[h].hsh == key)
						{
							tok == tokens[h].tok_id;
						//std::cout << "[h]="<<tokens[h].hsh<<std::endl;
						}
						
						/*
					if(tok == UNK_TOK)
						std::cout<<"not find"<<std::endl;
					else
					std::cout << "find!"<<std::endl;
				*/
				
				
			//compare token ids
			//if (!strcmp(token, "newmtl")) {
			if (tok == newmtl_TOK) {
				
				if(iis>2) strcpy(ret[1], ((char*)now_line.c_str())+7);
				now_material_name = ret[1];
			} else 
				//if (!strcmp(token, "endmtl")) {
				if (tok == endmtl_TOK) {
				matmap->insert(std::pair<std::string, Material>(now_material_name, Material(diffuse, specular, specular_coefficient, metalic)));
				now_material_name = "";
			} else 
				//if (!strcmp(token, "diffuse") && ret.size() >= 4) {
				if (tok == diffuse_TOK && iis >= 4) {
				const float x = ATOF(ret[1]);
				const float y = ATOF(ret[2]);
				const float z = ATOF(ret[3]);
				diffuse = Float3(x, y, z);
			} else 
				//if (!strcmp(token,  "specular") && ret.size() >= 4) {
				if (tok == specular_TOK && iis >= 4) {
				const float x = ATOF(ret[1]);
				const float y = ATOF(ret[2]);
				const float z = ATOF(ret[3]);
				specular = Float3(x, y, z);
			} else 
				//if (!strcmp(token, "metalic") && ret.size() >= 2) {
				if (tok == metalic_TOK && iis >= 2) {
				const float x = ATOF(ret[1]);
				metalic = x;
			} else 
				//if (!strcmp(token, "specular_coefficient") && ret.size() >= 2) {
				if (tok == specular_coefficient_TOK && iis >= 2) {
				const float x = ATOF(ret[1]);
				specular_coefficient = x;
			}

			lines++;
		}

		return true;
	}

	static bool load(const char *filename, TriangleMesh *mesh) {
		// 一行ずつ処理すんぞ
		/*
		FILE *fp = fopen(filename, "rb");
		 
		if (fp == NULL) {
			std::cerr << "Load Error: " << filename << std::endl;
			return false;
		}
		
		// いまどきメモリたくさんあるっしょ
		// ってことで一括読み込み
		const long now_pos = ftell(fp);
		fseek(fp, 0, SEEK_END);
		const long end_pos = ftell(fp);
		fseek(fp, now_pos, SEEK_SET);

		const ULONGLONG total_size = end_pos - now_pos;
		fclose(fp);

		MEMORYSTATUSEX statex;

		statex.dwLength = sizeof (statex);

		GlobalMemoryStatusEx (&statex);
		
		const ULONGLONG freemem = statex.ullAvailPageFile;
		
		bool use_memfile = (total_size < (freemem*0.7f));
		
		
		_tprintf (TEXT("free mem: %*I64d KB\n"),
            7, freemem/1024);
			
		_tprintf (TEXT("file size: %*I64d KB\n"),
            7, total_size/1024);
		
		std::cout<<"use_memfile="<<use_memfile<<std::endl;
		*/

prepareMy();
prepareVariant2();
	
		FileManager manager;
		if (!manager.load(filename))
			return false;		
		

		Timer timer;
		float t0,t1;
		float elapsedTime;

	
	unsigned long long lines=0;
			
		printf("objMesh load %s\n",filename);	

		timer.begin();
		
		t0=timer.end();

		MeshBody mesh_body;
		Material *now_material = NULL;


		//tigra: default white diffuse
		now_material = new Material(Float3(1, 1, 1), Float3(0, 0, 0), 0, 0);
		
		unsigned long key, v;
				int l, h, c, iis;


			long vs, os, fs, vns, vts;
			
			vs=os=fs=vns=vts=0;

			char *str, *token;
			char delim = ' ';

			int strl,i;
			int tok_l;

			std::vector <char*> ret;
			
			std::vector <tok_act> tokens;
			tok_act ta;
			
			ta.hsh 		= adler32s("mtllib");
			ta.tok_id	= mtllib_TOK;
			tokens.push_back(ta);
			//std::cout << "mtllib " << ta.hsh << std::endl;
			
			ta.hsh 		= adler32s("usemtl");
			ta.tok_id	= usemtl_TOK;
			tokens.push_back(ta);
			//std::cout << "usemtl " << ta.hsh << std::endl;
		
			sort (tokens.begin(), tokens.end(), sort_objectX);
			
			/*
			for(i=0;i<tokens.size();i++)
				std::cout << "[" <<i <<"]="<<tokens[i].hsh<<std::endl;
			*/
			
			int c0 = tokens.size()>>1;
			unsigned long hsh0 = tokens[c0].hsh;


			
		for (;;) {
			std::string now_line;
			if (!manager.gets(&now_line))
				break;


			if (now_line.size() == 0 || now_line[0] == '#') {
				continue;
			}

			str = (char*)now_line.c_str();

			strl = strlen(str);

			/*
			std::vector<std::string> ret = split(now_line, ' ');
			
			std::string token = ret[0];
			*/

			ret.clear();
			ret.push_back(str);
			iis=1;

			for(i=1;i<strl;i++)
			{
				if(str[i]==delim)
				{
					if(strl>(i+1))
					{
						str[i]='\0';
						ret.push_back(str+i+1);
						iis++;
					}
				}
			}
			
			
			tok_l=strlen(ret[0]);

			token = ret[0];

			unsigned char ch = token[0];

			//ret[0] лучше хэшировать и сравнивать с хэшироваными версиями строк
			
			
			int tok=UNK_TOK;

			if(tok_l>=1 && tok_l<3)
			{				
				if(tok_l==1)
				{
					if(ch=='v')
						{
							tok=V_TOK;
							vs++;
						}
					else
					if(ch=='o')
						{
							tok=O_TOK;
							os++;
						}
					else
					if(ch=='f')
					{
						tok=F_TOK;
						fs++;
					}
				}
				else
				if(tok_l==2)
				{
					if(ch=='v')
					{
						ch=token[1];
						if(ch=='n')
							{
								tok=VN_TOK;
								vns++;
							}
						else
						if(ch=='t')
							{
								tok=VT_TOK;
								vts++;
							}
					}
				}
			}

			if(tok==UNK_TOK)
			{

				key = adler32s(token);
				//std::cout << std::endl <<"TOKEN "<<token <<" "<<key<<std::endl;

				l=0;
				h=tokens.size()-1;

					c=c0;
					v=hsh0;
				
				while((h-l)>1 && v!=key)
				{
					if(v<key)
						l=c;
					else
						if(v>key)
							h=c;
						else
							break;
						
					c=(h+l)>>1;
					v=tokens[c].hsh;
				}

				//std::cout << "v="<<v<<std::endl;
				
				if(v==key)
				{
					tok = tokens[c].tok_id;
				}
				else
					if(tokens[l].hsh == key)
					{
						tok = tokens[l].tok_id;
						//std::cout << "[l]="<<tokens[l].hsh<<std::endl;
					}
					else
						if(tokens[h].hsh == key)
						{
							tok == tokens[h].tok_id;
						//std::cout << "[h]="<<tokens[h].hsh<<std::endl;
						}
						
						/*
					if(tok == UNK_TOK)
						std::cout<<"not find"<<std::endl;
					else
					std::cout << "find!"<<std::endl;
				*/
				
			//if (!strcmp(token,"mtllib")) {
			if (tok == mtllib_TOK) {
				// マテリアルの読み込み


				if (!load_material(ret[1], &mesh_body.matmap))
					return false;

				/*
				MaterialMap::iterator result = mesh_body.matmap.find("Material.001");
				std::cout << &(result->second) << std::endl;
				*/

			} else 
				//if (!strcmp(token,"usemtl")) {
				if (tok == usemtl_TOK) {
					//(char*)now_line.c_str();
				if(iis>2) strcpy(ret[1], ((char*)now_line.c_str())+7);
				MaterialMap::iterator result = mesh_body.matmap.find(ret[1]);
				if (result == mesh_body.matmap.end()) {
					//tigra: default white diffuse
					now_material = new Material(Float3(1, 1, 1), Float3(0, 0, 0), 0, 0);
				} else {
//					now_material = &(result->second);

//					std::cout << result->second.specular << std::endl;

					now_material = new Material(result->second);
				}
			}
			else				
				{
					//unk_tok
					//printf("%s %s\n",ret[0],ret[1]);
				}
			}
			else 
			{
				switch(tok)
				{
				case O_TOK:
					//if (ret[0] == "o") {
				// めんどいからとりあえずoはなしね
				// имя обьекта игнорируется
					//std::cout << "o "<< ret[1] << std::endl;
					break;

				case V_TOK:
					//} else if (ret[0] == "v" && 
					if(iis >= 4) {
						const float x = ATOF(ret[1]);
						const float y = ATOF(ret[2]);
						const float z = ATOF(ret[3]);
						
						//только для hairball - умножить на 2.9 и по у -0.01 
						//mesh_body.v.push_back(Float3(x*2.9f, y*2.9f-0.01f, z*2.9f));
						mesh_body.v.push_back(Float3(x, y, z));
						} 
					else tok = TOK_ERR_PARAMS;
					break;

				case VN_TOK:
					//else if (ret[0] == "vn" && 
					if(iis >= 4) {
						const float x = ATOF(ret[1]);
						const float y = ATOF(ret[2]);
						const float z = ATOF(ret[3]);
						mesh_body.vn.push_back(Float3(x, y, z));
					} 
					else tok = TOK_ERR_PARAMS;
					break;

				case VT_TOK:
					//else if (ret[0] == "vt" && ret.size() >= 4) {
					if(iis >= 4) {
						const float x = ATOF(ret[1]);
						const float y = ATOF(ret[2]);
						const float z = ATOF(ret[3]);
						mesh_body.vt.push_back(Float3(x, y, z));
					} 
					else tok = TOK_ERR_PARAMS;
					break;

				case F_TOK:
					//else if (ret[0] == "f" && ret.size() >= 4) {
					if(iis >= 4) {
						const Int3 f1 = face_to_data1(ret[1]);
						const Int3 f2 = face_to_data1(ret[2]);
						const Int3 f3 = face_to_data1(ret[3]);
						Triangle t;

						t.v_index  = Int3(f1.x, f2.x, f3.x) - Int3(1, 1, 1);
						t.vt_index = Int3(f1.y, f2.y, f3.y) - Int3(1, 1, 1);
						t.vn_index = Int3(f1.z, f2.z, f3.z) - Int3(1, 1, 1);
						t.material = now_material;

						mesh_body.triangle.push_back(t); 
					} 
					else tok = TOK_ERR_PARAMS;
					break;
				}
			}

			lines++;
		}
		
		//MaterialMap::iterator result = mesh_body.matmap.find("Material.001");

		//если закоментить, материалы не сработают
		//fixed!

		//std::cout << &(result->second) << std::endl;
		//std::cout << "result->second.specular=" << result->second.specular << std::endl;
		

		t1 = timer.end();		
		
		elapsedTime = float((t1 - t0) / CLOCKS_PER_SEC );
		
		printf("%0.2fs\n\n",elapsedTime);
		
		
	printf("vs: %ld\n", vs);
	printf("vns: %ld\n", vns);
	printf("vts: %ld\n", vts);
	printf("os: %ld\n", os);
	printf("fs: %ld\n", fs);
	printf("LINES: %ld\n\n", lines);

		mesh->set(mesh_body);

		// BVH作る
		std::vector<RefTriangle> ref_triangle;

		for (int i = 0; i < mesh_body.triangle.size(); ++i) {
			const Int3 v_index = mesh_body.triangle[i].v_index;
			const Float3 v0 = mesh_body.v[v_index.x];
			const Float3 v1 = mesh_body.v[v_index.y];
			const Float3 v2 = mesh_body.v[v_index.z];

			RefTriangle tr(&mesh_body.v[v_index.x], &mesh_body.v[v_index.y], &mesh_body.v[v_index.z], i);
			ref_triangle.push_back(tr);
		}

		printf("objMesh build QBVH for %d triangles...\n",
		mesh_body.triangle.size());

		timer.begin();
		
		t0=timer.end();

		mesh->build(ref_triangle);

		t1 = timer.end();		
		
		elapsedTime = float((t1 - t0) / CLOCKS_PER_SEC );
		
		printf("%0.2fs\n",elapsedTime);

		printf("done\n");

		return true;
	}
};



} // namespace rt

} // namespace hstd


#endif // _OBJ_MESH_