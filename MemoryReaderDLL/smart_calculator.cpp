#include "smart_calculator.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm> 
#include <limits>    
#include <cstdlib>   
#include <ctime>     
#include <map>      

// Global Variable Definitions 
const Vector3D _00E42544_vect_slope(0.0, 0.0, 1.0);

Ball global_ball_obj;
Club global_club_obj;
Wind global_wind_obj;

const std::vector<ClubInfo> ALL_CLUB_INFO = {
    ClubInfo(CLUB_TYPE::WOOD, 0.55, 1.61, 236.0, 10.0, 230.0), // 1W
    ClubInfo(CLUB_TYPE::WOOD, 0.50, 1.41, 204.0, 13.0, 210.0), // 2W
    ClubInfo(CLUB_TYPE::WOOD, 0.45, 1.26, 176.0, 16.0, 190.0), // 3W
    ClubInfo(CLUB_TYPE::IRON, 0.45, 1.07, 161.0, 20.0, 180.0), // 2I
    ClubInfo(CLUB_TYPE::IRON, 0.45, 0.95, 149.0, 24.0, 170.0), // 3I
    ClubInfo(CLUB_TYPE::IRON, 0.45, 0.83, 139.0, 28.0, 160.0), // 4I
    ClubInfo(CLUB_TYPE::IRON, 0.45, 0.73, 131.0, 32.0, 150.0), // 5I
    ClubInfo(CLUB_TYPE::IRON, 0.41, 0.67, 124.0, 36.0, 140.0), // 6I
    ClubInfo(CLUB_TYPE::IRON, 0.36, 0.61, 118.0, 40.0, 130.0), // 7I
    ClubInfo(CLUB_TYPE::IRON, 0.30, 0.57, 114.0, 44.0, 120.0), // 8I
    ClubInfo(CLUB_TYPE::IRON, 0.25, 0.53, 110.0, 48.0, 110.0), // 9I
    ClubInfo(CLUB_TYPE::PW,   0.18, 0.49, 107.0, 52.0, 100.0), // PW
    ClubInfo(CLUB_TYPE::PW,   0.17, 0.42, 93.0,  56.0, 80.0),  // SW
    // para que PT1 e PT2 que porra e essa mas esta no HTML doenca da porra.
    // ClubInfo(CLUB_TYPE::PT, 0.00, 0.00, 30.0, 0.00, 20.0),    // PT1
    // ClubInfo(CLUB_TYPE::PT, 0.00, 0.00, 21.0, 0.00, 10.0)     // PT2
};

const std::vector<std::string> CLUB_INFO_ENUM_NAMES = {
    "1W", "2W", "3W", "2I", "3I", "4I", "5I", "6I", "7I", "8I", "9I", "PW", "SW"
};

const std::vector<std::string> POWER_SHOT_FACTORY_ENUM_NAMES = {
    "NO_POWER_SHOT", "ONE_POWER_SHOT", "TWO_POWER_SHOT", "ITEM_15_POWER_SHOT"
};
const std::vector<std::string> SHOT_TYPE_ENUM_NAMES = {
    "DUNK", "TOMAHAWK", "SPIKE", "COBRA"
};


