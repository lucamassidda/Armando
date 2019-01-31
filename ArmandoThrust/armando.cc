#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/sort.h>
#include <thrust/remove.h>
#include <thrust/binary_search.h>
#include <thrust/iterator/counting_iterator.h>
#include <thrust/for_each.h>
#include <thrust/system_error.h>

#define MAXN 24
//#define MAXN 64
#define PI 3.14159f

#define THREADS 256

struct grid {
    float oX, oY, oZ;
    float size;
    int nX, nY, nZ;
};

struct simulation {
    float minX, maxX;
    float minY, maxY;
    float minZ, maxZ;
    float dt;
    int tsn;
    int ssi;
    int nsi;
};

struct load {
    float minX, maxX;
    float minY, maxY;
    float minZ, maxZ;
    float gx;
    float gy;
    float gz;
    float w;
};

struct disp {
    float oX, oY, oZ;
    float nX, nY, nZ;
    float dX, dY, dZ;
};

struct host_model {
    int pn;
    thrust::host_vector<int> Material;
    thrust::host_vector<float> Mass;
    thrust::host_vector<float> Smooth;
    thrust::host_vector<float> PosX;
    thrust::host_vector<float> PosY;
    thrust::host_vector<float> PosZ;
    thrust::host_vector<float> VelX;
    thrust::host_vector<float> VelY;
    thrust::host_vector<float> VelZ;
    thrust::host_vector<float> Density;
    thrust::host_vector<float> Energy;
    thrust::host_vector<float> Pressure;
    thrust::host_vector<float> Sound;
    thrust::host_vector<float> VelDotX;
    thrust::host_vector<float> VelDotY;
    thrust::host_vector<float> VelDotZ;
    thrust::host_vector<float> DensityDot;
    thrust::host_vector<float> EnergyDot;
    thrust::host_vector<float> PosX0;
    thrust::host_vector<float> PosY0;
    thrust::host_vector<float> PosZ0;
    thrust::host_vector<float> VelX0;
    thrust::host_vector<float> VelY0;
    thrust::host_vector<float> VelZ0;
    thrust::host_vector<float> Density0;
    thrust::host_vector<float> Energy0;
    thrust::host_vector<int> List;
    thrust::host_vector<int> Hash;
    thrust::host_vector<int> Index;
    thrust::host_vector<int> SetStart;
    thrust::host_vector<int> SetStop;
    thrust::host_vector<int> IntDummy;
    thrust::host_vector<float> FloatDummy;
};

struct device_model {
    int pn;
    thrust::device_vector<int> Material;
    thrust::device_vector<float> Mass;
    thrust::device_vector<float> Smooth;
    thrust::device_vector<float> PosX;
    thrust::device_vector<float> PosY;
    thrust::device_vector<float> PosZ;
    thrust::device_vector<float> VelX;
    thrust::device_vector<float> VelY;
    thrust::device_vector<float> VelZ;
    thrust::device_vector<float> Density;
    thrust::device_vector<float> Energy;
    thrust::device_vector<float> Pressure;
    thrust::device_vector<float> Sound;
    thrust::device_vector<float> VelDotX;
    thrust::device_vector<float> VelDotY;
    thrust::device_vector<float> VelDotZ;
    thrust::device_vector<float> DensityDot;
    thrust::device_vector<float> EnergyDot;
    thrust::device_vector<float> PosX0;
    thrust::device_vector<float> PosY0;
    thrust::device_vector<float> PosZ0;
    thrust::device_vector<float> VelX0;
    thrust::device_vector<float> VelY0;
    thrust::device_vector<float> VelZ0;
    thrust::device_vector<float> Density0;
    thrust::device_vector<float> Energy0;
    thrust::device_vector<int> List;
    thrust::device_vector<int> Hash;
    thrust::device_vector<int> Index;
    thrust::device_vector<int> SetStart;
    thrust::device_vector<int> SetStop;
    thrust::device_vector<int> IntDummy;
    thrust::device_vector<float> FloatDummy;
};

struct pointer_model {
    int pn;
    int* Material;
    float* Mass;
    float* Smooth;
    float* PosX;
    float* PosY;
    float* PosZ;
    float* VelX;
    float* VelY;
    float* VelZ;
    float* Density;
    float* Energy;
    float* Pressure;
    float* Sound;
    float* VelDotX;
    float* VelDotY;
    float* VelDotZ;
    float* DensityDot;
    float* EnergyDot;
    float* PosX0;
    float* PosY0;
    float* PosZ0;
    float* VelX0;
    float* VelY0;
    float* VelZ0;
    float* Density0;
    float* Energy0;
    int* List;
    int* Hash;
    int* Index;
    int* SetStart;
    int* SetStop;
    int* IntDummy;
    float* FloatDummy;
};


// Host Variables

//float hSmooth, hMass, hSound;
int hMatType[10];
float hMatProp[10][10];
struct simulation hRun;
struct grid hGrid;
struct load hLoad[10];
struct disp hDisp[10];

// Device Variables

//__device__ __constant__ float dSmooth, dMass, dSound;
__device__ __constant__ int dMatType[10];
__device__ __constant__ float dMatProp[10][10];
__device__ __constant__ struct simulation dRun;
__device__ struct grid dGrid;
__device__ struct load dLoad[10];
__device__ struct disp dDisp[10];


void initPump(struct host_model *hm) {

    int i, j, k, m, p;
    double rho, c0, pmin;
    double dr;

    p = 1;

    m = 1;
    rho = 1000.f;
    c0 = 50.f;
    pmin = -1.e12;

    hMatType[m] = 4;
    hMatProp[m][0] = rho;
    hMatProp[m][1] = c0;
    hMatProp[m][2] = pmin;

    dr = 0.02f / p;

    for (k = 0; k < 20 * p ; k++) {
        for (j = 0; j < 20 * p ; j++) {
            for (i = 0; i < 40 * p; i++) {
                hm->PosX.push_back(i * dr);
                hm->PosY.push_back(j * dr);
                hm->PosZ.push_back(k * dr);
                hm->Material.push_back(m);
            }
        }
    }

    for (k = 0; k < 20 * p ; k++) {
        for (j = 20 * p; j < 50 * p ; j++) {
            for (i = 0; i < 20 * p -1; i++) {
                hm->PosX.push_back(i * dr);
                hm->PosY.push_back(j * dr);
                hm->PosZ.push_back(k * dr);
                hm->Material.push_back(m);
            }
        }
    }

    for (k = 0; k < 20 * p ; k++) {
        for (j = 20 * p; j < 50 * p ; j++) {
            for (i = 20 * p +1; i < 40 * p; i++) {
                hm->PosX.push_back(i * dr);
                hm->PosY.push_back(j * dr);
                hm->PosZ.push_back(k * dr);
                hm->Material.push_back(m);
            }
        }
    }

    for (k = 0; k < 20 * p ; k++) {
        for (j = -3; j < 0; j++) {
            for (i = -3; i < 40 * p + 3; i++) {
                hm->PosX.push_back(i * dr);
                hm->PosY.push_back(j * dr);
                hm->PosZ.push_back(k * dr);
                hm->Material.push_back(-m);
            }
        }
    }

    for (k = 0; k < 20 * p ; k++) {
        for (j = 0; j < 80 * p; j++) {
            for (i = -3; i < 0; i++) {
                hm->PosX.push_back(i * dr);
                hm->PosY.push_back(j * dr);
                hm->PosZ.push_back(k * dr);
                hm->Material.push_back(-m);
            }
        }
    }

    for (k = 0; k < 20 * p ; k++) {
        for (j = 0; j < 80 * p; j++) {
            for (i = 40 * p; i < 40 * p +3; i++) {
                hm->PosX.push_back(i * dr);
                hm->PosY.push_back(j * dr);
                hm->PosZ.push_back(k * dr);
                hm->Material.push_back(-m);
            }
        }
    }

    for (k = 0; k < 20 * p ; k++) {
        for (j = 20 * p; j < 60 * p; j++) {
            for (i = 20 * p -1; i < 20 * p +1; i++) {
                hm->PosX.push_back(i * dr);
                hm->PosY.push_back(j * dr);
                hm->PosZ.push_back(k * dr);
                hm->Material.push_back(-m);
            }
        }
    }

    for (k = -3; k < 0 ; k++) {
        for (j = -3; j < 80 * p ; j++) {
            for (i = -3; i < 40 * p +3; i++) {
                hm->PosX.push_back(i * dr);
                hm->PosY.push_back(j * dr);
                hm->PosZ.push_back(k * dr);
                hm->Material.push_back(-m);
            }
        }
    }

    for (k = 20 * p; k < 20 * p +3; k++) {
        for (j = -3; j < 80 * p ; j++) {
            for (i = -3; i < 40 * p +3; i++) {
                hm->PosX.push_back(i * dr);
                hm->PosY.push_back(j * dr);
                hm->PosZ.push_back(k * dr);
                hm->Material.push_back(-m);
            }
        }
    }

    hm->pn = hm->Material.size();
    if ((hm->PosX.size() != hm->pn) ||
            (hm->PosY.size() != hm->pn) ||
            (hm->PosZ.size() != hm->pn)) {
        printf("Wrong vector size\n");
        exit(-1);
    }

    hm->Mass.resize(hm->pn);
    hm->Smooth.resize(hm->pn);
    hm->VelX.resize(hm->pn);
    hm->VelY.resize(hm->pn);
    hm->VelZ.resize(hm->pn);
    hm->Density.resize(hm->pn);
    hm->Energy.resize(hm->pn);
    hm->Pressure.resize(hm->pn);
    hm->Sound.resize(hm->pn);

    thrust::fill(hm->Mass.begin(), hm->Mass.end(), rho * dr * dr * dr);
    thrust::fill(hm->Smooth.begin(), hm->Smooth.end(), 1.2f * dr);
    thrust::fill(hm->VelX.begin(), hm->VelX.end(), 0.0f);
    thrust::fill(hm->VelY.begin(), hm->VelY.end(), 0.0f);
    thrust::fill(hm->VelZ.begin(), hm->VelZ.end(), 0.0f);
    thrust::fill(hm->Density.begin(), hm->Density.end(), rho);
    thrust::fill(hm->Energy.begin(), hm->Energy.end(), 0.0f);
    thrust::fill(hm->Pressure.begin(), hm->Pressure.end(), 0.0f);
    thrust::fill(hm->Sound.begin(), hm->Sound.end(), c0);

    hm->VelDotX.resize(hm->pn);
    hm->VelDotY.resize(hm->pn);
    hm->VelDotZ.resize(hm->pn);
    hm->DensityDot.resize(hm->pn);
    hm->EnergyDot.resize(hm->pn);

    hm->PosX0.resize(hm->pn);
    hm->PosY0.resize(hm->pn);
    hm->PosZ0.resize(hm->pn);
    hm->VelX0.resize(hm->pn);
    hm->VelY0.resize(hm->pn);
    hm->VelZ0.resize(hm->pn);
    hm->Density0.resize(hm->pn);
    hm->Energy0.resize(hm->pn);

    hm->List.resize(hm->pn*MAXN);
    hm->Hash.resize(hm->pn);
    hm->Index.resize(hm->pn);

    hRun.minX = -0.5f;
    hRun.maxX =  2.0f;
    hRun.minY = -0.5f;
    hRun.maxY =  2.0f;
    hRun.minZ = -0.5f;
    hRun.maxZ =  2.0f;

    hRun.dt = 2.0e-4 / p; //1.0e-3;
    //hRun.dt = 0.5f * hSmooth / c0;
    hRun.tsn = 4000 * p; //1000;
    hRun.ssi = 100 * p;

    hGrid.oX = hRun.minX;
    hGrid.oY = hRun.minY;
    hGrid.oZ = hRun.minZ;
    hGrid.size = 2.0f * 1.2f * dr;
    hGrid.nX = (int) ((hRun.maxX - hRun.minX) / hGrid.size) +1;
    hGrid.nY = (int) ((hRun.maxY - hRun.minY) / hGrid.size) +1;
    hGrid.nZ = (int) ((hRun.maxZ - hRun.minZ) / hGrid.size) +1;

    hm->SetStart.resize(hGrid.nX * hGrid.nY * hGrid.nZ);
    hm->SetStop.resize(hGrid.nX * hGrid.nY * hGrid.nZ);

    hm->IntDummy.resize(hm->pn);
    hm->FloatDummy.resize(hm->pn);

    hLoad[0].minX = hRun.minX;
    hLoad[0].maxX = hRun.maxX;
    hLoad[0].minY = hRun.minY;
    hLoad[0].maxY = hRun.maxY;
    hLoad[0].minZ = hRun.minZ;
    hLoad[0].maxZ = hRun.maxZ;
    hLoad[0].gx = 0.0f;
    hLoad[0].gy = -9.81f;
    hLoad[0].gz = 0.0f;

    hLoad[1].minX = dr * (20 * p);
    hLoad[1].maxX = dr * (40 * p);
    hLoad[1].minY = dr * (20 * p);
    hLoad[1].maxY = dr * (30 * p);
    hLoad[1].minZ = hRun.minZ;
    hLoad[1].maxZ = hRun.maxZ;
    hLoad[1].gx = 0.0f;
    hLoad[1].gy = 4.0f * 9.81f;
    hLoad[1].gz = 0.0f;

    printf("Pump in a box \n");
    printf("Particles: %i \n", hm->pn);
    printf("Neighbours: %i \n", hm->pn*MAXN);
    printf("Grid cells: %i \n", hGrid.nX * hGrid.nY * hGrid.nZ);
}


