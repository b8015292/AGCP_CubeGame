#pragma once

#include "Common/d3dApp.h"
using namespace DirectX;

class Collision {
public:
    struct ColCube
    {

        XMFLOAT3 tfl;//top front left
        XMFLOAT3 tfr;//top front right
        XMFLOAT3 tbl;//top back left
        XMFLOAT3 tbr;//top back right

        XMFLOAT3 bfl;//bottom front left
        XMFLOAT3 bfr;//bottom front right
        XMFLOAT3 bbl;//bottom back left
        XMFLOAT3 bbr;//bottom back right

        ColCube() {
            tfl = XMFLOAT3(0, 0, 0);
            tfr = XMFLOAT3(0, 0, 0);
            tbl = XMFLOAT3(0, 0, 0);
            tbr = XMFLOAT3(0, 0, 0);
            bfl = XMFLOAT3(0, 0, 0);
            bfr = XMFLOAT3(0, 0, 0);
            bbl = XMFLOAT3(0, 0, 0);
            bbr = XMFLOAT3(0, 0, 0);
        }

        ColCube(XMFLOAT3 ntfl, XMFLOAT3 ntfr, XMFLOAT3 ntbl, XMFLOAT3 ntbr, XMFLOAT3 nbfl, XMFLOAT3 nbfr, XMFLOAT3 nbbl, XMFLOAT3 nbbr) {
            tfl = ntfl;
            tfr = ntfr;
            tbl = ntbl;
            tbr = ntbr;
            bfl = nbfl;
            bfr = nbfr;
            bbl = nbbl;
            bbr = nbbr;
        }
    };

    struct ColPoint {
        XMFLOAT3 ftl; //Front Top Left		- 7th in vertex array
        XMFLOAT3 size; //Width, Height, Depth

        ColPoint() {
            ftl = XMFLOAT3(0, 0, 0);
            size = XMFLOAT3(0, 0, 0);
        }

        ColPoint(const XMFLOAT3 nFTL, const XMFLOAT3 nSize) {
            ftl = nFTL;
            size = nSize;
        }
    };

    bool Within(const ColCube cube, const XMFLOAT3 point);
    bool CheckCollisions(const ColCube a, const ColCube b);

};