// Vector3D Implementation 
Vector3D::Vector3D(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {}

Vector3D& Vector3D::normalize() {
    double l = length();
    if (l != 0) {
        x /= l; y /= l; z /= l;
    }
    else {
        x = y = z = 0.0;
    }
    return *this;
}

Vector3D& Vector3D::multiplyScalar(double value) {
    x *= value; y *= value; z *= value;
    return *this;
}

Vector3D& Vector3D::add(const Vector3D& other) {
    x += other.x; y += other.y; z += other.z;
    return *this;
}
Vector3D& Vector3D::add3D(double dx, double dy, double dz) {
    x += dx; y += dy; z += dz;
    return *this;
}


Vector3D& Vector3D::sub(const Vector3D& other) {
    x -= other.x; y -= other.y; z -= other.z;
    return *this;
}
Vector3D& Vector3D::sub3D(double dx, double dy, double dz) {
    x -= dx; y -= dy; z -= dz;
    return *this;
}

Vector3D& Vector3D::divideScalar(double value) {
    if (value != 0) {
        double scalar = 1.0 / value;
        x *= scalar; y *= scalar; z *= scalar;
    }
    else {
        x = y = z = 0.0;
    }
    return *this;
}

Vector3D& Vector3D::cross(const Vector3D& other) {
    double cur_x = x, cur_y = y, cur_z = z;
    x = cur_y * other.z - cur_z * other.y;
    y = cur_z * other.x - cur_x * other.z;
    z = cur_x * other.y - cur_y * other.x;
    return *this;
}

double Vector3D::length() const {
    return std::sqrt((x * x) + (y * y) + (z * z));
}

Vector3D Vector3D::clone() const {
    return Vector3D(x, y, z);
}

// Ball Implementation
Ball::Ball() :
    position(0, 0, 0), slope(0, 1, 0), state_process(0), max_height(0.0), num_max_height(-1),
    count(0), velocity(0, 0, 0), ball_28(0.0), ball_2C(0.0), ball_30(0.0),
    curva(0.0), spin(0.0), rotation_curve(0.0), rotation_spin(0.0),
    ball_44(0), ball_48(0), ball_70(-1), ball_90(0), ball_BC(0),
    mass(0.045926999), diametro(0.14698039) {
}

void Ball::copy(const Ball& other) {
    position = other.position;
    slope = other.slope;
    velocity = other.velocity;
    state_process = other.state_process;
    max_height = other.max_height;
    spin = other.spin;
    curva = other.curva;
    count = other.count;
    num_max_height = other.num_max_height;
    ball_28 = other.ball_28;
    ball_2C = other.ball_2C;
    ball_30 = other.ball_30;
    // ball_3C = other.ball_3C; // Foda-se
    // ball_40 = other.ball_40; // Foda-se
    ball_44 = other.ball_44;
    ball_48 = other.ball_48;
    ball_70 = other.ball_70;
    ball_90 = other.ball_90;
    ball_BC = other.ball_BC;
    // ball_C4 = other.ball_C4; // Foda-se
    // ball_C8 = other.ball_C8; // Foda-se
    //mass e diametro e const nao precisa de copy quem fez isso e um idiota.
}

//  ClubInfo Implementation
ClubInfo::ClubInfo(CLUB_TYPE t, double rs, double rc, double pf, double deg, double pb)
    : type(t), rotation_spin(rs), rotation_curve(rc), power_factor(pf), degree(deg), power_base(pb) {
}

// Club Implementation
Club::Club() :
    type(CLUB_TYPE::WOOD), type_distance(TYPE_DISTANCE::BIGGER_OR_EQUAL_58),
    rotation_spin(0.55), rotation_curve(1.61), power_factor(236), degree(10), power_base(230) {
}

void Club::init(const ClubInfo& club_info) {
    type = club_info.type;
    rotation_spin = club_info.rotation_spin;
    rotation_curve = club_info.rotation_curve;
    power_factor = club_info.power_factor;
    degree = club_info.degree;
    power_base = club_info.power_base;
}

double Club::getDregRad() const {
    return degree * M_PI / 180.0;
}

double Club::ExtraPower::total(POWER_SHOT_FACTORY ps_type) const {
    double pwr = auxpart + mascot + card;
    if (ps_type == POWER_SHOT_FACTORY::ONE_POWER_SHOT ||
        ps_type == POWER_SHOT_FACTORY::TWO_POWER_SHOT ||
        ps_type == POWER_SHOT_FACTORY::ITEM_15_POWER_SHOT) {
        pwr += ps_auxpart + ps_mascot + ps_card;
    }
    return pwr;
}

double Club::getPower(const ExtraPower& extraPower, int pwrSlot, POWER_SHOT_FACTORY ps, double spin_val) const {
    double pwrjard = 0.0;
    double ps_factory_val = getPowerShotFactoryValue(ps);

    switch (type) {
    case CLUB_TYPE::WOOD: {
        pwrjard = extraPower.total(ps) + ps_factory_val + ((static_cast<double>(pwrSlot) - 15.0) * 2.0);
        pwrjard *= 1.5;
        pwrjard /= power_base;
        pwrjard += 1.0;
        pwrjard *= power_factor;
        break;
    }
    case CLUB_TYPE::IRON: {
        pwrjard = ((ps_factory_val / power_base + 1.0) * power_factor) + (extraPower.total(ps) * power_factor * 1.3) / power_base;
        break;
    }
    case CLUB_TYPE::PW: {
        auto getPowerByDegree = [&](double deg_rad, double spin_arg) {
            return 0.5 + ((0.5 * (deg_rad + (spin_arg * _00D19B98))) / (56.0 * M_PI / 180.0));
            };

        switch (type_distance) {
        case TYPE_DISTANCE::LESS_10:
        case TYPE_DISTANCE::LESS_15:
        case TYPE_DISTANCE::LESS_28:
            pwrjard = (getPowerByDegree(getDregRad(), spin_val) * (52.0 + (ps_factory_val > 0 ? 28.0 : 0.0))) + (extraPower.total(ps) * power_factor) / power_base;
            break;
        case TYPE_DISTANCE::LESS_58:
            pwrjard = (getPowerByDegree(getDregRad(), spin_val) * (80.0 + (ps_factory_val > 0 ? 18.0 : 0.0))) + (extraPower.total(ps) * power_factor) / power_base;
            break;
        case TYPE_DISTANCE::BIGGER_OR_EQUAL_58:
            pwrjard = ((ps_factory_val / power_base + 1.0) * power_factor) + (extraPower.total(ps) * power_factor) / power_base;
            break;
        }
        break;
    }
    case CLUB_TYPE::PT: {
        pwrjard = power_factor;
        break;
    }
    }
    return pwrjard;
}

double Club::getPower2(const ExtraPower& extraPower, int pwrSlot, POWER_SHOT_FACTORY ps) const {

    bool is_ps_active = (ps != POWER_SHOT_FACTORY::NO_POWER_SHOT);

    double pwrjard = (extraPower.auxpart + extraPower.mascot + extraPower.card) / 2.0 + (static_cast<double>(pwrSlot) - 15.0);
    if (is_ps_active) {
        pwrjard += (extraPower.ps_card / 2.0);
    }
    pwrjard /= 170.0;
    return pwrjard + 1.5;
}

double Club::getRange(const ExtraPower& extraPower, int pwrSlot, POWER_SHOT_FACTORY ps) const {
    double pwr_range = power_base + extraPower.total(ps) + getPowerShotFactoryValue(ps);
    double ps_factory_val = getPowerShotFactoryValue(ps);


    if (type == CLUB_TYPE::WOOD) {
        pwr_range += ((static_cast<double>(pwrSlot) - 15.0) * 2.0);
    }

    if (type == CLUB_TYPE::PW) {
        switch (type_distance) {
        case TYPE_DISTANCE::LESS_10:
        case TYPE_DISTANCE::LESS_15:
        case TYPE_DISTANCE::LESS_28:
            pwr_range = 30.0 + (ps_factory_val > 0 ? 30.0 : 0.0) + extraPower.total(ps);
            break;
        case TYPE_DISTANCE::LESS_58:
            pwr_range = 60.0 + (ps_factory_val > 0 ? 20.0 : 0.0) + extraPower.total(ps);
            break;
        case TYPE_DISTANCE::BIGGER_OR_EQUAL_58:
            break;
        }
    }
    return pwr_range;
}


//  Wind Implementation 
Wind::Wind() : wind_strength(0), degree(0) {}

Vector3D Wind::getWind() const {
    return Vector3D(
        wind_strength * std::sin(degree * M_PI / 180.0) * -1.0,
        0.0,
        wind_strength * std::cos(degree * M_PI / 180.0)
    );
}

//  QuadTree Implementation 
QuadTree::QuadTree() :
    gravityFactor(1.0), gravity(34.295295715332), _21D8_vect(0, 0, 0),
    current_ball(nullptr), current_club(nullptr), current_wind(nullptr),
    ball_position_init(0, 0, 0), power_range_shot(0.0),
    current_shot_type(SHOT_TYPE::DUNK), power_factor_shot(0.0),
    percentShot_sqrt(0.0), spike_init(-1), spike_med(-1), power_factor(0.0), cobra_init(-1)
{
}

double QuadTree::getGravity() const {
    return gravity * gravityFactor;
}

void QuadTree::initShot(Ball& ball_ref, Club& club_ref, Wind& wind_ref, const InitShotOptions& options) {
    current_ball = &ball_ref;
    current_club = &club_ref;
    current_wind = &wind_ref;

    current_shot_type = options.shot_type;
    spike_init = -1;
    spike_med = -1;
    cobra_init = -1;

    current_ball->position = options.position.clone();
    ball_position_init = options.position.clone(); // para o Cobra que nao funciona direito vou corrigir isso mais tarde
    //pelo que analisei fazendo Engenharia Reversa na estrutura do Cobra a algumas conclusoes erroneas parece ter sido feito de qualquer jeito.

    current_club->type_distance = calculeTypeDistance(options.distance);

    current_ball->max_height = current_ball->position.y;
    current_ball->count = 0;
    current_ball->num_max_height = -1;

    double pwr = current_club->getPower(options.power.options, options.power.pwr, options.ps_type, options.spin);

    power_range_shot = current_club->getRange(options.power.options, options.power.pwr, options.ps_type);
    power_factor = pwr; // Para Spike mesmo problema que o Cobra mal feito.

    pwr *= std::sqrt(options.percentShot);

    if (options.shot_type == SHOT_TYPE::TOMAHAWK || options.shot_type == SHOT_TYPE::SPIKE)
        pwr *= 1.3;


    pwr *= std::sqrt(options.ground * 0.01); // Ground 0-100

    power_factor_shot = pwr;
    percentShot_sqrt = std::sqrt(options.percentShot);

    current_ball->curva = options.curva; // curva/30
    current_ball->spin = options.spin;   // spin/30

    //Essa parte tive que modificar um pouco muito muito muito mal feito ta loco.
    DegreeValues val1_deg_vals = getValuesDegree(options.mira_rad - (current_ball->curva * _00D17908), 1);
    DegreeValues val2_deg_vals = getValuesDegree(
        (current_club->type_distance == TYPE_DISTANCE::BIGGER_OR_EQUAL_58 ? current_club->getDregRad() : current_club->getDregRad() + (current_ball->spin * _00D19B98)),
        0
    );

    double line_ball_random_val = static_cast<double>(std::rand()) / RAND_MAX;
    current_ball->curva -= getSlope(options.mira_rad - options.slope_mira_rad, line_ball_random_val);

    pwr *= ((std::abs(current_ball->curva) * 0.1) + 1.0);

    Vector3D vectA(val2_deg_vals.neg_sin, val2_deg_vals.neg_rad, val2_deg_vals.cos2);
    vectA.multiplyScalar(pwr);

    Vector3D v1(val1_deg_vals.cos, val1_deg_vals.rad, val1_deg_vals.sin);
    Vector3D v2(val1_deg_vals._C, val1_deg_vals._10, val1_deg_vals._14);
    Vector3D v3(val1_deg_vals.neg_sin, val1_deg_vals.neg_rad, val1_deg_vals.cos2);
    Vector3D v4(val1_deg_vals._24, val1_deg_vals._28, val1_deg_vals._2C);

    current_ball->velocity.x = v2.x * vectA.y + vectA.x * v1.x + v3.x * vectA.z + v4.x;
    current_ball->velocity.y = v1.y * vectA.x + v2.y * vectA.y + v3.y * vectA.z + v4.y;
    current_ball->velocity.z = v1.z * vectA.x + v2.z * vectA.y + v3.z * vectA.z + v4.z;

    current_ball->rotation_curve = current_ball->curva * options.percentShot;
    current_ball->rotation_spin = current_club->type_distance == TYPE_DISTANCE::BIGGER_OR_EQUAL_58
        ? (current_club->getPower2(options.power.options, options.power.pwr, options.ps_type) * options.percentShot) * options.percentShot
        : 0.0;

    current_ball->ball_48 = current_ball->ball_44; // Flag Power Shot
}


DegreeValues QuadTree::getValuesDegree(double degree_val, int option) const {
    DegreeValues obj;
    if (option == 0) {
        obj.cos = 1.0;
        obj.rad = 0.0;
        obj.sin = 0.0;
        obj._C = 0.0;
        obj._10 = std::cos(degree_val);
        obj._14 = std::sin(degree_val) * -1.0;
        obj.neg_sin = 0.0;
        obj.neg_rad = std::sin(degree_val);
        obj.cos2 = obj._10;
        obj._24 = 0.0;
        obj._28 = 0.0;
        obj._2C = 0.0;
    }
    else if (option == 1) {
        obj.cos = std::cos(degree_val);
        obj.rad = 0.0;
        obj.sin = std::sin(degree_val);
        obj._C = 0.0;
        obj._10 = 1.0;
        obj._14 = 0.0;
        obj.neg_sin = obj.sin * -1.0;
        obj.neg_rad = 0.0;
        obj.cos2 = obj.cos;
        obj._24 = 0.0;
        obj._28 = 0.0;
        obj._2C = 0.0;
    }
    return obj;
}

struct Matrix4x3 {
    Vector3D v1, v2, v3, v4;
};

double QuadTree::getSlope(double mira, double line_ball) {
    // Ajudar lambda a converter DegreeValues para Matrix4x3
    auto valuesDegreeToMatrix = [](const DegreeValues& value) {
        Matrix4x3 m;
        m.v1 = Vector3D(value.cos, value.rad, value.sin);
        m.v2 = Vector3D(value._C, value._10, value._14); // y-axis vector
        m.v3 = Vector3D(value.neg_sin, value.neg_rad, value.cos2);
        m.v4 = Vector3D(value._24, value._28, value._2C);
        return m;
        };

    auto applyMatrix = [](const Matrix4x3& m1, const Matrix4x3& m2) {
        Matrix4x3 result;
        // v1
        result.v1.x = m1.v1.x * m2.v1.x + m1.v1.y * m2.v2.x + m1.v1.z * m2.v3.x;
        result.v1.y = m1.v1.x * m2.v1.y + m1.v1.y * m2.v2.y + m1.v1.z * m2.v3.y;
        result.v1.z = m1.v1.x * m2.v1.z + m1.v1.y * m2.v2.z + m1.v1.z * m2.v3.z;
        // v2
        result.v2.x = m1.v2.x * m2.v1.x + m1.v2.y * m2.v2.x + m1.v2.z * m2.v3.x;
        result.v2.y = m1.v2.x * m2.v1.y + m1.v2.y * m2.v2.y + m1.v2.z * m2.v3.y;
        result.v2.z = m1.v2.x * m2.v1.z + m1.v2.y * m2.v2.z + m1.v2.z * m2.v3.z;
        // v3
        result.v3.x = m1.v3.x * m2.v1.x + m1.v3.y * m2.v2.x + m1.v3.z * m2.v3.x;
        result.v3.y = m1.v3.x * m2.v1.y + m1.v3.y * m2.v2.y + m1.v3.z * m2.v3.y;
        result.v3.z = m1.v3.x * m2.v1.z + m1.v3.y * m2.v2.z + m1.v3.z * m2.v3.z;
        // v4 (translation part)
        result.v4.x = m1.v4.x * m2.v1.x + m1.v4.y * m2.v2.x + m1.v4.z * m2.v3.x + m2.v4.x;
        result.v4.y = m1.v4.x * m2.v1.y + m1.v4.y * m2.v2.y + m1.v4.z * m2.v3.y + m2.v4.y;
        result.v4.z = m1.v4.x * m2.v1.z + m1.v4.y * m2.v2.z + m1.v4.z * m2.v3.z + m2.v4.z;
        return result;
        };

    Vector3D ball_slope_cross_const_vect = current_ball->slope.clone().cross(_00E42544_vect_slope);

    Matrix4x3 slope_matrix;
    slope_matrix.v1 = ball_slope_cross_const_vect.clone().normalize();
    slope_matrix.v2 = current_ball->slope.clone(); // Y-axis
    slope_matrix.v3 = ball_slope_cross_const_vect.clone().cross(current_ball->slope).normalize(); // Re-cross para Z
    slope_matrix.v4 = Vector3D(0.0, 0.0, 0.0);


    DegreeValues val1_deg = getValuesDegree(mira * -1.0, 1);
    DegreeValues val2_deg = getValuesDegree(line_ball * -2.0, 1); // line_ball Random e a cabeca da minha pika!

    Matrix4x3 m1 = applyMatrix(valuesDegreeToMatrix(val2_deg), slope_matrix);
    Matrix4x3 m2 = applyMatrix(m1, valuesDegreeToMatrix(val1_deg));

    return m2.v2.x * _00D66CA0;
}


void QuadTree::ballProcess(double steptime, std::optional<double> final_step) {
    bounceProcess(steptime, final_step);

    if (current_shot_type == SHOT_TYPE::COBRA && cobra_init < 0) {
        if (percentShot_sqrt < std::sqrt(0.8))
            percentShot_sqrt = std::sqrt(0.8);

        if (current_ball->count == 0) {
            current_ball->velocity.y = 0.0;
            current_ball->velocity.normalize().multiplyScalar(power_factor_shot);
        }

        double diff = current_ball->position.clone().sub(ball_position_init).length();
        double cobra_init_up = ((power_range_shot * percentShot_sqrt) - 100.0) * 3.2;

        if (diff >= cobra_init_up) {
            double power_multiply = 0.0;
            if (current_club->type == CLUB_TYPE::WOOD) {
                if (current_club->power_base == 230.0) power_multiply = 74.0;
                else if (current_club->power_base == 210.0) power_multiply = 76.0;
                else if (current_club->power_base == 190.0) power_multiply = 80.0;
            }
            cobra_init = current_ball->count;
            current_ball->velocity.normalize().multiplyScalar(power_multiply).multiplyScalar(percentShot_sqrt);
            current_ball->rotation_spin = 2.5;
        }
    }
    else {
        if (spike_init < 0 && cobra_init < 0 && current_club->type_distance == TYPE_DISTANCE::BIGGER_OR_EQUAL_58) {
            current_ball->rotation_spin -= ((_00D66CA0 - (current_ball->spin * _00CFF040)) * _00D083A0);
        }
        else if ((current_shot_type == SHOT_TYPE::SPIKE && spike_init >= 0) || (current_shot_type == SHOT_TYPE::COBRA && cobra_init >= 0)) {
            current_ball->rotation_spin -= _00D083A0;
        }

        if (current_shot_type == SHOT_TYPE::SPIKE && current_ball->count == 0) {
            current_ball->velocity.y = 0.0;
            current_ball->velocity.normalize().multiplyScalar(power_factor_shot);
            current_ball->velocity.normalize().multiplyScalar(72.5).multiplyScalar(percentShot_sqrt * 2.0);
            current_ball->rotation_spin = 3.1;
            spike_init = current_ball->count;
        }

        if (current_shot_type == SHOT_TYPE::SPIKE && current_ball->num_max_height >= 0 && (current_ball->num_max_height + 0x3C) < current_ball->count && spike_med < 0) {
            spike_med = current_ball->count;
            if (current_club->type == CLUB_TYPE::WOOD) {
                double new_power = 0.0;
                // This logic is highly specific, ensure all numbers are doubles
                if (current_club->power_base == 230.0) {
                    new_power = 344.0;
                    if ((power_factor * percentShot_sqrt) < 344.0) new_power -= (power_factor * percentShot_sqrt);
                    else new_power = 0.0;
                    new_power = new_power / 112.0 * 21.5;
                    new_power = -8.0 - new_power;
                    current_ball->velocity.y = new_power;
                }
                else if (current_club->power_base == 210.0) {
                    new_power = 306.0;
                    if ((power_factor * percentShot_sqrt) < 306.0) new_power -= (power_factor * percentShot_sqrt);
                    else new_power = 0.0;
                    new_power = new_power / 105.0 * 20.5;
                    new_power = -10.3 - new_power;
                    current_ball->velocity.y = new_power;
                }
                else if (current_club->power_base == 190.0) {
                    new_power = 273.0;
                    if ((power_factor * percentShot_sqrt) < 273.0) new_power -= (power_factor * percentShot_sqrt);
                    else new_power = 0.0;
                    new_power = new_power / 100.0 * 20.2;
                    new_power = -10.8 - new_power;
                    current_ball->velocity.y = new_power;
                }
            }
            current_ball->velocity.multiplyScalar(percentShot_sqrt * 7.0);
            current_ball->rotation_spin = current_ball->spin; // spin/30
        }
    }

    if (current_ball->velocity.y < 0 && current_ball->num_max_height < 0) {
        current_ball->max_height = current_ball->position.y; // max_altura
        current_ball->num_max_height = current_ball->count;
    }
    current_ball->count++;
}

void QuadTree::bounceProcess(double steptime, std::optional<double> final_step_opt) {
    if (current_shot_type == SHOT_TYPE::SPIKE && current_ball->num_max_height >= 0 && (current_ball->num_max_height + 0x3C) > current_ball->count)
        return;

    Vector3D accelVect = applyForce();
    Vector3D otherVect = accelVect.clone();
    otherVect.divideScalar(current_ball->mass).multiplyScalar(steptime);
    current_ball->velocity.add(otherVect);

    if (current_ball->num_max_height == -1) {
        Vector3D tmpVect = _21D8_vect.clone().divideScalar(current_ball->mass).multiplyScalar(steptime);
        current_ball->velocity.add(tmpVect);
    }

    current_ball->ball_2C += (current_ball->rotation_curve * _00D1A888 * steptime);
    current_ball->ball_30 += (current_ball->rotation_spin * _00D3D210 * steptime);

    double effective_steptime = final_step_opt.has_value() ? final_step_opt.value() : steptime;
    current_ball->position.add(current_ball->velocity.clone().multiplyScalar(effective_steptime));
}

Vector3D QuadTree::applyForce() {
    Vector3D retVect(0, 0, 0);

    if (current_ball->rotation_curve != 0) {
        Vector3D vectorb(current_ball->velocity.z * _00D046A8, 0, current_ball->velocity.x);
        vectorb.normalize();
        if (cobra_init < 0 || spike_init < 0) // Tanto erro mas tanto erro que refiz tudo vai tomar no cu aqui foi o ponto que quase desisti qualquer um fica louco.
            vectorb.multiplyScalar(_00D00190 * current_ball->rotation_curve * current_club->rotation_curve);
        retVect.add(vectorb);
    }

    if (current_shot_type == SHOT_TYPE::SPIKE && spike_init < 0) // Spike inicio
        return Vector3D(0.0, 0.0, 0.0);
    else if (current_shot_type == SHOT_TYPE::COBRA && cobra_init < 0) // Cobra inicio
        return retVect;

    // All other cases, or later phases of Spike/Cobra
    Vector3D windVect = current_wind->getWind();
    windVect.multiplyScalar((current_shot_type == SHOT_TYPE::SPIKE ? _00D16758 : _00D083A0));
    retVect.add(windVect);

    retVect.y = retVect.y - (getGravity() * current_ball->mass);

    if (current_ball->rotation_spin != 0)
        retVect.y = retVect.y + (current_club->rotation_spin * _00D66CF8 * current_ball->rotation_spin);

    Vector3D velVect = current_ball->velocity.clone();
    velVect.multiplyScalar(velVect.length() * _00D3D028); // resistencia do ar
    retVect.sub(velVect);

    return retVect;
}


// Helper Function Implementations 
TYPE_DISTANCE calculeTypeDistance(double distance) {
    if (distance >= 58.0) return TYPE_DISTANCE::BIGGER_OR_EQUAL_58;
    if (distance < 10.0) return TYPE_DISTANCE::LESS_10;
    if (distance < 15.0) return TYPE_DISTANCE::LESS_15;
    if (distance < 28.0) return TYPE_DISTANCE::LESS_28;
    // if (distance < 58.0) // Vou nem comentar nada!
    return TYPE_DISTANCE::LESS_58;
}

double getPowerShotFactoryValue(POWER_SHOT_FACTORY ps) {
    switch (ps) {
    case POWER_SHOT_FACTORY::ONE_POWER_SHOT: return 10.0;
    case POWER_SHOT_FACTORY::TWO_POWER_SHOT: return 20.0;
    case POWER_SHOT_FACTORY::ITEM_15_POWER_SHOT: return 15.0;
    case POWER_SHOT_FACTORY::NO_POWER_SHOT:
    default: return 0.0;
    }
}

double diffYZ(const Vector3D& v1, const Vector3D& v2) {
    return std::sqrt(std::pow(v1.x - v2.x, 2) + std::pow(v1.z - v2.z, 2));
}

FindPowerResult::FindPowerResult() : power(-1.0), desvio(0.0), power_range(0.0), found(false) {}


FindPowerResult find_power(
    const Club::ExtraPower& power_player_options,
    int player_pwr_slot,
    const ClubInfo& club_info_to_use,
    SHOT_TYPE shot,
    POWER_SHOT_FACTORY power_shot,
    double distancia,
    double altura,
    double vento_strength,
    double angulo_vento,
    double terreno,
    double spin_input, // 0-30 spin
    double curva_input, //  0-30 curve
    const SlopeInputType& slope_param,
    std::optional<double> mira_opt,
    std::optional<double> percent_opt
) {
    const double altura_colision = altura * 1.094 * 3.2;
    const double distanciaScale = distancia * 3.2;

    Ball vball_instance; // instancia local para simulacao
    Club vclub_instance; // instancia local

    vclub_instance.init(club_info_to_use);
    vclub_instance.type_distance = calculeTypeDistance(distancia);

    double slope_mira_rad_val = 0.0;

    // Handle slope_param
    if (std::holds_alternative<Vector3D>(slope_param)) {
        const Vector3D& slope_vec = std::get<Vector3D>(slope_param);
        slope_mira_rad_val = slope_vec.y;
        vball_instance.slope = slope_vec;
        vball_instance.slope.y = 1.0;
    }
    else if (std::holds_alternative<double>(slope_param)) {
        double slope_double = std::get<double>(slope_param);
        vball_instance.slope = Vector3D(slope_double * SLOPE_BREAK_TO_CURVE_SLOPE * -1.0, 1.0, 0.0);
    }


    const double margin = 0.05;
    const int limit_checking = 1000;
    int count = 0;
    bool isFind = false;
    FindPowerResult found_result;

    Wind vwind_instance; // instancia local
    vwind_instance.wind_strength = vento_strength;
    vwind_instance.degree = angulo_vento;

    InitShotOptions shot_options;
    shot_options.distance = distancia;
    shot_options.percentShot = percent_opt.has_value() ? percent_opt.value() : 1.0;
    shot_options.ground = terreno; // 0-100
    shot_options.mira_rad = mira_opt.has_value() ? mira_opt.value() : 0.0;
    shot_options.slope_mira_rad = slope_mira_rad_val;
    shot_options.spin = spin_input / 30.0;   // 0-1 
    shot_options.curva = curva_input / 30.0; // 0-1 
    shot_options.position = Vector3D(0, 0, 0); // posicao inicial da bola
    shot_options.shot_type = shot;
    shot_options.ps_type = power_shot;
    shot_options.power.pwr = player_pwr_slot;
    shot_options.power.options = power_player_options;


    found_result.smartData.options_copy = shot_options;
    found_result.smartData.club_info_copy = club_info_to_use;


    double powerRange = vclub_instance.getRange(shot_options.power.options, shot_options.power.pwr, shot_options.ps_type);

    // Lambda for findAlturaColision
    auto findAlturaColision_lambda =
        [&](QuadTree& qt, double alt_col) -> double {
        int inner_count = 0;
        Ball copy_ball;

        do {
            copy_ball.copy(vball_instance);
            qt.ballProcess(_00D083A0);
        } while ((vball_instance.position.y > alt_col || vball_instance.num_max_height == -1) && (++inner_count) < 3000);


        double denominator = vball_instance.position.y - copy_ball.position.y;
        double last_step_factor = 0.0;
        if (std::abs(denominator) > 1e-9) { // evitar divisao por 0 se nao ja viu ne C++ e chato. SEXO!
            last_step_factor = std::abs((alt_col - copy_ball.position.y) / denominator);
        }

        vball_instance.copy(copy_ball);

        qt.ballProcess(_00D083A0, _00D083A0 * last_step_factor);

        if (std::abs(distanciaScale - vball_instance.position.z) <= margin)
            return 0.0;

        return distanciaScale - vball_instance.position.z;
        };

    QuadTree qt_instance; // instancia local
    double ret_val = 0;
    int lado = 0;
    double feed = 0.00006;

    do {
        if (shot_options.percentShot > 1.3) shot_options.percentShot = 1.3;
        else if (shot_options.percentShot < 0.0) shot_options.percentShot = 0.1; // sqrt de 0 e 0 porra me diz que porra e essa ta ligado, JS completamente feito por maluco

        qt_instance.initShot(vball_instance, vclub_instance, vwind_instance, shot_options);

        ret_val = findAlturaColision_lambda(qt_instance, altura_colision);

        if (ret_val == 0.0) {
            isFind = true;
        }
        else {
            if (shot_options.percentShot >= 1.3 && ret_val > 0) break;
            if (shot_options.percentShot <= 0.1 && ret_val < 0) break;

            if (lado == 0) {
                lado = (ret_val < 0 ? -1 : 1);
            }
            else if ((ret_val < 0 && lado == 1) || (ret_val > 0 && lado == -1)) {
                feed *= 0.5;
            }
            shot_options.percentShot += ret_val * feed;
        }
    } while (!isFind && (++count) < limit_checking);

    if (isFind) {
        found_result.power = shot_options.percentShot;

        found_result.desvio = (vball_instance.position.x + (std::tan(shot_options.mira_rad) * distanciaScale)) * DESVIO_SCALE_PANGYA_TO_YARD;
        found_result.power_range = powerRange;

        found_result.smartData.desvio = found_result.desvio;
        found_result.smartData.altura_colision = altura_colision;
        // smartData.club e complexo para copiar invez disso so copiar as parte relevante mais facil
        found_result.smartData.options_copy = shot_options; // atualiza
        found_result.found = true;
    }
    return found_result;
}


// Console Emulation 
double checkValidInputDouble(const std::string& prompt, double defaultValue) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    if (line.empty()) return defaultValue;
    try {
        return std::stod(line);
    }
    catch (const std::invalid_argument& ia) {
        std::cerr << "Invalid argument: " << ia.what() << ". Using default: " << defaultValue << std::endl;
        return defaultValue;
    }
    catch (const std::out_of_range& oor) {
        std::cerr << "Out of Range error: " << oor.what() << ". Using default: " << defaultValue << std::endl;
        return defaultValue;
    }
}

