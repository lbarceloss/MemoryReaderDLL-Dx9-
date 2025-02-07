#include <cmath>
#include <cstdio>
#include <string>
#include <algorithm>
#include <cctype>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace GameCalc {

    double BallBreak(double x, double y, double ballX, double ballY) {
        double PIXEL_CONST = 1 / 0.00875;
        double DEGREE = 180.0 / M_PI;
        double sineRadians = asin(x) * DEGREE;
        double cosineRadians = acos(y) * DEGREE;
        double position = (sineRadians < 0.0) ? 180 - (cosineRadians - 180) : cosineRadians;
        double radiansPosition = position * M_PI / 180.0;
        radiansPosition *= -1;
        double inverseSine = sin(radiansPosition) * -1;
        double cosine = cos(radiansPosition);
        double resultAutoBreak = round(((ballX * cosine) + (ballY * inverseSine)) * -1 * PIXEL_CONST * 100.0) / 100.0;
        if (resultAutoBreak == -0.0)
            resultAutoBreak = 0.0;
        return resultAutoBreak;
    }

    double PB(double x1, double x2, double z1, double z2, double charGridMem) {
        double cameraAngle = atan2(x2 - x1, z2 - z1);
        double distanceRoot = sqrt(pow(x2 - x1, 2) + pow(z2 - z1, 2));
        double radsex = charGridMem;
        double rad = fmod(fabs(radsex), 6.28318530717659);
        rad *= (radsex <= 0 ? -1 : 1);
        double pb2 = ((distanceRoot * 0.3125) * tan(rad + cameraAngle)) / (1.5 * 0.2167) * -1;
        return round(pb2 * 100.0) / 100.0;
    }


    double DegreeAngle(double x, double y) {
        double angleInRadians = atan2(-y, x);
        double angleInDegrees = angleInRadians * 180.0 / M_PI;
        if (angleInDegrees < 0)
            angleInDegrees += 360;
        return angleInDegrees;
    }

    double Angle90(double x, double y) {
        if (x < 0) x = -x;
        if (y < 0) y = -y;
        double angleInRadians = atan2(y, x);
        double angleInDegrees = angleInRadians * 180.0 / M_PI;
        if (angleInDegrees < 0)
            angleInDegrees += 360;
        angleInDegrees = fmod(angleInDegrees + 90, 90);
        return angleInDegrees;
    }

    double Distance(double x1, double x2, double y1, double y2) {
        return round(sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2)) * 0.312495 * 100.0) / 100.0;
    }

    double Height(double x1, double x2) {
        return round((x2 - x1 + 0.14) * (0.312495 * 0.914) * 10.0) / 10.0;
    }

    int Terrain(int x) {
        return 100 - x;
    }

    std::string Spin(double spinMem, double spinMaxMem) {
        char buffer[64];
        sprintf_s(buffer, "%0.2f/%g", spinMem, spinMaxMem);
        return std::string(buffer);
    }

    std::string Curve(double curveMem, double curveMaxMem) {
        char buffer[64];
        sprintf_s(buffer, "%0.2f/%g", curveMem, curveMaxMem);
        return std::string(buffer);
    }
}