void initDamBreak(struct host_model *hm) {

    int i, j, m, pi, k;
    double rho, c0, pmin;
    double dr;

    k = 1;

    m = 1;
    rho = 1000.f;
    c0 = 50.f;
    pmin = -1.e12;

    hMatType[m] = 4;
    hMatProp[m][0] = rho;
    hMatProp[m][1] = c0;
    hMatProp[m][2] = pmin;

    dr = 0.02f / k; // x4
    pi = 0;

    for (j = 0; j <= 50 * k ; j++) {
        for (i = 0; i <= 100 * k; i++) {
            hm->PosX.push_back(i * dr + 0.5f * dr);
            hm->PosY.push_back(j * dr + 0.5f * dr);
            hm->PosZ.push_back(0.0f);
            hm->Material.push_back(m);
            pi++;
        }
    }

    for (j = -3; j <= -1; j++) {
        for (i = -3; i <= 269 * k + 2; i++) {
            hm->PosX.push_back(i * dr);
            hm->PosY.push_back(j * dr);
            hm->PosZ.push_back(0.0f);
            hm->Material.push_back(-m);
            pi++;
        }
    }

    for (j = -0; j <= 80 * k; j++) {
        for (i = -3; i <= -1; i++) {
            hm->PosX.push_back(i * dr);
            hm->PosY.push_back(j * dr);
            hm->PosZ.push_back(0.0f);
            hm->Material.push_back(-m);
            pi++;
        }
    }

    for (j = -0; j <= 80 * k; j++) {
        for (i = 269 * k; i <= 269 * k +2; i++) {
            hm->PosX.push_back(i * dr);
            hm->PosY.push_back(j * dr);
            hm->PosZ.push_back(0.0f);
            hm->Material.push_back(-m);
            pi++;
        }
    }

    hm->pn = hm->Material.size();
    if ((hm->PosX.size() != hm->pn) ||
            (hm->PosY.size() != hm->pn) ||
            (hm->PosZ.size() != hm->pn)) {
        printf("Wrong vector size\n");
        exit(-1);
    }

    hm->Mass.resize(hm->pn);
    hm->Smooth.resize(hm->pn);
    hm->VelX.resize(hm->pn);
    hm->VelY.resize(hm->pn);
    hm->VelZ.resize(hm->pn);
    hm->Density.resize(hm->pn);
    hm->Energy.resize(hm->pn);
    hm->Pressure.resize(hm->pn);
    hm->Sound.resize(hm->pn);

    thrust::fill(hm->Mass.begin(), hm->Mass.end(), rho * dr * dr);
    thrust::fill(hm->Smooth.begin(), hm->Smooth.end(), 1.2f * dr);
    thrust::fill(hm->VelX.begin(), hm->VelX.end(), 0.0f);
    thrust::fill(hm->VelY.begin(), hm->VelY.end(), 0.0f);
    thrust::fill(hm->VelZ.begin(), hm->VelZ.end(), 0.0f);
    thrust::fill(hm->Density.begin(), hm->Density.end(), rho);
    thrust::fill(hm->Energy.begin(), hm->Energy.end(), 0.0f);
    thrust::fill(hm->Pressure.begin(), hm->Pressure.end(), 0.0f);
    thrust::fill(hm->Sound.begin(), hm->Sound.end(), c0);

    hm->VelDotX.resize(hm->pn);
    hm->VelDotY.resize(hm->pn);
    hm->VelDotZ.resize(hm->pn);
    hm->DensityDot.resize(hm->pn);
    hm->EnergyDot.resize(hm->pn);
    hm->List.resize(hm->pn*MAXN);
    hm->Hash.resize(hm->pn);
    hm->Index.resize(hm->pn);

    hm->PosX0.resize(hm->pn);
    hm->PosY0.resize(hm->pn);
    hm->PosZ0.resize(hm->pn);
    hm->VelX0.resize(hm->pn);
    hm->VelY0.resize(hm->pn);
    hm->VelZ0.resize(hm->pn);
    hm->Density0.resize(hm->pn);
    hm->Energy0.resize(hm->pn);

    hRun.minX = -1.0f;
    hRun.maxX =  6.0f;
    hRun.minY = -1.0f;
    hRun.maxY =  4.0f;
    hRun.minZ = -1.0f;
    hRun.maxZ =  1.0f;

    hRun.dt = 4.0e-4 / k; //1.0e-3;
    hRun.tsn = 10000 * k; //1000;
    hRun.ssi = 200 * k;

    hGrid.oX = hRun.minX;
    hGrid.oY = hRun.minY;
    hGrid.oZ = hRun.minZ;
    hGrid.size = 2.0f * 1.2f * dr;
    hGrid.nX = (int) ((hRun.maxX - hRun.minX) / hGrid.size) +1;
    hGrid.nY = (int) ((hRun.maxY - hRun.minY) / hGrid.size) +1;
    hGrid.nZ = (int) ((hRun.maxZ - hRun.minZ) / hGrid.size) +1;

    hm->SetStart.resize(hGrid.nX * hGrid.nY * hGrid.nZ);
    hm->SetStop.resize(hGrid.nX * hGrid.nY * hGrid.nZ);

    hm->IntDummy.resize(hm->pn);
    hm->FloatDummy.resize(hm->pn);

    hLoad[0].minX = hRun.minX;
    hLoad[0].maxX = hRun.maxX;
    hLoad[0].minY = hRun.minY;
    hLoad[0].maxY = hRun.maxY;
    hLoad[0].minZ = hRun.minZ;
    hLoad[0].maxZ = hRun.maxZ;
    hLoad[0].gy = -9.81f;

    printf("Dam break in a box \n");
    printf("Particles: %i \n", hm->pn);
}