int checkValidInputInt(const std::string& prompt, int defaultValue) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    if (line.empty()) return defaultValue;
    try {
        return std::stoi(line);
    }
    catch (const std::invalid_argument& ia) {
        std::cerr << "Invalid argument: " << ia.what() << ". Using default: " << defaultValue << std::endl;
        return defaultValue;
    }
    catch (const std::out_of_range& oor) {
        std::cerr << "Out of Range error: " << oor.what() << ". Using default: " << defaultValue << std::endl;
        return defaultValue;
    }
}

SlopeInputType checkValidInputSlopeConsole(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    if (line.empty()) return 0.0;

    std::stringstream ss(line);
    std::string segment;
    std::vector<double> parts;
    while (std::getline(ss, segment, ',')) {
        try {
            parts.push_back(std::stod(segment));
        }
        catch (...) {
            try {
                return std::stod(line);
            }
            catch (...) {
                std::cerr << "Invalid slope input. Using 0.0." << std::endl;
                return 0.0;
            }
        }
    }

    if (parts.size() == 3) {
        return Vector3D(parts[0] * SLOPE_BREAK_TO_CURVE_SLOPE,
            parts[1] * M_PI / 180.0,
            parts[2] * SLOPE_BREAK_TO_CURVE_SLOPE);
    }
    else if (parts.size() == 1) {
        return parts[0];
    }
    else {
        try {
            return std::stod(line);
        }
        catch (...) {
            std::cerr << "Invalid slope input format. Must be 'x,y,z' or a single number. Using 0.0." << std::endl;
            return 0.0;
        }
    }
}

