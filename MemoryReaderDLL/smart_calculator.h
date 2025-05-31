#include <vector>
#include <string>
#include <cmath>
#include <variant> 
#include <optional> 

// Constants
const double DESVIO_SCALE_PANGYA_TO_YARD = 0.3125 / 1.5;
const double _00D3D008 = 0.00001;
const double _00D046A8 = -1.0;
const double _00D00190 = 0.75;
const double _00D083A0 = 0.02; // Step time
const double _00D66CF8 = 3.0;
const double _00D3D028 = 0.00008;
const double _00D1A888 = 12.566371;
const double _00D3D210 = 25.132742;
const double _00CFF040 = 0.1;
const double _00D66CA0 = 0.5;
const double _00D16928 = 0.002;
const double _00D17908 = 0.349065847694874;
const double _00D19B98 = 0.0698131695389748;
const double _00D16758 = 0.01;
const double SLOPE_BREAK_TO_CURVE_SLOPE = 0.00875;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void initialize_enum_maps();

// Enums
enum class TYPE_DISTANCE {
    LESS_10 = 0,
    LESS_15,
    LESS_28,
    LESS_58,
    BIGGER_OR_EQUAL_58,
};

enum class POWER_SHOT_FACTORY {
    NO_POWER_SHOT = 0,
    ONE_POWER_SHOT,
    TWO_POWER_SHOT,
    ITEM_15_POWER_SHOT
};

enum class CLUB_TYPE {
    WOOD = 0,
    IRON,
    PW,
    PT
};

enum class SHOT_TYPE {
    DUNK = 0,
    TOMAHAWK,
    SPIKE,
    COBRA
};

// Structs and Classes
struct Vector3D {
    double x, y, z;

    Vector3D(double x = 0.0, double y = 0.0, double z = 0.0);
    Vector3D& normalize();
    Vector3D& multiplyScalar(double value);
    Vector3D& add(const Vector3D& other);
    Vector3D& add3D(double dx, double dy, double dz);
    Vector3D& sub(const Vector3D& other);
    Vector3D& sub3D(double dx, double dy, double dz);
    Vector3D& divideScalar(double value);
    Vector3D& cross(const Vector3D& other);
    double length() const;
    Vector3D clone() const;
};

extern const Vector3D _00E42544_vect_slope;

struct Ball {
    Vector3D position;
    Vector3D slope;
    int state_process;
    double max_height;
    int num_max_height;
    int count;
    Vector3D velocity;
    double ball_28, ball_2C, ball_30;
    double curva, spin;
    double rotation_curve, rotation_spin;
    int ball_44, ball_48, ball_70, ball_90;
    int ball_BC;
    double mass;
    double diametro;

    Ball();
    void copy(const Ball& other);
};

struct ClubInfo {
    CLUB_TYPE type;
    double rotation_spin;
    double rotation_curve;
    double power_factor;
    double degree;
    double power_base;

    ClubInfo() : type(CLUB_TYPE::WOOD),
        rotation_spin(0.0),
        rotation_curve(0.0),
        power_factor(0.0),
        degree(0.0),
        power_base(0.0) {
    }

    ClubInfo(CLUB_TYPE t, double rs, double rc, double pf, double deg, double pb);
};

extern const std::vector<ClubInfo> ALL_CLUB_INFO;
extern const std::vector<std::string> CLUB_INFO_ENUM_NAMES;
extern const std::vector<std::string> POWER_SHOT_FACTORY_ENUM_NAMES;
extern const std::vector<std::string> SHOT_TYPE_ENUM_NAMES;


struct Club {
    CLUB_TYPE type;
    TYPE_DISTANCE type_distance;
    double rotation_spin;
    double rotation_curve;
    double power_factor;
    double degree;
    double power_base;

    Club();
    void init(const ClubInfo& club_info);
    double getDregRad() const;

    struct ExtraPower {
        double auxpart;
        double mascot;
        double card;
        double ps_auxpart; // Nao usa no JS getPower essa porra
        double ps_mascot;  // Nao usa no JS getPower essa porra
        double ps_card;
        double total(POWER_SHOT_FACTORY ps_type) const;
    };