void initChannel(struct host_model *hm) {

    int i, j, m, pi, k;
    double rho, c0, pmin;
    double dr;

    k = 1;

    m = 1;
    rho = 1000.f;
    c0 = 50.f;
    pmin = -1.e12;

    hMatType[m] = 4;
    hMatProp[m][0] = rho;
    hMatProp[m][1] = c0;
    hMatProp[m][2] = pmin;

    dr = 0.02f / k; // x4
    pi = 0;

    for (j = 0; j < 10 * k ; j++) {
        for (i = 0; i < 100 * k; i++) {
            hm->PosX.push_back(i * dr);
            hm->PosY.push_back(j * dr);
            hm->PosZ.push_back(0.0f);
            hm->Material.push_back(m);
            pi++;
        }
    }

    for (j = -3; j < 0; j++) {
        for (i = -10; i < 100 * k + 10; i++) {
            hm->PosX.push_back(i * dr);
            hm->PosY.push_back(j * dr);
            hm->PosZ.push_back(0.0f);
            hm->Material.push_back(-m);
            pi++;
        }
    }

    for (j = 0; j < 15 * k; j++) {
        for (i = -3; i < 0; i++) {
            hm->PosX.push_back(i * dr);
            hm->PosY.push_back(j * dr);
            hm->PosZ.push_back(0.0f);
            hm->Material.push_back(-m);
            pi++;
        }
    }

    hm->pn = hm->Material.size();
    if ((hm->PosX.size() != hm->pn) ||
            (hm->PosY.size() != hm->pn) ||
            (hm->PosZ.size() != hm->pn)) {
        printf("Wrong vector size\n");
        exit(-1);
    }

    hm->Mass.resize(hm->pn);
    hm->Smooth.resize(hm->pn);
    hm->VelX.resize(hm->pn);
    hm->VelY.resize(hm->pn);
    hm->VelZ.resize(hm->pn);
    hm->Density.resize(hm->pn);
    hm->Energy.resize(hm->pn);
    hm->Pressure.resize(hm->pn);
    hm->Sound.resize(hm->pn);

    thrust::fill(hm->Mass.begin(), hm->Mass.end(), rho * dr * dr);
    thrust::fill(hm->Smooth.begin(), hm->Smooth.end(), 1.2f * dr);
    thrust::fill(hm->VelX.begin(), hm->VelX.end(), 0.0f);
    thrust::fill(hm->VelY.begin(), hm->VelY.end(), 0.0f);
    thrust::fill(hm->VelZ.begin(), hm->VelZ.end(), 0.0f);
    thrust::fill(hm->Density.begin(), hm->Density.end(), rho);
    thrust::fill(hm->Energy.begin(), hm->Energy.end(), 0.0f);
    thrust::fill(hm->Pressure.begin(), hm->Pressure.end(), 0.0f);
    thrust::fill(hm->Sound.begin(), hm->Sound.end(), c0);

    hm->VelDotX.resize(hm->pn);
    hm->VelDotY.resize(hm->pn);
    hm->VelDotZ.resize(hm->pn);
    hm->DensityDot.resize(hm->pn);
    hm->EnergyDot.resize(hm->pn);
    hm->List.resize(hm->pn*MAXN);
    hm->Hash.resize(hm->pn);
    hm->Index.resize(hm->pn);

    hm->PosX0.resize(hm->pn);
    hm->PosY0.resize(hm->pn);
    hm->PosZ0.resize(hm->pn);
    hm->VelX0.resize(hm->pn);
    hm->VelY0.resize(hm->pn);
    hm->VelZ0.resize(hm->pn);
    hm->Density0.resize(hm->pn);
    hm->Energy0.resize(hm->pn);

    hRun.minX = -1.0f;
    hRun.maxX =  3.0f;
    hRun.minY = -1.0f;
    hRun.maxY =  1.0f;
    hRun.minZ = -0.5f;
    hRun.maxZ =  0.5f;

    hRun.dt = dr / c0;
    hRun.tsn = 10000 * k; //1000;
    hRun.ssi = 200 * k;

    hGrid.oX = hRun.minX;
    hGrid.oY = hRun.minY;
    hGrid.oZ = hRun.minZ;
    hGrid.size = 2.0f * 1.2f * dr;
    hGrid.nX = (int) ((hRun.maxX - hRun.minX) / hGrid.size) +1;
    hGrid.nY = (int) ((hRun.maxY - hRun.minY) / hGrid.size) +1;
    hGrid.nZ = (int) ((hRun.maxZ - hRun.minZ) / hGrid.size) +1;

    hm->SetStart.resize(hGrid.nX * hGrid.nY * hGrid.nZ);
    hm->SetStop.resize(hGrid.nX * hGrid.nY * hGrid.nZ);

    hm->IntDummy.resize(hm->pn);
    hm->FloatDummy.resize(hm->pn);

    hLoad[0].minX = hRun.minX;
    hLoad[0].maxX = hRun.maxX;
    hLoad[0].minY = hRun.minY;
    hLoad[0].maxY = hRun.maxY;
    hLoad[0].minZ = hRun.minZ;
    hLoad[0].maxZ = hRun.maxZ;
    hLoad[0].gy = -9.81f;

    hDisp[0].oX = 100.0f * k * dr;
    hDisp[0].oY = 0.0f;
    hDisp[0].oZ = 0.0f;
    hDisp[0].nX = 1.0f;
    hDisp[0].nY = 0.0f;
    hDisp[0].nZ = 0.0f;
    hDisp[0].dX = 1.0f;
    hDisp[0].dY = 1.0f;
    hDisp[0].dZ = 0.0f;

    printf("Channel \n");
    printf("Particles: %i \n", hm->pn);
}


void initBlock(struct host_model *hm) {

    int i, j, k, m, p;
    double rho, c0, pmin;
    double dr;

    p = 1;

    m = 1;
    rho = 1000.f;
    c0 = 50.f;
    pmin = -1.e12;

    hMatType[m] = 4;
    hMatProp[m][0] = rho;
    hMatProp[m][1] = c0;
    hMatProp[m][2] = pmin;

    dr = 1.0f / p;

    for (k = 0; k < 8 * p ; k++) {
        for (j = 0; j < 8 * p ; j++) {
            for (i = 0; i < 8 * p; i++) {
                hm->PosX.push_back(i * dr);
                hm->PosY.push_back(j * dr);
                hm->PosZ.push_back(k * dr);
                hm->Material.push_back(m);
            }
        }
    }

    hm->pn = hm->Material.size();
    if ((hm->PosX.size() != hm->pn) ||
            (hm->PosY.size() != hm->pn) ||
            (hm->PosZ.size() != hm->pn)) {
        printf("Wrong vector size\n");
        exit(-1);
    }

    thrust::fill(hm->Mass.begin(), hm->Mass.end(), rho * dr * dr);
    thrust::fill(hm->Smooth.begin(), hm->Smooth.end(), 1.2f * dr);
    thrust::fill(hm->VelX.begin(), hm->VelX.end(), 0.0f);
    thrust::fill(hm->VelY.begin(), hm->VelY.end(), 0.0f);
    thrust::fill(hm->VelZ.begin(), hm->VelZ.end(), 0.0f);
    thrust::fill(hm->Density.begin(), hm->Density.end(), rho);
    thrust::fill(hm->Energy.begin(), hm->Energy.end(), 0.0f);
    thrust::fill(hm->Pressure.begin(), hm->Pressure.end(), 0.0f);
    thrust::fill(hm->Sound.begin(), hm->Sound.end(), c0);

    hm->VelDotX.resize(hm->pn);
    hm->VelDotY.resize(hm->pn);
    hm->VelDotZ.resize(hm->pn);
    hm->DensityDot.resize(hm->pn);
    hm->EnergyDot.resize(hm->pn);
    hm->List.resize(hm->pn*MAXN);
    hm->Hash.resize(hm->pn);
    hm->Index.resize(hm->pn);

    hm->PosX0.resize(hm->pn);
    hm->PosY0.resize(hm->pn);
    hm->PosZ0.resize(hm->pn);
    hm->VelX0.resize(hm->pn);
    hm->VelY0.resize(hm->pn);
    hm->VelZ0.resize(hm->pn);
    hm->Density0.resize(hm->pn);
    hm->Energy0.resize(hm->pn);

    hRun.minX =  -1.0f;
    hRun.maxX =  9.0f;
    hRun.minY =  -1.0f;
    hRun.maxY =  9.0f;
    hRun.minZ =  -1.0f;
    hRun.maxZ =  9.0f;

    hRun.dt = 1.0e-2 / p;
    hRun.tsn = 100 * p;
    hRun.ssi = 10 * p;

    hGrid.oX = hRun.minX;
    hGrid.oY = hRun.minY;
    hGrid.oZ = hRun.minZ;
    hGrid.size = 2.0f * 1.2f * dr;
    hGrid.nX = (int) ((hRun.maxX - hRun.minX) / hGrid.size) +1;
    hGrid.nY = (int) ((hRun.maxY - hRun.minY) / hGrid.size) +1;
    hGrid.nZ = (int) ((hRun.maxZ - hRun.minZ) / hGrid.size) +1;

    hm->SetStart.resize(hGrid.nX * hGrid.nY * hGrid.nZ);
    hm->SetStop.resize(hGrid.nX * hGrid.nY * hGrid.nZ);

    hm->IntDummy.resize(hm->pn);
    hm->FloatDummy.resize(hm->pn);

    hLoad[0].minX = hRun.minX;
    hLoad[0].maxX = hRun.maxX;
    hLoad[0].minY = hRun.minY;
    hLoad[0].maxY = hRun.maxY;
    hLoad[0].minZ = hRun.minZ;
    hLoad[0].maxZ = hRun.maxZ;
    hLoad[0].gx = 0.0f;
    hLoad[0].gy = -9.81f;
    hLoad[0].gz = 0.0f;

    printf("Block \n");
    printf("Particles: %i \n", hm->pn);
    printf("Neighbours: %i \n", hm->pn*MAXN);
    printf("Grid cells: %i \n", hGrid.nX * hGrid.nY * hGrid.nZ);
}


int copyHostToDevice(struct host_model *hm, struct device_model *dm) {

    dm->pn = hm->pn;
    dm->Material = hm->Material;
    dm->Mass = hm->Mass;
    dm->Smooth = hm->Smooth;
    dm->PosX = hm->PosX;
    dm->PosY = hm->PosY;
    dm->PosZ = hm->PosZ;
    dm->VelX = hm->VelX;
    dm->VelY = hm->VelY;
    dm->VelZ = hm->VelZ;
    dm->Density = hm->Density;
    dm->Energy = hm->Energy;
    dm->Pressure = hm->Pressure;
    dm->Sound = hm->Sound;
    dm->VelDotX = hm->VelDotX;
    dm->VelDotY = hm->VelDotY;
    dm->VelDotZ = hm->VelDotZ;
    dm->DensityDot = hm->DensityDot;
    dm->EnergyDot = hm->EnergyDot;

    dm->PosX0 = hm->PosX0;
    dm->PosY0 = hm->PosY0;
    dm->PosZ0 = hm->PosZ0;
    dm->VelX0 = hm->VelX0;
    dm->VelY0 = hm->VelY0;
    dm->VelZ0 = hm->VelZ0;
    dm->Density0 = hm->Density0;
    dm->Energy0 = hm->Energy0;

    dm->List = hm->List;
    dm->Hash = hm->Hash;
    dm->Index = hm->Index;

    dm->SetStart = hm->SetStart;
    dm->SetStop = hm->SetStop;

    dm->IntDummy = hm->IntDummy;
    dm->FloatDummy = hm->FloatDummy;

    dGrid.oX = hGrid.oX;
    dGrid.oY = hGrid.oY;
    dGrid.oZ = hGrid.oZ;
    dGrid.nX = hGrid.nX;
    dGrid.nY = hGrid.nY;
    dGrid.nZ = hGrid.nZ;
    dGrid.size = hGrid.size;

    return 0;
}