double mycella_output_degree = 0.0;
double mycella_output_slope_break_scaled = 0.0;

void console_calc_mycella() {
    std::cout << "\n--- Mycella Slope Calculation ---" << std::endl;
    double shot_degree = checkValidInputDouble("Shot Degree (e.g., from aiming line): ", 0.0);
    double align_degree = checkValidInputDouble("Align Degree (character facing): ", 0.0);
    double slope_break_raw = checkValidInputDouble("Slope Break (raw value from terrain): ", 0.0);

    double console_res_width = checkValidInputDouble("Screen Width (for scaling, e.g., 1920): ", 800);
    double console_res_height = checkValidInputDouble("Screen Height (for scaling, e.g., 1080): ", 600);
    bool console_auto_fit = (checkValidInputInt("Auto Fit Resolution Scaling? (1 for yes, 0 for no): ", 1) == 1);


    double slope_real = std::abs(std::cos(std::abs(M_PI / 180.0 * (shot_degree - align_degree)))) * slope_break_raw;

    mycella_output_degree = shot_degree;
    mycella_output_slope_break_scaled = slope_real / getSlopeByResolution(console_res_height, console_auto_fit);

    std::cout << "Mycella Calculated Degree: " << mycella_output_degree << std::endl;
    std::cout << "Mycella Calculated Scaled Slope Break: " << std::fixed << std::setprecision(3) << mycella_output_slope_break_scaled << std::endl;
    std::cout << "These values will be used as defaults in the main calculation if you proceed." << std::endl;
}