    double getPower(const ExtraPower& extraPower, int pwrSlot, POWER_SHOT_FACTORY ps, double spin_val) const;
    double getPower2(const ExtraPower& extraPower, int pwrSlot, POWER_SHOT_FACTORY ps) const; // boolean
    double getRange(const ExtraPower& extraPower, int pwrSlot, POWER_SHOT_FACTORY ps) const;
};

struct Wind {
    double wind_strength;
    double degree;

    Wind();
    Vector3D getWind() const;
};

// Structure for getValuesDegree result
struct DegreeValues {
    double cos, rad, sin;
    double _C, _10, _14;
    double neg_sin, neg_rad, cos2;
    double _24, _28, _2C;
};

// Global Game Objects 
extern Ball global_ball_obj;
extern Club global_club_obj;
extern Wind global_wind_obj;

// Options for QuadTree::initShot
struct InitShotOptions {
    Vector3D position;
    double distance;
    double percentShot;
    double ground; // 0-100
    double mira_rad;
    double slope_mira_rad;
    double spin; // 0-30 
    double curva; // 0-30 
    SHOT_TYPE shot_type;
    POWER_SHOT_FACTORY ps_type;

    struct PowerInfo {
        int pwr;
        Club::ExtraPower options;
    } power;
};


class QuadTree {
public:
    double gravityFactor;
    double gravity;
    Vector3D _21D8_vect;

    Ball* current_ball; // Pointeiro para o inicio do processo da bola
    Club* current_club;
    Wind* current_wind;

    Vector3D ball_position_init;
    double power_range_shot;
    SHOT_TYPE current_shot_type;
    double power_factor_shot;
    double percentShot_sqrt;
    int spike_init;
    int spike_med;
    double power_factor; // club.getPower result
    int cobra_init;

    QuadTree();
    double getGravity() const;

    void initShot(Ball& ball_ref, Club& club_ref, Wind& wind_ref, const InitShotOptions& options);
    DegreeValues getValuesDegree(double degree_val, int option) const;
    double getSlope(double mira, double line_ball);

    void ballProcess(double steptime, std::optional<double> final_step = std::nullopt);
    void bounceProcess(double steptime, std::optional<double> final_step = std::nullopt);
    Vector3D applyForce();
};

// Helper Functions 
TYPE_DISTANCE calculeTypeDistance(double distance);
double getPowerShotFactoryValue(POWER_SHOT_FACTORY ps);
double diffYZ(const Vector3D& v1, const Vector3D& v2);


struct SmartData {
    double desvio;
    double altura_colision;
    InitShotOptions options_copy;
    ClubInfo club_info_copy;
};

struct FindPowerResult {
    double power; // -1 se nao encontro essa porra
    double desvio;
    double power_range;
    SmartData smartData;
    bool found;

    FindPowerResult();
};

using SlopeInputType = std::variant<double, Vector3D>;

FindPowerResult find_power(
    const Club::ExtraPower& power_player_options,
    int player_pwr_slot,
    const ClubInfo& club_info,
    SHOT_TYPE shot,
    POWER_SHOT_FACTORY power_shot,
    double distancia,
    double altura,
    double vento,
    double angulo_vento,
    double terreno,
    double spin,
    double curva,
    const SlopeInputType& slope_param, // Essa desgraca pode ser double ou Vector3D na real tanto faz da tudo no mesmo.
    std::optional<double> mira = std::nullopt,
    std::optional<double> percent = std::nullopt
);

//  UI
double checkValidInputDouble(const std::string& prompt, double defaultValue = 0.0);
int checkValidInputInt(const std::string& prompt, int defaultValue = 0);
SlopeInputType checkValidInputSlopeConsole(const std::string& prompt);

void console_calc();
void console_calc_mycella();

double desvioByDegree(double yards, double distance);
double fix(double value);
double getSlopeByResolution(double res_height, bool auto_fit);
double getResolutionPBLimit(double res_width, double res_height);
std::string smartDesvio(const SmartData& smartDataIn,
    double console_res_width, double console_res_height,
    bool console_auto_fit, double console_smart_dev_limit,
    const Club::ExtraPower& p_options, int p_pwr_slot);


ClubInfo getClubInfoFromString(const std::string& club_name_str);
SHOT_TYPE getShotTypeFromString(const std::string& shot_name_str);
POWER_SHOT_FACTORY getPowerShotFactoryFromString(const std::string& ps_name_str);