int copyDeviceToHost(struct device_model *dm, struct host_model *hm) {

    hm->pn = dm->pn;

    thrust::copy(dm->Material.begin(), dm->Material.end(), hm->Material.begin());
    thrust::copy(dm->Mass.begin(), dm->Mass.end(), hm->Mass.begin());
    thrust::copy(dm->Smooth.begin(), dm->Smooth.end(), hm->Smooth.begin());
    thrust::copy(dm->PosX.begin(), dm->PosX.end(), hm->PosX.begin());
    thrust::copy(dm->PosY.begin(), dm->PosY.end(), hm->PosY.begin());
    thrust::copy(dm->PosZ.begin(), dm->PosZ.end(), hm->PosZ.begin());
    thrust::copy(dm->VelX.begin(), dm->VelX.end(), hm->VelX.begin());
    thrust::copy(dm->VelY.begin(), dm->VelY.end(), hm->VelY.begin());
    thrust::copy(dm->VelZ.begin(), dm->VelZ.end(), hm->VelZ.begin());
    thrust::copy(dm->Density.begin(), dm->Density.end(), hm->Density.begin());
    thrust::copy(dm->Energy.begin(), dm->Energy.end(), hm->Energy.begin());
    thrust::copy(dm->Pressure.begin(), dm->Pressure.end(), hm->Pressure.begin());
    thrust::copy(dm->Sound.begin(), dm->Sound.end(), hm->Sound.begin());
    thrust::copy(dm->VelDotX.begin(), dm->VelDotX.end(), hm->VelDotX.begin());
    thrust::copy(dm->VelDotY.begin(), dm->VelDotY.end(), hm->VelDotY.begin());
    thrust::copy(dm->VelDotZ.begin(), dm->VelDotZ.end(), hm->VelDotZ.begin());
    thrust::copy(dm->DensityDot.begin(), dm->DensityDot.end(), hm->DensityDot.begin());
    thrust::copy(dm->EnergyDot.begin(), dm->EnergyDot.end(), hm->EnergyDot.begin());

    thrust::copy(dm->List.begin(), dm->List.end(), hm->List.begin());
    thrust::copy(dm->Hash.begin(), dm->Hash.end(), hm->Hash.begin());
    thrust::copy(dm->Index.begin(), dm->Index.end(), hm->Index.begin());

    thrust::copy(dm->SetStart.begin(), dm->SetStart.end(), hm->SetStart.begin());
    thrust::copy(dm->SetStop.begin(), dm->SetStop.end(), hm->SetStop.begin());

    thrust::copy(dm->IntDummy.begin(), dm->IntDummy.end(), hm->IntDummy.begin());
    thrust::copy(dm->FloatDummy.begin(), dm->FloatDummy.end(), hm->FloatDummy.begin());

    hGrid.oX = dGrid.oX;
    hGrid.oY = dGrid.oY;
    hGrid.oZ = dGrid.oZ;
    hGrid.nX = dGrid.nX;
    hGrid.nY = dGrid.nY;
    hGrid.nZ = dGrid.nZ;
    hGrid.size = dGrid.size;

    return 0;
}


int copyDeviceToPointer(struct device_model *dm, struct pointer_model *pm) {

    pm->pn = dm->pn;

    pm->Material = thrust::raw_pointer_cast(&dm->Material[0]);
    pm->Mass = thrust::raw_pointer_cast(&dm->Mass[0]);
    pm->Smooth = thrust::raw_pointer_cast(&dm->Smooth[0]);
    pm->PosX = thrust::raw_pointer_cast(&dm->PosX[0]);
    pm->PosY = thrust::raw_pointer_cast(&dm->PosY[0]);
    pm->PosZ = thrust::raw_pointer_cast(&dm->PosZ[0]);
    pm->VelX = thrust::raw_pointer_cast(&dm->VelX[0]);
    pm->VelY = thrust::raw_pointer_cast(&dm->VelY[0]);
    pm->VelZ = thrust::raw_pointer_cast(&dm->VelZ[0]);
    pm->Density = thrust::raw_pointer_cast(&dm->Density[0]);
    pm->Energy = thrust::raw_pointer_cast(&dm->Energy[0]);
    pm->Pressure = thrust::raw_pointer_cast(&dm->Pressure[0]);
    pm->Sound = thrust::raw_pointer_cast(&dm->Sound[0]);
    pm->VelDotX = thrust::raw_pointer_cast(&dm->VelDotX[0]);
    pm->VelDotY = thrust::raw_pointer_cast(&dm->VelDotY[0]);
    pm->VelDotZ = thrust::raw_pointer_cast(&dm->VelDotZ[0]);
    pm->DensityDot = thrust::raw_pointer_cast(&dm->DensityDot[0]);
    pm->EnergyDot = thrust::raw_pointer_cast(&dm->EnergyDot[0]);
    pm->PosX0 = thrust::raw_pointer_cast(&dm->PosX0[0]);
    pm->PosY0 = thrust::raw_pointer_cast(&dm->PosY0[0]);
    pm->PosZ0 = thrust::raw_pointer_cast(&dm->PosZ0[0]);
    pm->VelX0 = thrust::raw_pointer_cast(&dm->VelX0[0]);
    pm->VelY0 = thrust::raw_pointer_cast(&dm->VelY0[0]);
    pm->VelZ0 = thrust::raw_pointer_cast(&dm->VelZ0[0]);
    pm->Density0 = thrust::raw_pointer_cast(&dm->Density0[0]);
    pm->Energy0 = thrust::raw_pointer_cast(&dm->Energy0[0]);
    pm->Hash = thrust::raw_pointer_cast(&dm->Hash[0]);
    pm->Index = thrust::raw_pointer_cast(&dm->Index[0]);
    pm->SetStart = thrust::raw_pointer_cast(&dm->SetStart[0]);
    pm->SetStop = thrust::raw_pointer_cast(&dm->SetStop[0]);
    pm->List = thrust::raw_pointer_cast(&dm->List[0]);
    pm->IntDummy = thrust::raw_pointer_cast(&dm->IntDummy[0]);
    pm->FloatDummy = thrust::raw_pointer_cast(&dm->FloatDummy[0]);

    return 0;
}

int resizeHost(struct host_model *hm) {

    hm->Material.resize(hm->pn);
    hm->Mass.resize(hm->pn);
    hm->Smooth.resize(hm->pn);
    hm->PosX.resize(hm->pn);
    hm->PosY.resize(hm->pn);
    hm->PosZ.resize(hm->pn);
    hm->VelX.resize(hm->pn);
    hm->VelY.resize(hm->pn);
    hm->VelZ.resize(hm->pn);
    hm->Density.resize(hm->pn);
    hm->Energy.resize(hm->pn);
    hm->Pressure.resize(hm->pn);
    hm->Sound.resize(hm->pn);
    hm->VelDotX.resize(hm->pn);
    hm->VelDotY.resize(hm->pn);
    hm->VelDotZ.resize(hm->pn);
    hm->DensityDot.resize(hm->pn);
    hm->EnergyDot.resize(hm->pn);
    hm->PosX0.resize(hm->pn);
    hm->PosY0.resize(hm->pn);
    hm->PosZ0.resize(hm->pn);
    hm->VelX0.resize(hm->pn);
    hm->VelY0.resize(hm->pn);
    hm->VelZ0.resize(hm->pn);
    hm->Density0.resize(hm->pn);
    hm->Energy0.resize(hm->pn);
    hm->Hash.resize(hm->pn);
    hm->Index.resize(hm->pn);
    hm->List.resize(hm->pn * MAXN);
    hm->IntDummy.resize(hm->pn);
    hm->FloatDummy.resize(hm->pn);

    return 0;
}


int resizeDevice(struct device_model *dm) {

    dm->Material.resize(dm->pn);
    dm->Mass.resize(dm->pn);
    dm->Smooth.resize(dm->pn);
    dm->PosX.resize(dm->pn);
    dm->PosY.resize(dm->pn);
    dm->PosZ.resize(dm->pn);
    dm->VelX.resize(dm->pn);
    dm->VelY.resize(dm->pn);
    dm->VelZ.resize(dm->pn);
    dm->Density.resize(dm->pn);
    dm->Energy.resize(dm->pn);
    dm->Pressure.resize(dm->pn);
    dm->Sound.resize(dm->pn);
    dm->VelDotX.resize(dm->pn);
    dm->VelDotY.resize(dm->pn);
    dm->VelDotZ.resize(dm->pn);
    dm->DensityDot.resize(dm->pn);
    dm->EnergyDot.resize(dm->pn);
    dm->PosX0.resize(dm->pn);
    dm->PosY0.resize(dm->pn);
    dm->PosZ0.resize(dm->pn);
    dm->VelX0.resize(dm->pn);
    dm->VelY0.resize(dm->pn);
    dm->VelZ0.resize(dm->pn);
    dm->Density0.resize(dm->pn);
    dm->Energy0.resize(dm->pn);
    dm->Hash.resize(dm->pn);
    dm->Index.resize(dm->pn);
    dm->List.resize(dm->pn * MAXN);
    dm->IntDummy.resize(dm->pn);
    dm->FloatDummy.resize(dm->pn);

    return 0;
}

int printData(struct host_model *hm) {
    /**
     * \brief Particle data file output
     *
     * Saves particle data on a disk file
     *
     * \date Oct 21, 2010
     * \author Luca Massidda
     */

    FILE *stream;
    int i;

    // Stream file position
    stream = fopen("new_pos.txt", "w");
    for (i = 0; i < hm->pn; i++)
        fprintf(stream, "%+14.8e %+14.8e %+14.8e \n", hm->PosX[i], hm->PosY[i], hm->PosZ[i]);
    fclose(stream);

    // Stream file velocity
    stream = fopen("new_vel.txt", "w");
    for (i = 0; i < hm->pn; i++)
        fprintf(stream, "%+14.8e %+14.8e %+14.8e \n", hm->VelX[i], hm->VelY[i], hm->VelZ[i]);
    fclose(stream);

    // Stream file info
    stream = fopen("new_info.txt", "w");
    for (i = 0; i < hm->pn; i++)
        fprintf(stream, "%i %+14.8e %+14.8e \n", hm->Material[i], hm->Mass[i], hm->Smooth[i]);
    fclose(stream);

    // Stream file field
    stream = fopen("new_field.txt", "w");
    for (i = 0; i < hm->pn; i++)
        fprintf(stream, "%+14.8e %+14.8e %+14.8e \n", hm->Density[i], hm->Pressure[i], hm->Energy[i]);
    fclose(stream);

    // Stream file add1
    stream = fopen("new_debug.txt", "w");
    for (i = 0; i < hm->pn; i++)
        fprintf(stream, "%+14.8e %+14.8e %+14.8e %+14.8e %+14.8e \n", hm->DensityDot[i],
                hm->VelDotX[i], hm->VelDotY[i], hm->VelDotZ[i], hm->EnergyDot[i]);
    fclose(stream);

    /*
    for (i = 0; i < hm->pn; i++) {
    	printf("%d - ", i);
    	for (int j = 0; j < MAXN; j++)
    		printf("%d ", hm->List[i * MAXN +j]);
    	printf("\n");
    }
    */

    return 0;
}