void console_calc() {
    std::cout << "\n--- Smart Calculator ---" << std::endl;

    Club::ExtraPower power_options;
    power_options.auxpart = checkValidInputDouble("Auxpart Power: ", 0);
    power_options.mascot = checkValidInputDouble("Mascot Power: ", 4);
    power_options.card = checkValidInputDouble("Card Power: ", 4);
    power_options.ps_card = checkValidInputDouble("Card Power Shot Power: ", 8);
    power_options.ps_auxpart = 0;
    power_options.ps_mascot = 0;

    int player_pwr_slot = checkValidInputInt("Player Power Stat (e.g., 31): ", 31);

    std::string club_name_str;
    std::cout << "Club (1W, 2W, ..., SW - default 1W): ";
    std::getline(std::cin, club_name_str);
    if (club_name_str.empty()) club_name_str = "1W";
    ClubInfo selected_club_info = getClubInfoFromString(club_name_str);

    std::string shot_name_str;
    std::cout << "Shot Type (DUNK, TOMAHAWK, SPIKE, COBRA - default DUNK): ";
    std::getline(std::cin, shot_name_str);
    if (shot_name_str.empty()) shot_name_str = "DUNK";
    SHOT_TYPE selected_shot_type = getShotTypeFromString(shot_name_str);

    std::string ps_name_str;
    std::cout << "Power Shot (NO_POWER_SHOT, ONE_POWER_SHOT, TWO_POWER_SHOT, ITEM_15_POWER_SHOT - default NO_POWER_SHOT): ";
    std::getline(std::cin, ps_name_str);
    if (ps_name_str.empty()) ps_name_str = "NO_POWER_SHOT";
    POWER_SHOT_FACTORY selected_ps_type = getPowerShotFactoryFromString(ps_name_str);

    double distance = checkValidInputDouble("Distance (yards): ", 200);
    double height = checkValidInputDouble("Height (yards, target relative to ball): ", 0);
    double wind_str = checkValidInputDouble("Wind Strength (m/s): ", 0);


    double degree_val = checkValidInputDouble("Wind Angle (degrees, 0=tailwind, 90=right-to-left, 180=headwind, 270=left-to-right): ", mycella_output_degree != 0.0 ? mycella_output_degree : 0.0);

    double ground = checkValidInputDouble("Ground Condition (% e.g., 100): ", 100);
    if (ground == 0.0) ground = 100.0;
    double spin = checkValidInputDouble("Spin (0-30): ", 0);
    double curve = checkValidInputDouble("Curve (0-30): ", 0);


    std::string slope_prompt = "Slope Break (value or 'x,y,z' for vector - default from Mycella or 0.0): ";
    SlopeInputType slope_break_input;
    if (mycella_output_slope_break_scaled != 0.0) {
        std::cout << slope_prompt << "(Using Mycella default: " << std::fixed << std::setprecision(3) << mycella_output_slope_break_scaled << ")" << std::endl;
        slope_break_input = mycella_output_slope_break_scaled;
    }
    else {
        slope_break_input = checkValidInputSlopeConsole(slope_prompt);
    }


    FindPowerResult f_initial = find_power(
        power_options, player_pwr_slot, selected_club_info, selected_shot_type, selected_ps_type,
        distance, height, wind_str, degree_val, ground, spin, curve, slope_break_input
    );

    std::vector<FindPowerResult> f_results;
    f_results.push_back(f_initial);
    int index_f = 0;

    if (f_results[0].found) {
        do {
            index_f++;
            FindPowerResult next_f = find_power(
                power_options, player_pwr_slot, selected_club_info, selected_shot_type, selected_ps_type,
                distance, height, wind_str, degree_val, ground, spin, curve, slope_break_input,
                std::atan2(f_results[index_f - 1].desvio * 1.5, distance),
                f_results[index_f - 1].power
            );
            f_results.push_back(next_f);
        } while (f_results[index_f].found && f_results[index_f - 1].found &&
            std::abs(f_results[index_f - 1].desvio - f_results[index_f].desvio) >= 0.05 && index_f < 10);
    }

    if (f_results[index_f].found) {
        FindPowerResult final_result = f_results[index_f];
        double power_percent = final_result.power * 100.0;
        double power_yards = final_result.power_range * final_result.power;
        double desvio_pb_real = desvioByDegree(final_result.desvio, distance) / 0.2167;
        double desvio_pb_raw = (final_result.desvio / 0.2167 * -1.0);


        double console_rel_width = checkValidInputDouble("Client Relative Width (for smart desvio, e.g., 800): ", 800);
        double console_rel_height = checkValidInputDouble("Client Relative Height (for smart desvio, e.g., 600): ", 600);
        bool console_auto_fit = (checkValidInputInt("Auto Fit for Smart Desvio? (1 for yes, 0 for no): ", 1) == 1);
        double console_smart_dev_limit = checkValidInputDouble("Smart Deviation Limit (pb, 0 for auto): ", 0);


        std::string smart_desvio_str = smartDesvio(final_result.smartData,
            console_rel_width, console_rel_height,
            console_auto_fit, console_smart_dev_limit,
            power_options, player_pwr_slot);

        std::cout << "\n--- Calculation Result ---" << std::endl;
        std::cout << std::fixed << std::setprecision(1) << power_percent << "%, "
            << std::fixed << std::setprecision(1) << power_yards << "y, "
            << std::fixed << std::setprecision(2) << desvio_pb_real << "pb Real("
            << std::fixed << std::setprecision(2) << desvio_pb_raw << "pb), Smart("
            << smart_desvio_str << ")" << std::endl;
    }
    else {
        std::cout << "\n--- Calculation Result ---" << std::endl;
        std::cout << "Failed to calculate the shot. Target may be unreachable or too close." << std::endl;
    }
}


double desvioByDegree(double yards, double distance_val) {
    if (distance_val == 0) return 0;

    return std::sin(std::atan2(yards * -1.5, distance_val)) * distance_val / 1.5;
}

double fix(double value) {
    if (value < 0) return std::ceil(value);
    return std::floor(value);
}

double getSlopeByResolution(double res_height, bool auto_fit) {
    if (res_height < 480) return 1.0;
    double value = res_height / 480.0;
    if (!auto_fit) value = fix(value);
    if (value == 0) value = 1.0;
    return value;
}

double getResolutionPBLimit(double res_width, double res_height) {
    const double YARDS_TO_PB = 0.2167;
    if (res_height == 0) return 1.0;
    double value = ((480.0 / res_height * 0.006) * (res_width / 2.0)) / YARDS_TO_PB;
    if (value <= 0) value = 1.0;
    return value;
}

std::string smartDesvio(const SmartData& smartDataIn,
    double console_res_width, double console_res_height,
    bool console_auto_fit, double console_smart_dev_limit,
    const Club::ExtraPower& p_options, int p_pwr_slot) {
    double MAX_PB = console_smart_dev_limit;
    if (MAX_PB <= 0) {
        MAX_PB = std::floor(getResolutionPBLimit(console_res_width, console_res_height) * 10.0) / 10.0;
    }

    const double YARDS_TO_PB = 0.2167;
    const double YARDS_TO_PBA = 0.8668;
    const double YARDS_TO_PBA_PLUS = 1.032;

    double yards_val = desvioByDegree(smartDataIn.desvio, smartDataIn.options_copy.distance);
    std::ostringstream result_ss;

    double pb_sample = yards_val / YARDS_TO_PB;
    if (std::abs(pb_sample) <= MAX_PB) {
        result_ss << std::fixed << std::setprecision(2) << pb_sample << "pb";
        return result_ss.str();
    }

    pb_sample = yards_val / YARDS_TO_PBA;
    if (std::abs(pb_sample) <= MAX_PB) {
        result_ss << std::fixed << std::setprecision(2) << pb_sample << "pba";
        return result_ss.str();
    }

    pb_sample = yards_val / YARDS_TO_PBA_PLUS;
    if (std::abs(pb_sample) <= MAX_PB) {
        result_ss << std::fixed << std::setprecision(2) << pb_sample << "pba+";
        return result_ss.str();
    }


    Club temp_club;
    double powerRange = 230;


    for (int i = static_cast<int>(ALL_CLUB_INFO.size()) - 1; i >= 0; --i) {
        temp_club.init(ALL_CLUB_INFO[i]);
        powerRange = temp_club.getRange(p_options, p_pwr_slot, smartDataIn.options_copy.ps_type);

        double denominator = (powerRange * 3.2 * 1.4 - smartDataIn.altura_colision) * 0.0625;
        if (std::abs(denominator) < 1e-9) {
            pb_sample = (yards_val / YARDS_TO_PB) / 1e6;
        }
        else {
            pb_sample = (yards_val / YARDS_TO_PB) / denominator;
        }

        if (std::abs(pb_sample) <= MAX_PB) {
            result_ss << std::fixed << std::setprecision(2) << pb_sample << "pba" << static_cast<int>(powerRange);
            return result_ss.str();
        }
    }

    result_ss << std::fixed << std::setprecision(2) << pb_sample << "pba" << static_cast<int>(powerRange);
    return result_ss.str();
}