int outputVTK(struct host_model *hm, int ss) {
    /**
     * \brief Output Data file
     *
     * Saves vtk data file
     *
     * \date Oct 21, 2010
     * \author Luca Massidda
     */

    FILE *stream;
    char filename[80];
    int i;

    // Stream position file
    sprintf(filename, "out%05d.vtk", ss);
    stream = fopen(filename, "w");

    fprintf(stream, "# vtk DataFile Version 2.0\n");
    fprintf(stream, "Unstructured Grid Example\n");
    fprintf(stream, "ASCII\n");
    fprintf(stream, "DATASET UNSTRUCTURED_GRID\n");

    fprintf(stream, "POINTS %i float\n", hm->pn);
    for (i = 0; i < hm->pn; i++)
        fprintf(stream, "%+e %+e %+e \n", hm->PosX[i], hm->PosY[i], hm->PosZ[i]);

    fprintf(stream, "CELLS %i %i \n", hm->pn, 2*hm->pn);
    for (i = 0; i < hm->pn; i++)
        fprintf(stream, "%i %i \n", 1, i);

    fprintf(stream, "CELL_TYPES %i \n", hm->pn);
    for (i = 0; i < hm->pn; i++)
        fprintf(stream, "%i \n", 1);

    fprintf(stream, "POINT_DATA %i \n", hm->pn);

    fprintf(stream, "SCALARS density float 1 \n", hm->pn);
    fprintf(stream, "LOOKUP_TABLE default\n");
    for (i = 0; i < hm->pn; i++)
        fprintf(stream, "%+e \n", hm->Density[i]);

    fprintf(stream, "SCALARS pressure float 1 \n", hm->pn);
    fprintf(stream, "LOOKUP_TABLE default\n");
    for (i = 0; i < hm->pn; i++)
        fprintf(stream, "%+e \n", hm->Pressure[i]);

    fprintf(stream, "SCALARS energy float 1 \n", hm->pn);
    fprintf(stream, "LOOKUP_TABLE default\n");
    for (i = 0; i < hm->pn; i++)
        fprintf(stream, "%+e \n", hm->Energy[i]);

    fprintf(stream, "VECTORS velocity float\n");
    for (i = 0; i < hm->pn; i++)
        fprintf(stream, "%+e %+e %+e \n", hm->VelX[i], hm->VelY[i], hm->VelZ[i]);

    fclose(stream);
    /*
    for (i = 0; i < hm->pn; i++) printf("%d %d \n", i, hm->Hash[i]);
    printf("\n\n\n");
    for (i = 0; i < hm->SetStart.size(); i++) printf("%d %d %d \n", i, hm->SetStart[i], hm->SetStop[i]);

    for (i = 0; i < hm->pn; i++) {
    	printf("%d  -  ", i);
    	for (j = 0; j < MAXN; j++) printf("%d ", hm->List[i*MAXN +j]);
    	printf("\n");
    }
    */
    return 0;
}


__global__ void updateHashDevice(const int pn, const struct grid Grid,
                                 const float* PosX, const float* PosY, const float* PosZ,
                                 int* Hash, int* Index) {

    /**
     * \brief Update particles
     *
     * \date Jan 6, 2010
     * \author Luca Massidda
     */

    int ip, ix, iy, iz, ic;

    ip = threadIdx.x + blockDim.x * blockIdx.x;

    if (ip < pn) {
        ix = (int) truncf((PosX[ip] - Grid.oX) / Grid.size);
        iy = (int) truncf((PosY[ip] - Grid.oY) / Grid.size);
        iz = (int) truncf((PosZ[ip] - Grid.oZ) / Grid.size);
        ic = ix + iy * Grid.nX + iz * Grid.nX * Grid.nY;

        Hash[ip] = ic;
        Index[ip] = ip;
    }
}

__global__ void checkBoundariesDevice(const int pn, const struct simulation sim,
                                      const float* PosX, const float* PosY, const float* PosZ,
                                      int* Hash) {

    /**
     * \brief Update particles
     *
     * \date Jan 6, 2010
     * \author Luca Massidda
     */

    int ip;

    ip = threadIdx.x + blockDim.x * blockIdx.x;

    if (ip < pn) {
        if ((PosX[ip] < sim.minX) || (PosX[ip] > sim.maxX) ||
            (PosY[ip] < sim.minY) || (PosY[ip] > sim.maxY) ||
            (PosZ[ip] < sim.minZ) || (PosZ[ip] > sim.maxZ)) {
            Hash[ip] = -1;
        }
    }
}

__global__ void checkOutletDevice(const int pn, const struct disp Disp,
                                      const float* PosX, const float* PosY, const float* PosZ,
                                      int* Hash) {

    /**
     * \brief Update particles
     *
     * \date Jan 6, 2010
     * \author Luca Massidda
     */

    float d;
    int ip;

    ip = threadIdx.x + blockDim.x * blockIdx.x;

    if (ip < pn) {
        d = 0.0f;
        d += (PosX[ip] - Disp.oX) * Disp.nX;
        d += (PosY[ip] - Disp.oY) * Disp.nY;
        d += (PosZ[ip] - Disp.oZ) * Disp.nZ;
        if (d > 0.0f) {
            Hash[ip] = -1;
        }
    }
}


__global__ void updateSetsDevice(const int pn, int *SetStart, int *SetStop,
                                 const int* Hash) {

    /**
     * \brief Update particles
     *
     * \date Jan 6, 2010
     * \author Luca Massidda
     */

    __shared__ int prevHash[THREADS];
    __shared__ int nextHash[THREADS];

    int ip;
    int hash;

    ip = threadIdx.x + blockDim.x * blockIdx.x;
    if (ip >= pn) return;

    hash = Hash[ip];

    if (threadIdx.x < THREADS -1) prevHash[threadIdx.x +1] = hash;
    if (threadIdx.x > 0) nextHash[threadIdx.x -1] = hash;

    if (threadIdx.x == 0) {
        if (ip == 0) prevHash[threadIdx.x] = -1;
        else prevHash[threadIdx.x] = Hash[ip -1];
    }

    if (threadIdx.x == THREADS -1) {
        if (ip == pn -1) nextHash[threadIdx.x] = -1;
        else nextHash[threadIdx.x] = Hash[ip +1];
    }

    __syncthreads();

    if (hash != prevHash[threadIdx.x]) SetStart[hash] = ip;

    if (hash != nextHash[threadIdx.x]) SetStop[hash] = ip +1;

}


__global__ void updateListDevice(const int pn, int *List,
                                 const int* SetStart, const int* SetStop,
                                 const struct grid Grid, const float* Smooth,
                                 const float* PosX, const float* PosY, const float* PosZ) {

    int ip, ic, ix, iy, iz, i, j, k, jp, jc, np;
    float dx, dy, dz, dr;

    // Particles list is filled
    ip = threadIdx.x + blockDim.x * blockIdx.x;
    if (ip >= pn) return;

    ix = (int) ((PosX[ip] - Grid.oX) / Grid.size);
    iy = (int) ((PosY[ip] - Grid.oY) / Grid.size);
    iz = (int) ((PosZ[ip] - Grid.oZ) / Grid.size);
    ic = ix + iy * Grid.nX + iz * Grid.nX * Grid.nY;
    np = 0;

    for (k = -1; k <= 1; k++) {
        for (j = -1; j <= 1; j++) {
            for (i = -1; i <= 1; i++) {
                jc = ic + i + j * Grid.nX + k * Grid.nX * Grid.nY;

                for (jp = SetStart[jc]; jp < SetStop[jc]; jp++) {
                    dx = PosX[ip] - PosX[jp];
                    dy = PosY[ip] - PosY[jp];
                    dz = PosZ[ip] - PosZ[jp];
                    dr = sqrtf(dx * dx + dy * dy + dz * dz);

                    if ((dr < 2.0f * Smooth[ip]) && (np < MAXN)) {
                        List[ip * MAXN + np] = jp;
                        np++;
                    }
                }
            }
        }
    }

    while (np < MAXN) {
        List[ip * MAXN + np] = ip;
        np++;
    }

}

struct is_outside
{
    template <typename Tuple>
    __host__ __device__
    bool operator()(const Tuple& tuple) const
    {
        // unpack the tuple into x and y coordinates
//        const T x = thrust::get<0>(tuple);
//        const T y = thrust::get<1>(tuple);
        int hash = thrust::get<0>(tuple);

        if (hash < 0)
            return true;
        else
            return false;
    }
};