// String to Enum Mappers
std::map<std::string, const ClubInfo*> club_name_to_info_map;
std::map<std::string, SHOT_TYPE> shot_name_to_type_map;
std::map<std::string, POWER_SHOT_FACTORY> ps_name_to_factory_map;


void initialize_enum_maps() {
    for (size_t i = 0; i < CLUB_INFO_ENUM_NAMES.size(); ++i) {
        if (i < ALL_CLUB_INFO.size()) {
            std::string upper_name = CLUB_INFO_ENUM_NAMES[i];
            std::transform(upper_name.begin(), upper_name.end(), upper_name.begin(), ::toupper);
            club_name_to_info_map[upper_name] = &ALL_CLUB_INFO[i];
        }
    }
    for (size_t i = 0; i < SHOT_TYPE_ENUM_NAMES.size(); ++i) {
        std::string upper_name = SHOT_TYPE_ENUM_NAMES[i];
        std::transform(upper_name.begin(), upper_name.end(), upper_name.begin(), ::toupper);
        shot_name_to_type_map[upper_name] = static_cast<SHOT_TYPE>(i);
    }
    for (size_t i = 0; i < POWER_SHOT_FACTORY_ENUM_NAMES.size(); ++i) {
        std::string upper_name = POWER_SHOT_FACTORY_ENUM_NAMES[i];
        std::transform(upper_name.begin(), upper_name.end(), upper_name.begin(), ::toupper);
        ps_name_to_factory_map[upper_name] = static_cast<POWER_SHOT_FACTORY>(i);
    }
}