int neighbourListDevice(struct device_model *dm, struct pointer_model *pm) {

    int blocks, threads;
    int i;

    blocks = (pm->pn + THREADS - 1) / THREADS;
    threads = THREADS;

    updateHashDevice <<< blocks, threads >>>
    (pm->pn, dGrid, pm->PosX, pm->PosY, pm->PosZ, pm->Hash, pm->Index);
	
    checkBoundariesDevice <<< blocks, threads >>>
    (pm->pn, dRun, pm->PosX, pm->PosY, pm->PosZ, pm->Hash);
	
	for (i = 0; i < 10; i++) {
		checkOutletDevice <<< blocks, threads >>>
		(pm->pn, dDisp[i], pm->PosX, pm->PosY, pm->PosZ, pm->Hash);
	}
	
    thrust::copy(dm->Hash.begin(), dm->Hash.end(), dm->IntDummy.begin());
    
    dm->pn = thrust::remove_if(thrust::make_zip_iterator(thrust::make_tuple(
        dm->Hash.begin(), dm->Material.begin(), dm->Mass.begin(), dm->Smooth.begin(),
        dm->PosX.begin(), dm->PosY.begin(), dm->PosZ.begin())), 
                               thrust::make_zip_iterator(thrust::make_tuple(
        dm->Hash.end(), dm->Material.end(), dm->Mass.end(), dm->Smooth.end(),
        dm->PosX.end(), dm->PosY.end(), dm->PosZ.end()), 
                               is_outside())
                             - thrust::make_zip_iterator(thrust::make_tuple(
        dm->Hash.begin(), dm->Material.begin(), dm->Mass.begin(), dm->Smooth.begin(),
        dm->PosX.begin(), dm->PosY.begin(), dm->PosZ.begin()));
	
    dm->pn = thrust::remove_if(thrust::make_zip_iterator(thrust::make_tuple(
        dm->IntDummy.begin(), dm->VelX.begin(), dm->VelY.begin(), dm->VelZ.begin(), 
        dm->Density.begin(), dm->Energy.begin())), 
                               thrust::make_zip_iterator(thrust::make_tuple(
        dm->IntDummy.end(), dm->VelX.end(), dm->VelY.end(), dm->VelZ.end(), 
        dm->Density.end(), dm->Energy.end())), 
                               is_outside())
                             - thrust::make_zip_iterator(thrust::make_tuple(
        dm->IntDummy.begin(), dm->VelX.begin(), dm->VelY.begin(), dm->VelZ.begin(), 
        dm->Density.begin(), dm->Energy.begin()));
	
	resizeDevice(dm);
	
    try {
        thrust::sort_by_key(dm->Hash.begin(), dm->Hash.end(), dm->Index.begin());

        thrust::copy(dm->Material.begin(), dm->Material.end(), dm->IntDummy.begin());
        thrust::gather(dm->Index.begin(), dm->Index.end(), dm->IntDummy.begin(), dm->Material.begin());

        thrust::copy(dm->Mass.begin(), dm->Mass.end(), dm->FloatDummy.begin());
        thrust::gather(dm->Index.begin(), dm->Index.end(), dm->FloatDummy.begin(), dm->Mass.begin());

        thrust::copy(dm->Smooth.begin(), dm->Smooth.end(), dm->FloatDummy.begin());
        thrust::gather(dm->Index.begin(), dm->Index.end(), dm->FloatDummy.begin(), dm->Smooth.begin());

        thrust::copy(dm->PosX.begin(), dm->PosX.end(), dm->FloatDummy.begin());
        thrust::gather(dm->Index.begin(), dm->Index.end(), dm->FloatDummy.begin(), dm->PosX.begin());

        thrust::copy(dm->PosY.begin(), dm->PosY.end(), dm->FloatDummy.begin());
        thrust::gather(dm->Index.begin(), dm->Index.end(), dm->FloatDummy.begin(), dm->PosY.begin());

        thrust::copy(dm->PosZ.begin(), dm->PosZ.end(), dm->FloatDummy.begin());
        thrust::gather(dm->Index.begin(), dm->Index.end(), dm->FloatDummy.begin(), dm->PosZ.begin());

        thrust::copy(dm->VelX.begin(), dm->VelX.end(), dm->FloatDummy.begin());
        thrust::gather(dm->Index.begin(), dm->Index.end(), dm->FloatDummy.begin(), dm->VelX.begin());

        thrust::copy(dm->VelY.begin(), dm->VelY.end(), dm->FloatDummy.begin());
        thrust::gather(dm->Index.begin(), dm->Index.end(), dm->FloatDummy.begin(), dm->VelY.begin());

        thrust::copy(dm->VelZ.begin(), dm->VelZ.end(), dm->FloatDummy.begin());
        thrust::gather(dm->Index.begin(), dm->Index.end(), dm->FloatDummy.begin(), dm->VelZ.begin());

        thrust::copy(dm->Density.begin(), dm->Density.end(), dm->FloatDummy.begin());
        thrust::gather(dm->Index.begin(), dm->Index.end(), dm->FloatDummy.begin(), dm->Density.begin());

        thrust::copy(dm->Energy.begin(), dm->Energy.end(), dm->FloatDummy.begin());
        thrust::gather(dm->Index.begin(), dm->Index.end(), dm->FloatDummy.begin(), dm->Energy.begin());

    } catch(std::bad_alloc &e) {
        printf("Ran out of memory while sorting\n");
        exit(-1);
    } catch(thrust::system_error &e) {
        printf("Error accessing vector element: %s \n", e.what());
        exit(-1);
    }

    thrust::fill(dm->SetStart.begin(), dm->SetStart.end(), 0);
    thrust::fill(dm->SetStop.begin(), dm->SetStop.end(), 0);

    updateSetsDevice <<< blocks, threads >>>
    (pm->pn, pm->SetStart, pm->SetStop, pm->Hash);

    updateListDevice <<< blocks, threads >>>
    (pm->pn, pm->List, pm->SetStart, pm->SetStop, dGrid, pm->Smooth,
     pm->PosX, pm->PosY, pm->PosZ);

    return 0;
}

int backupDataDevice(struct device_model *dm) {

    thrust::copy(dm->PosX.begin(), dm->PosX.end(), dm->PosX0.begin());
    thrust::copy(dm->PosY.begin(), dm->PosY.end(), dm->PosY0.begin());
    thrust::copy(dm->PosZ.begin(), dm->PosZ.end(), dm->PosZ0.begin());
    thrust::copy(dm->VelX.begin(), dm->VelX.end(), dm->VelX0.begin());
    thrust::copy(dm->VelY.begin(), dm->VelY.end(), dm->VelY0.begin());
    thrust::copy(dm->VelZ.begin(), dm->VelZ.end(), dm->VelZ0.begin());
    thrust::copy(dm->Density.begin(), dm->Density.end(), dm->Density0.begin());
    thrust::copy(dm->Energy.begin(), dm->Energy.end(), dm->Energy0.begin());

    return 0;
}


__global__ void updateLoadsDevice(const int pn,
                                  const int* Material,
                                  float* PosX, float* PosY, float* PosZ,
                                  float* VelX, float* VelY, float* VelZ,
                                  float* VelDotX, float* VelDotY, float* VelDotZ,
                                  float* EnergyDot) {

    int ip, i;

    ip = threadIdx.x + blockDim.x * blockIdx.x;

    if ((ip < pn) && (Material[ip] > 0)) {
        for (i = 0; i < 10; i++) {
            if ((PosX[ip] > dLoad[i].minX) &&
                    (PosX[ip] < dLoad[i].maxX) &&
                    (PosZ[ip] < dLoad[i].maxZ) &&
                    (PosY[ip] > dLoad[i].minY) &&
                    (PosY[ip] < dLoad[i].maxY) &&
                    (PosZ[ip] > dLoad[i].minZ) &&
                    (PosZ[ip] < dLoad[i].maxZ)) {
                VelDotX[ip] += dLoad[i].gx;
                VelDotY[ip] += dLoad[i].gy;
                VelDotZ[ip] += dLoad[i].gz;
                EnergyDot[ip] += dLoad[i].w;
            }
        }

    }

}

__global__ void updateDispDevice(const int pn,
                                 int* Material,
                                 float* PosX, float* PosY, float* PosZ) {

    int ip, i;
    float d;

    ip = threadIdx.x + blockDim.x * blockIdx.x;

    if ((ip < pn) && (Material[ip] > 0)) {

        for (i = 0; i < 10; i++) {
            d = 0.0f;
            d += (PosX[ip] - dDisp[i].oX) * dDisp[i].nX;
            d += (PosY[ip] - dDisp[i].oY) * dDisp[i].nY;
            d += (PosZ[ip] - dDisp[i].oZ) * dDisp[i].nZ;
            if (d > 0.0f) {
                PosX[ip] = dDisp[i].dX;
                PosY[ip] = dDisp[i].dY;
                PosZ[ip] = dDisp[i].dZ;
                Material[ip] = 0;
            }
        }

        if (PosX[ip] < dRun.minX)
            PosX[ip] += dRun.maxX - dRun.minX;
        if (PosX[ip] > dRun.maxX)
            PosX[ip] -= dRun.maxX - dRun.minX;

        if (PosY[ip] < dRun.minY)
            PosY[ip] += dRun.maxY - dRun.minY;
        if (PosY[ip] > dRun.maxY)
            PosY[ip] -= dRun.maxY - dRun.minY;

        if (PosZ[ip] < dRun.minZ)
            PosZ[ip] += dRun.maxZ - dRun.minZ;
        if (PosZ[ip] > dRun.maxZ)
            PosZ[ip] -= dRun.maxZ - dRun.minZ;

    }

}

__device__ float pressureGas(int mat ,float rho, float u) {
    /**
     * \brief Ideal gas Equation Of State
     *
     * p = (k -1) rho u
     * c = (k(k -1) u)^0.5
     *
     * k = dMatProp[mat][1]
     * pshift = dMatProp[mat][2]
     *
     * \date Jun 10, 2010
     * \author Luca Massidda
     */

    float p;

    p = (dMatProp[mat][1] - 1.0f) * rho * u;
    p += dMatProp[mat][2];

    return p;
}



__device__ float pressurePoly(int mat , float rho, float u) {
    /**
     * \brief Mie-Gruneisen polynomial Equation Of State
     *
     * p = a1 mu + a2 mu^2 + a3 mu^3 + (b0 + b1 mu) rho0 u  in compression
     * p = t1 mu + t2 mu^2 + b0 rho0 u                      in tension
     *
     * rho0 = dMatProp[mat][0];
     * a1 = dMatProp[mat][1];
     * a2 = dMatProp[mat][2];
     * a3 = dMatProp[mat][3];
     * b0 = dMatProp[mat][4];
     * b1 = dMatProp[mat][5];
     * t1 = dMatProp[mat][6];
     * t2 = dMatProp[mat][7];
     * pmin = dMatProp[mat][8];
     *
     * \date Jun 10, 2010
     * \author Luca Massidda
     */

    float mu;
    float p;

    mu = (rho - dMatProp[mat][0]) / dMatProp[mat][0];

    if (mu < 0)
        p = (dMatProp[mat][6] * mu + dMatProp[mat][7] * mu*mu)
            + (dMatProp[mat][4] * dMatProp[mat][0] * u);
    else
        p = (dMatProp[mat][1] * mu + dMatProp[mat][2] * mu*mu
             + dMatProp[mat][3] * mu*mu*mu)
            + ((dMatProp[mat][4] + dMatProp[mat][5] * mu)
               * dMatProp[mat][0] * u);

    if (p < dMatProp[mat][8]) p = dMatProp[mat][8];

    return p;
}

__device__ float pressureShock(int mat, float rho, float u) {
    /**
     * \brief Mie-Gruneisen Shock Hugoniot Equation Of State
     *
     * mu = rho / rho0 -1
     * g = g * rho0 / rho
     * ph = (rho0 c0^2 mu (1 + mu)) / (1 - (s0 - 1) * mu)^2
     * uh = 1/2 ph/rho0 * (mu / (1 + mu))
     * p = ph + g * rho * (u - uh)
     *
     * rho0 = dMatProp[mat][0];
     * c0 = dMatProp[mat][1];
     * g0 = dMatProp[mat][2];
     * s0 = dMatProp[mat][3];
     * pmin = dMatProp[mat][4];
     *
     * \date Jun 10, 2010
     * \author Luca Massidda
     */

    float mu;
    float p, ph;

    mu = (rho - dMatProp[mat][0]) / dMatProp[mat][0];

    ph = (dMatProp[mat][0] * powf(dMatProp[mat][1], 2) * mu*(1.0f +mu))
         / powf((1.0f - (dMatProp[mat][3] -1.0f) * mu), 2);

    p = ph + dMatProp[mat][2] * dMatProp[mat][0]
        * (u - (0.5f * ph / dMatProp[mat][0] * (mu / (1.0f + mu))));

    if (p < dMatProp[mat][4]) p = dMatProp[mat][4];

    return p;
}


__device__ float pressureTait(int mat, float rho, float u) {
    /**
     * \brief Tait Equation Of State
     *
     * p = rho0 * c0 * c0 / 7.0 * (powf((rho / rho0), 7) - 1.0);
     * c = c0;
     *
     * rho0 = dMatProp[mat][0];
     * c0 = dMatProp[mat][1];
     * pmin = dMatProp[mat][2];
     *
     * \date Jun 10, 2010
     * \author Luca Massidda
     */

    float p;

    p = dMatProp[mat][0] * powf(dMatProp[mat][1], 2) / 7.0f
        * (powf((rho / dMatProp[mat][0]), 7) - 1.0f);

    if (p < dMatProp[mat][2]) p = dMatProp[mat][2];

    return p;
}


__device__ float soundGas(int mat ,float rho, float u) {
    /**
     * \brief Ideal gas Equation Of State
     *
     * p = (k -1) rho u
     * c = (k(k -1) u)^0.5
     *
     * k = dMatProp[mat][1]
     * pshift = dMatProp[mat][2]
     *
     * \date Jun 10, 2010
     * \author Luca Massidda
     */

    float c;

    c = sqrtf(dMatProp[mat][1] * (dMatProp[mat][1] - 1.0f) * u);

    return c;
}



__device__ float soundPoly(int mat , float rho, float u) {
    /**
     * \brief Mie-Gruneisen polynomial Equation Of State
     *
     * p = a1 mu + a2 mu^2 + a3 mu^3 + (b0 + b1 mu) rho0 u  in compression
     * p = t1 mu + t2 mu^2 + b0 rho0 u                      in tension
     *
     * rho0 = dMatProp[mat][0];
     * a1 = dMatProp[mat][1];
     * a2 = dMatProp[mat][2];
     * a3 = dMatProp[mat][3];
     * b0 = dMatProp[mat][4];
     * b1 = dMatProp[mat][5];
     * t1 = dMatProp[mat][6];
     * t2 = dMatProp[mat][7];
     * pmin = dMatProp[mat][8];
     *
     * \date Jun 10, 2010
     * \author Luca Massidda
     */

    float c;

    c = sqrtf(dMatProp[mat][1] / rho);

    return c;
}

__device__ float soundShock(int mat, float rho, float u) {
    /**
     * \brief Mie-Gruneisen Shock Hugoniot Equation Of State
     *
     * mu = rho / rho0 -1
     * g = g * rho0 / rho
     * ph = (rho0 c0^2 mu (1 + mu)) / (1 - (s0 - 1) * mu)^2
     * uh = 1/2 ph/rho0 * (mu / (1 + mu))
     * p = ph + g * rho * (u - uh)
     *
     * rho0 = dMatProp[mat][0];
     * c0 = dMatProp[mat][1];
     * g0 = dMatProp[mat][2];
     * s0 = dMatProp[mat][3];
     * pmin = dMatProp[mat][4];
     *
     * \date Jun 10, 2010
     * \author Luca Massidda
     */

    float c;

    c = dMatProp[mat][1];

    return c;
}


__device__ float soundTait(int mat, float rho, float u) {
    /**
     * \brief Tait Equation Of State
     *
     * p = rho0 * c0 * c0 / 7.0 * (powf((rho / rho0), 7) - 1.0);
     * c = c0;
     *
     * rho0 = dMatProp[mat][0];
     * c0 = dMatProp[mat][1];
     * pmin = dMatProp[mat][2];
     *
     * \date Jun 10, 2010
     * \author Luca Massidda
     */

    float c;

    c = dMatProp[mat][1];

    return c;
}


__global__ void updateParticlesDevice(const int pn, const float alpha,
                                      const int* Material,
                                      const float* VelDotX, const float* VelDotY, const float* VelDotZ,
                                      const float* DensityDot, const float* EnergyDot,
                                      const float* PosX0, const float* PosY0, const float* PosZ0,
                                      const float* VelX0, const float* VelY0, const float* VelZ0,
                                      const float* Density0, const float* Energy0,
                                      float* PosX, float* PosY, float* PosZ,
                                      float* VelX, float* VelY, float* VelZ,
                                      float* Density, float* Energy, float* Pressure, float* Sound) {

    /**
     * \brief Update particles
     *
     * \date Jan 6, 2010
     * \author Luca Massidda
     */

    int ip;
    int iMaterial;
    float iDensity, iEnergy;

    ip = threadIdx.x + blockDim.x * blockIdx.x;

    if ((ip < pn) && (Material[ip] != 0)) {
        PosX[ip] = PosX0[ip] + alpha * (PosX[ip] + dRun.dt * VelX[ip] - PosX0[ip]);
        PosY[ip] = PosY0[ip] + alpha * (PosY[ip] + dRun.dt * VelY[ip] - PosY0[ip]);
        PosZ[ip] = PosZ0[ip] + alpha * (PosZ[ip] + dRun.dt * VelZ[ip] - PosZ0[ip]);

        VelX[ip] = VelX0[ip] + alpha * (VelX[ip] + dRun.dt * VelDotX[ip] - VelX0[ip]);
        VelY[ip] = VelY0[ip] + alpha * (VelY[ip] + dRun.dt * VelDotY[ip] - VelY0[ip]);
        VelZ[ip] = VelZ0[ip] + alpha * (VelZ[ip] + dRun.dt * VelDotZ[ip] - VelZ0[ip]);
        VelZ[ip] = 0.0f;

        Density[ip] = Density0[ip] + alpha * (Density[ip] + dRun.dt * DensityDot[ip] - Density0[ip]);

        Energy[ip] = Energy0[ip] + alpha * (Energy[ip] + dRun.dt * EnergyDot[ip] - Energy0[ip]);

        iMaterial = Material[ip];

        if (iMaterial <= 0) {
            VelX[ip] = VelX0[ip];
            VelY[ip] = VelY0[ip];
            VelZ[ip] = VelZ0[ip];
        }

        iMaterial = abs(iMaterial);
        iDensity = Density[ip];
        iEnergy = Energy[ip];

        switch (dMatType[iMaterial]) {
        case (1) : // IDEAL GAS EOS
            Pressure[ip] = pressureGas(iMaterial, iDensity, iEnergy);
            Sound[ip] = soundGas(iMaterial, iDensity, iEnergy);
            break;
        case (2) : // MIE-GRUNEISEN POLYNOMIAL EOS
            Pressure[ip] = pressurePoly(iMaterial, iDensity, iEnergy);
            Sound[ip] = soundPoly(iMaterial, iDensity, iEnergy);
            break;
        case (3) : // MIE-GRUNEISEN SHOCK EOS
            Pressure[ip] = pressureShock(iMaterial, iDensity, iEnergy);
            Sound[ip] = soundShock(iMaterial, iDensity, iEnergy);
            break;
        case (4) : // TAIT EOS
            Pressure[ip] = pressureTait(iMaterial, iDensity, iEnergy);
            Sound[ip] = soundTait(iMaterial, iDensity, iEnergy);
            break;
        default :
            Pressure[ip] = 0.0f;
        }

    }
}


__device__ float kernelWendland(float r, float h) {

    float q, alpha, w;
    /**
     * \brief Wendland kernel
     *
     * \date Feb 8, 2011
     * \author Luca Massidda
     */

    q = r / h;

    // for 3D
    //alpha = 15.0f / (16.0f * PI * h * h * h);

    // for 2D
    alpha = 7.0f / (4.0f * PI * h * h);

    w = 0.0f;
    if (q < 2) {
        w = powf((1.0f - 0.5f*q),4);
        w *= 1.0f + 2.0f*q;
        w *= alpha;
    }

    return w;
}


__device__ float kernelDerivWendland(float r, float h) {

    float q, alpha, dwdr;
    /**
     * \brief Wendland kernel derivative
     *
     * \date Feb 8, 2011
     * \author Luca Massidda
     */

    q = r / h;

    // for 3D
    //alpha = 7.0f / (8.0f * PI * h * h * h);

    // for 2D
    alpha = 7.0f / (4.0f * PI * h * h);

    dwdr = 0.0f;
    if (q < 2) {
        dwdr = 5.0f / 8.0f * q * powf((q - 2.0f), 3) ;
        dwdr *= alpha / h;
    }

    return dwdr;
}


__device__ float kernelGauss(float r, float h) {

    float r2, q2, h2, alpha, w;//, dwdr;
    /**
     * \brief Gauss kernel
     *
     * \date Dec 21, 2010
     * \author Luca Massidda
     */

    r2 = r * r ;
    h2 = h * h;
    q2 = r2 / h2;


    //alpha = 1.0 / (pow(h, 1) * pow(3.14, 0.5));
    alpha = 1.0f / (3.14f * h2);

    w = 0.0f;
    //dwdr = 0.0;

    if (q2 < 4.0f) {
        w = alpha * expf(-q2);
        //dwdr = w * (-2.0 * r / h2);
    }

    return w;
}


__device__ float kernelDerivGauss(float r, float h) {

    float r2, q2, h2, alpha, w, dwdr;
    /**
     * \brief Gauss kernel
     *
     * \date Dec 21, 2010
     * \author Luca Massidda
     */

    r2 = r * r ;
    h2 = h * h;
    q2 = r2 / h2;


    alpha = 1.0f / (h * powf(3.14f, 0.5f));
    //alpha = 1.0f / (3.14f * h2);

    w = 0.0f;
    dwdr = 0.0f;

    if (q2 < 4.0f) {
        w = alpha * expf(-q2);
        dwdr = w * (-2.0f * r / h2);
    }

    return dwdr;
}


__global__ void balanceMassMomentumDevice(const int pn, const int* List,
        const float* Mass, const float* Smooth,
        const float* PosX, const float* PosY, const float* PosZ,
        const float* VelX, const float* VelY, const float* VelZ,
        const float* Density, const float* Pressure, const float* Sound,
        float* DensityDot, float* VelDotX, float* VelDotY, float* VelDotZ) {

    /**
     * \brief Interate particles
     *
     * \date Jan 6, 2011
     * \author Luca Massidda
     */

    int ip, il, jp;
    float iDensityDot;
    float iVelDotX, iVelDotY, iVelDotZ;
    float iSmooth, jMass;
    volatile float dx, dy, dz, dr, dvr, dwdr, f;

    ip = threadIdx.x + blockDim.x * blockIdx.x;

    if (ip < pn) {
        iDensityDot = 0.0f;
        iVelDotX = 0.0f;
        iVelDotY = 0.0f;
        iVelDotZ = 0.0f;
        iSmooth = Smooth[ip];

        for (il = 0; il < MAXN; il++) {
            jp = List[ip * MAXN + il];

            jMass = Mass[jp];

            dx = PosX[ip] - PosX[jp];
            dy = PosY[ip] - PosY[jp];
            dz = PosZ[ip] - PosZ[jp];
            dr = sqrtf(dx * dx + dy * dy + dz * dz);

            if (dr < (0.01f * iSmooth)) dr = 100.0f * iSmooth;

            //dwdr = kernelDerivGauss(dr, iSmooth);
            dwdr = kernelDerivWendland(dr, iSmooth);

            dvr = 0.0f;
            dvr += (PosX[ip] - PosX[jp]) * (VelX[ip] - VelX[jp]);
            dvr += (PosY[ip] - PosY[jp]) * (VelY[ip] - VelY[jp]);
            dvr += (PosZ[ip] - PosZ[jp]) * (VelZ[ip] - VelZ[jp]);

            iDensityDot += jMass * dvr * dwdr / dr;

            // Calculate interparticle pressure action
            f = -(Pressure[ip] + Pressure[jp])
                / (Density[ip] * Density[jp]);

            iVelDotX += jMass * f * dwdr * (PosX[ip] - PosX[jp]) / dr;
            iVelDotY += jMass * f * dwdr * (PosY[ip] - PosY[jp]) / dr;
            iVelDotZ += jMass * f * dwdr * (PosZ[ip] - PosZ[jp]) / dr;

            // Calculate shock correction for mass
            f = Density[ip] - Density[jp];
            f *= 2.0f * Sound[ip] / (Density[ip] + Density[jp]);

            iDensityDot += jMass * f * dwdr;

            // Calculate shock correction for momentum
            if (dvr < 0.0f) f = dvr;
            else f = 0.0f;

            f *= iSmooth / (dr * dr + 0.01f * iSmooth * iSmooth);
            f *= 2.0f * Sound[ip] / (Density[ip] + Density[jp]);
            f *= 0.03f;

            iVelDotX += jMass * f * dwdr * (PosX[ip] - PosX[jp]) / dr;
            iVelDotY += jMass * f * dwdr * (PosY[ip] - PosY[jp]) / dr;
            iVelDotZ += jMass * f * dwdr * (PosZ[ip] - PosZ[jp]) / dr;
        }

        DensityDot[ip] += iDensityDot;
        VelDotX[ip] += iVelDotX;
        VelDotY[ip] += iVelDotY;
        VelDotZ[ip] += iVelDotZ;
    }
}

__global__ void balanceEnergyDevice(const int pn,
                                    const float* Pressure, const float* Density,
                                    const float* DensityDot, float* EnergyDot) {

    /**
     * \brief Interate particles
     *
     * \date Jan 9, 2011
     * \author Luca Massidda
     */

    volatile int ip;
    float iPressure, iDensity, iDensityDot;
    float iEnergyDot;

    ip = threadIdx.x + blockDim.x * blockIdx.x;

    if (ip < pn) {
        iPressure = Pressure[ip];
        iDensity = Density[ip];
        iDensityDot = DensityDot[ip];

        iEnergyDot = (iPressure * iDensityDot) / (iDensity * iDensity);

        EnergyDot[ip] += iEnergyDot;
    }
}

int RKstepDevice(struct device_model *dm, struct pointer_model *pm, float alpha) {
    int blocks, threads;

    blocks = (pm->pn + THREADS - 1) / THREADS;
    threads = THREADS;

    thrust::fill(dm->VelDotX.begin(), dm->VelDotX.end(), 0.0f);
    thrust::fill(dm->VelDotY.begin(), dm->VelDotY.end(), 0.0f);
    thrust::fill(dm->VelDotZ.begin(), dm->VelDotZ.end(), 0.0f);
    thrust::fill(dm->DensityDot.begin(), dm->DensityDot.end(), 0.0f);
    thrust::fill(dm->EnergyDot.begin(), dm->EnergyDot.end(), 0.0f);

    // External loads
    updateLoadsDevice <<< blocks, threads >>>
    (pm->pn, pm->Material, pm->PosX, pm->PosY, pm->PosZ,
     pm->VelX, pm->VelY, pm->VelZ,
     pm->VelDotX, pm->VelDotY, pm->VelDotZ, pm->EnergyDot);

    // Calculate particle interactions
    balanceMassMomentumDevice <<< blocks, threads >>>
    (pm->pn, pm->List, pm->Mass, pm->Smooth, pm->PosX, pm->PosY, pm->PosZ,
     pm->VelX, pm->VelY, pm->VelZ, pm->Density, pm->Pressure, pm->Sound,
     pm->DensityDot, pm->VelDotX, pm->VelDotY, pm->VelDotZ);

    balanceEnergyDevice <<< blocks, threads >>>
    (pm->pn, pm->Pressure, pm->Density, pm->DensityDot, pm->EnergyDot);

    // Update particles
    updateParticlesDevice  <<< blocks, threads >>>
    (pm->pn, alpha, pm->Material, pm->VelDotX, pm->VelDotY, pm->VelDotZ, pm->DensityDot, pm->EnergyDot,
     pm->PosX0, pm->PosY0, pm->PosZ0, pm->VelX0, pm->VelY0, pm->VelZ0, pm->Density0, pm->Energy0,
     pm->PosX, pm->PosY, pm->PosZ, pm->VelX, pm->VelY, pm->VelZ, pm->Density, pm->Energy, pm->Pressure, pm->Sound);

    return 0;
}


int RKintegrateDevice(struct host_model *hm, struct device_model *dm, struct pointer_model *pm) {

    /**
     * \brief Runge Kutta 3rd order time integration
     *
     * Integrate the Navier Stokes equations in time with the
     * Total Variation Diminishing Runge-Kutta algorithm of the 3rd order
     *
     * \date Dec 20, 2010
     * \author Luca Massidda
     */

    int ts;


    // TIME CYCLE
    for (ts = 0; ts <= hRun.tsn; ts++) {

        // Output data
        if ((ts % hRun.ssi) == 0) {
            printf("Saving time: %g \n", ts * hRun.dt);
            copyDeviceToHost(dm, hm);
            resizeHost(hm);
            printData(hm);
            outputVTK(hm, ts / hRun.ssi);
        }
        // Calculate neighbour list
        neighbourListDevice(dm, pm);

        // Save initial condition
        backupDataDevice(dm);

        // Step 1
        RKstepDevice(dm, pm, 1.0f);

        // Step 2
        RKstepDevice(dm, pm, 1.0f / 4.0f);

        // Step 3
        RKstepDevice(dm, pm, 2.0f / 3.0f);

    }

    return 0;
}


int main() {
    /**
     * \brief armando2D v2.0
     *
     * An SPH code for non stationary fluid dynamics.
     * This is the reviewed and improved C version of Armando v1.0
     * developed at CERN in 2008
     *
     * \date May 2, 2012
     * \author Luca Massidda
     */

    struct host_model hModel;
    struct device_model dModel;
    struct pointer_model pModel;

    for (int i = 0; i < 10; i++) {
        hLoad[i].gx = 0.0f;
        hLoad[i].gy = 0.0f;
        hLoad[i].gz = 0.0f;
        hLoad[i].w = 0.0f;

        hDisp[i].nX = 0.0f;
        hDisp[i].nY = 0.0f;
        hDisp[i].nZ = 0.0f;
    }

    //initPump(&hModel);
    //initBlock(&hModel);
    //initDamBreak(&hModel);
    initChannel(&hModel);

    copyHostToDevice(&hModel, &dModel);
    resizeDevice(&dModel);

    copyDeviceToPointer(&dModel, &pModel);

//    cudaMemcpyToSymbol("dSmooth", &hSmooth, sizeof(float));
//    cudaMemcpyToSymbol("dMass", &hMass, sizeof(float));
//    cudaMemcpyToSymbol("dSound", &hSound, sizeof(float));
    cudaMemcpyToSymbol("dRun", &hRun, sizeof(struct simulation));
    cudaMemcpyToSymbol("dMatType", hMatType, 10 * sizeof(int));
    cudaMemcpyToSymbol("dMatProp", hMatProp, 100 * sizeof(float));
    cudaMemcpyToSymbol("dLoad", &hLoad, 10 * sizeof(struct load));
    cudaMemcpyToSymbol("dDisp", &hDisp, 10 * sizeof(struct disp));

    RKintegrateDevice(&hModel, &dModel, &pModel);

    return 0;
}