ClubInfo getClubInfoFromString(const std::string& club_name_str) {
    std::string upper_club_name = club_name_str;
    std::transform(upper_club_name.begin(), upper_club_name.end(), upper_club_name.begin(), ::toupper);
    auto it = club_name_to_info_map.find(upper_club_name);
    if (it != club_name_to_info_map.end()) {
        return *(it->second);
    }
    std::cerr << "Warning: Club '" << club_name_str << "' not found. Defaulting to 1W." << std::endl;
    return ALL_CLUB_INFO[0];
}

SHOT_TYPE getShotTypeFromString(const std::string& shot_name_str) {
    std::string upper_shot_name = shot_name_str;
    std::transform(upper_shot_name.begin(), upper_shot_name.end(), upper_shot_name.begin(), ::toupper);
    auto it = shot_name_to_type_map.find(upper_shot_name);
    if (it != shot_name_to_type_map.end()) {
        return it->second;
    }
    std::cerr << "Warning: Shot type '" << shot_name_str << "' not found. Defaulting to DUNK." << std::endl;
    return SHOT_TYPE::DUNK;
}

POWER_SHOT_FACTORY getPowerShotFactoryFromString(const std::string& ps_name_str) {
    std::string upper_ps_name = ps_name_str;
    std::transform(upper_ps_name.begin(), upper_ps_name.end(), upper_ps_name.begin(), ::toupper);
    auto it = ps_name_to_factory_map.find(upper_ps_name);
    if (it != ps_name_to_factory_map.end()) {
        return it->second;
    }
    std::cerr << "Warning: Power shot type '" << ps_name_str << "' not found. Defaulting to NO_POWER_SHOT." << std::endl;
    return POWER_SHOT_FACTORY::NO_POWER_SHOT;
}


int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    initialize_enum_maps();

    bool running = true;
    while (running) {
        std::cout << "\nSmart Calculator Menu:" << std::endl;
        std::cout << "1. Calculate Shot" << std::endl;
        std::cout << "2. Calculate Mycella Slope (affects defaults for option 1)" << std::endl;
        std::cout << "3. Exit" << std::endl;
        std::cout << "Enter your choice: ";

        std::string choice_str;
        std::getline(std::cin, choice_str);
        int choice = 0;
        try {
            if (!choice_str.empty()) choice = std::stoi(choice_str);
        }
        catch (...) {
            choice = 0;
        }


        switch (choice) {
        case 1:
            console_calc();
            break;
        case 2:
            console_calc_mycella();
            break;
        case 3:
            running = false;
            std::cout << "Exiting." << std::endl;
            break;
        default:
            std::cout << "Invalid choice. Please try again." << std::endl;
            break;
        }

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    return 0;
}